/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "pdcp_entity_tx.h"
#include "../security/security_engine_impl.h"
#include "../support/sdu_window_impl.h"
#include "srsran/instrumentation/traces/up_traces.h"
#include "srsran/support/bit_encoding.h"
#include "srsran/support/srsran_assert.h"

#ifdef JBPF_ENABLED
DEFINE_JBPF_HOOK(pdcp_dl_creation);
DEFINE_JBPF_HOOK(pdcp_dl_deletion);
DEFINE_JBPF_HOOK(pdcp_dl_new_sdu);
DEFINE_JBPF_HOOK(pdcp_dl_tx_data_pdu);
DEFINE_JBPF_HOOK(pdcp_dl_tx_control_pdu);
DEFINE_JBPF_HOOK(pdcp_dl_handle_tx_notification);
DEFINE_JBPF_HOOK(pdcp_dl_handle_delivery_notification);
DEFINE_JBPF_HOOK(pdcp_dl_discard_pdu);
DEFINE_JBPF_HOOK(pdcp_dl_reestablish);

#define CALL_JBPF_HOOK(hook_fn, ...)  \
  { \
    struct jbpf_pdcp_ctx_info jbpf_ctx = {0};\
    jbpf_ctx.ctx_id = 0;    \
    jbpf_ctx.cu_ue_index = ue_index;\
    jbpf_ctx.is_srb = rb_id.is_srb();\
    jbpf_ctx.rb_id = rb_id.is_srb() ? srb_id_to_uint(rb_id.get_srb_id()) \
                                    : drb_id_to_uint(rb_id.get_drb_id());\
    jbpf_ctx.rlc_mode = (uint8_t)rlc_mode; \
    if (cfg.discard_timer.has_value()) { \
      jbpf_ctx.window_info = {true,(uint32_t)tx_window->size(), tx_window_bytes}; \
    } else { \
      jbpf_ctx.window_info = {false, 0, 0}; \
    } \
    hook_fn(&jbpf_ctx, ##__VA_ARGS__); \
  }
  
#endif

using namespace srsran;

/// \brief Receive an SDU from the upper layers, apply encryption
/// and integrity protection and pass the resulting PDU
/// to the lower layers.
///
/// \param sdu Buffer that hold the SDU from higher layers.
/// \ref TS 38.323 section 5.2.1: Transmit operation
void pdcp_entity_tx::handle_sdu(byte_buffer buf)
{
  trace_point tx_tp = up_tracer.now();
  // Avoid TX'ing if we are close to overload RLC SDU queue
  if (st.tx_trans > st.tx_next) {
    logger.log_error("Invalid state, tx_trans is larger than tx_next. {}", st);
    return;
  }
  if ((st.tx_next - st.tx_trans) >= cfg.custom.rlc_sdu_queue) {
    if (not cfg.custom.warn_on_drop) {
      logger.log_info("Dropping SDU to avoid overloading RLC queue. rlc_sdu_queue={} {}", cfg.custom.rlc_sdu_queue, st);
    } else {
      logger.log_warning(
          "Dropping SDU to avoid overloading RLC queue. rlc_sdu_queue={} {}", cfg.custom.rlc_sdu_queue, st);
    }
    return;
  }
  if ((st.tx_next - st.tx_trans) >= (window_size - 1)) {
    logger.log_info("Dropping SDU to avoid going over the TX window size. {}", st);
    return;
  }

  metrics_add_sdus(1, buf.length());

  logger.log_debug(buf.begin(), buf.end(), "TX SDU. sdu_len={}", buf.length());

  // The PDCP is not allowed to use the same COUNT value more than once for a given security key,
  // see TS 38.331, section 5.3.1.2. To avoid this, we notify the RRC once we exceed a "maximum"
  // COUNT. It is then the RRC's responsibility to refresh the keys. We continue transmitting until
  // we reached a maximum hard COUNT, after which we simply refuse to TX any further.
  if (st.tx_next >= cfg.custom.max_count.hard) {
    if (!max_count_overflow) {
      logger.log_error("Reached maximum count, refusing to transmit further. count={}", st.tx_next);
      upper_cn.on_protocol_failure();
      max_count_overflow = true;
    }
    return;
  }
  if (st.tx_next >= cfg.custom.max_count.notify) {
    if (!max_count_notified) {
      logger.log_warning("Approaching count wrap-around, notifying RRC. count={}", st.tx_next);
      upper_cn.on_max_count_reached();
      max_count_notified = true;
    }
  }

  // We will need a copy of the SDU for the discard timer when using AM
  byte_buffer sdu;
  if (cfg.discard_timer.has_value() && is_am()) {
    auto sdu_copy = buf.deep_copy();
    if (not sdu_copy.has_value()) {
      logger.log_error("Unable to deep copy SDU");
      upper_cn.on_protocol_failure();
      return;
    }
    sdu = std::move(sdu_copy.value());
  }

  // Perform header compression
  // TODO

#ifdef JBPF_ENABLED 
  CALL_JBPF_HOOK(hook_pdcp_dl_new_sdu, st.tx_next, buf.length());
#endif

  // Prepare header
  pdcp_data_pdu_header hdr = {};
  hdr.sn                   = SN(st.tx_next);

  // Pack header
  if (not write_data_pdu_header(buf, hdr)) {
    logger.log_error("Could not append PDU header, dropping SDU and notifying RRC. count={}", st.tx_next);
    upper_cn.on_protocol_failure();
    return;
  }

  // Apply ciphering and integrity protection
  expected<byte_buffer> exp_buf = apply_ciphering_and_integrity_protection(std::move(buf), st.tx_next);
  if (not exp_buf.has_value()) {
    logger.log_error("Could not apply ciphering and integrity protection, dropping SDU and notifying RRC. count={}",
                     st.tx_next);
    upper_cn.on_protocol_failure();
    return;
  }
  byte_buffer protected_buf = std::move(exp_buf.value());

  // Create a discard timer and put into tx_window. For AM, also store the SDU for a possible data recovery procedure.
  if (cfg.discard_timer.has_value()) {
    unique_timer discard_timer = {};
    // Only start for finite durations
    if (cfg.discard_timer.value() != pdcp_discard_timer::infinity) {
      discard_timer = ue_dl_timer_factory.create_timer();
      discard_timer.set(std::chrono::milliseconds(static_cast<unsigned>(cfg.discard_timer.value())),
                        discard_callback{this, st.tx_next});
      discard_timer.run();
    }

    // If the place in the tx_window is occupied by an old element from previous wrap, discard that element first.
    if (tx_window->has_sn(st.tx_next)) {
      uint32_t old_count = (*tx_window)[st.tx_next].count;
      logger.log_error("Tx window full. Discarding old_count={}. tx_next={}", old_count, st.tx_next);
      discard_pdu(old_count);
    }

    pdcp_tx_sdu_info& sdu_info = tx_window->add_sn(st.tx_next);
#ifdef JBPF_ENABLED
    tx_window_bytes += sdu.length();
    sdu_info.time_of_arrival = std::chrono::high_resolution_clock::now();
#endif
    sdu_info.count             = st.tx_next;
    sdu_info.discard_timer     = std::move(discard_timer);
    if (is_am()) {
      sdu_info.sdu = std::move(sdu);
    }
    logger.log_debug("Added to tx window. count={} discard_timer={}", st.tx_next, cfg.discard_timer);
  }

  // Write to lower layers
  write_data_pdu_to_lower_layers(st.tx_next, std::move(protected_buf), /* is_retx = */ false);

  // Increment TX_NEXT
  uint32_t tx_count = st.tx_next++;

  // Automatically trigger delivery notifications when using test mode
  if (cfg.custom.test_mode) {
    handle_transmit_notification(SN(tx_count));
  }
  up_tracer << trace_event{"pdcp_tx_pdu", tx_tp};
}

void pdcp_entity_tx::reestablish(security::sec_128_as_config sec_cfg)
{
  logger.log_debug("Reestablishing PDCP. st={}", st);

#ifdef JBPF_ENABLED 
  CALL_JBPF_HOOK(hook_pdcp_dl_reestablish)
#endif

  // - for UM DRBs and AM DRBs, reset the ROHC protocol for uplink and start with an IR state in U-mode (as
  //   defined in RFC 3095 [8] and RFC 4815 [9]) if drb-ContinueROHC is not configured in TS 38.331 [3];
  // - for UM DRBs and AM DRBs, reset the EHC protocol for uplink if drb-ContinueEHC-UL is not configured in
  //   TS 38.331 [3];
  //   Header compression not supported yet (TODO).

  // - for UM DRBs and SRBs, set TX_NEXT to the initial value;
  // - for SRBs, discard all stored PDCP SDUs and PDCP PDUs;
  if (is_srb() || is_um()) {
    reset(); // While not explicitly stated in the spec,
             // there is no point in storing PDCP UM PDUs.
             // They cannot be RETXed and RLC already discarded them.
             // Also, this avoids having multiple discard timers
             // associated with the with the same COUNT.
  }

  // - apply the ciphering algorithm and key provided by upper layers during the PDCP entity re-establishment
  //   procedure;
  // - apply the integrity protection algorithm and key provided by upper layers during the PDCP entity re-
  //   establishment procedure;
  configure_security(sec_cfg, integrity_enabled, ciphering_enabled);

  // - for UM DRBs, for each PDCP SDU already associated with a PDCP SN but for which a corresponding PDU has
  //   not previously been submitted to lower layers, and;
  // - for AM DRBs for Uu interface whose PDCP entities were suspended, from the first PDCP SDU for which the
  //   successful delivery of the corresponding PDCP Data PDU has not been confirmed by lower layers, for each
  //   PDCP SDU already associated with a PDCP SN:
  //   - consider the PDCP SDUs as received from upper layer;
  //   - perform transmission of the PDCP SDUs in ascending order of the COUNT value associated to the PDCP
  //     SDU prior to the PDCP re-establishment without restarting the discardTimer, as specified in clause 5.2.1;
  //
  //  For UM DRBs, when SDUs are associated with a PDCP SN they are immediately pushed to the lower-layer.
  //  As such, there is nothing to do here.
  //  For AM DRBs, PDCP entity suspension is not supported yet (TODO).

  // - for AM DRBs whose PDCP entities were not suspended, from the first PDCP SDU for which the successful
  //   delivery of the corresponding PDCP Data PDU has not been confirmed by lower layers, perform retransmission
  //   or transmission of all the PDCP SDUs already associated with PDCP SNs in ascending order of the COUNT
  //   values associated to the PDCP SDU prior to the PDCP entity re-establishment as specified below:
  //   - perform header compression of the PDCP SDU using ROHC as specified in the clause 5.7.4 and/or using
  //     EHC as specified in the clause 5.12.4;
  //   - perform integrity protection and ciphering of the PDCP SDU using the COUNT value associated with this
  //     PDCP SDU as specified in the clause 5.9 and 5.8;
  //   - submit the resulting PDCP Data PDU to lower layer, as specified in clause 5.2.1.
  if (is_am()) {
    retransmit_all_pdus();
  }
  logger.log_info("Reestablished PDCP. st={}", st);
}

void pdcp_entity_tx::write_data_pdu_to_lower_layers(uint32_t count, byte_buffer buf, bool is_retx)
{
  logger.log_info(buf.begin(),
                  buf.end(),
                  "TX PDU. type=data pdu_len={} sn={} count={} is_retx={}",
                  buf.length(),
                  SN(count),
                  count,
                  is_retx);
  metrics_add_pdus(1, buf.length());

#ifdef JBPF_ENABLED 
  uint64_t latency = 0;
  if (cfg.discard_timer && tx_window->has_sn(count)) {
    const auto& sdu_info = (*tx_window)[count];
    latency = (uint64_t)(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() -
                                                                      sdu_info.time_of_arrival)).count();
  }
  CALL_JBPF_HOOK(hook_pdcp_dl_tx_data_pdu, buf.length(), count,
                static_cast<uint8_t>(is_retx),
                cfg.discard_timer && tx_window->has_sn(count),
                latency);  
#endif

  lower_dn.on_new_pdu(std::move(buf), is_retx);
}

void pdcp_entity_tx::write_control_pdu_to_lower_layers(byte_buffer buf)
{
  logger.log_info(buf.begin(), buf.end(), "TX PDU. type=ctrl pdu_len={}", buf.length());
  metrics_add_pdus(1, buf.length());

#ifdef JBPF_ENABLED 
  CALL_JBPF_HOOK(hook_pdcp_dl_tx_control_pdu, buf.length());
#endif

  lower_dn.on_new_pdu(std::move(buf), /* is_retx = */ false);
}

void pdcp_entity_tx::handle_status_report(byte_buffer_chain status)
{
  auto status_buffer = byte_buffer::create(status.begin(), status.end());
  if (not status_buffer.has_value()) {
    logger.log_warning("Unable to allocate byte_buffer");
    return;
  }

  byte_buffer buf = std::move(status_buffer.value());
  bit_decoder dec(buf);

  // Unpack and check PDU header
  uint32_t dc = 0;
  dec.unpack(dc, 1);
  if (dc != to_number(pdcp_dc_field::control)) {
    logger.log_warning(
        buf.begin(), buf.end(), "Invalid D/C field in status report. dc={}", to_number(pdcp_dc_field::control), dc);
    return;
  }
  uint32_t cpt = 0;
  dec.unpack(cpt, 3);
  if (cpt != to_number(pdcp_control_pdu_type::status_report)) {
    logger.log_warning(buf.begin(),
                       buf.end(),
                       "Invalid CPT field in status report. cpt={}",
                       to_number(pdcp_control_pdu_type::status_report),
                       cpt);
    return;
  }
  uint32_t reserved = 0;
  dec.unpack(reserved, 4);
  if (reserved != 0) {
    logger.log_warning(
        buf.begin(), buf.end(), "Ignoring status report because reserved bits are set. reserved={:#x}", reserved);
    return;
  }

  // Unpack FMC field
  uint32_t fmc = 0;
  dec.unpack(fmc, 32);
  logger.log_info("Status report. fmc={}", fmc);

  // Discard any SDU with COUNT < FMC
  for (uint32_t count = st.tx_next_ack; count < fmc; count++) {
    discard_pdu(count);
  }

  // Evaluate bitmap: discard any SDU with the bit in the bitmap set to 1
  unsigned bit = 0;
  while (dec.unpack(bit, 1)) {
    fmc++;
    // Bit == 0: PDCP SDU with COUNT = (FMC + bit position) modulo 2^32 is missing.
    // Bit == 1: PDCP SDU with COUNT = (FMC + bit position) modulo 2^32 is correctly received.
    if (bit == 1) {
      discard_pdu(fmc);
    }
  }
}

/*
 * Ciphering and Integrity Protection Helpers
 */
expected<byte_buffer> pdcp_entity_tx::apply_ciphering_and_integrity_protection(byte_buffer buf, uint32_t count)
{
  if (sec_engine == nullptr) {
    // Security is not configured. Pass through for DRBs; append zero MAC-I for SRBs.
    if (is_srb()) {
      security::sec_mac mac = {};
      if (not buf.append(mac)) {
        logger.log_warning("Failed to append MAC-I to PDU. count={}", count);
        return make_unexpected(default_error_t{});
      }
    }
    return buf;
  }

  // TS 38.323, section 5.8: Ciphering
  // The data unit that is ciphered is the MAC-I and the
  // data part of the PDCP Data PDU except the
  // SDAP header and the SDAP Control PDU if included in the PDCP SDU.

  // TS 38.323, section 5.9: Integrity protection
  // The data unit that is integrity protected is the PDU header
  // and the data part of the PDU before ciphering.

  unsigned                  hdr_size = cfg.sn_size == pdcp_sn_size::size12bits ? 2 : 3;
  security::security_result result   = sec_engine->encrypt_and_protect_integrity(std::move(buf), hdr_size, count);
  if (!result.buf.has_value()) {
    logger.log_warning("Failed to apply security on PDU. count={}", result.count);
  }
  return {std::move(result.buf.value())};
}

/*
 * Security configuration
 */
void pdcp_entity_tx::configure_security(security::sec_128_as_config sec_cfg,
                                        security::integrity_enabled integrity_enabled_,
                                        security::ciphering_enabled ciphering_enabled_)
{
  srsran_assert((is_srb() && sec_cfg.domain == security::sec_domain::rrc) ||
                    (is_drb() && sec_cfg.domain == security::sec_domain::up),
                "Invalid sec_domain={} for {} in {}",
                sec_cfg.domain,
                rb_type,
                rb_id);
  // The 'NULL' integrity protection algorithm (nia0) is used only for SRBs and for the UE in limited service mode,
  // see TS 33.501 [11] and when used for SRBs, integrity protection is disabled for DRBs. In case the ′NULL'
  // integrity protection algorithm is used, 'NULL' ciphering algorithm is also used.
  // Ref: TS 38.331 Sec. 5.3.1.2
  //
  // From TS 38.501 Sec. 6.7.3.6: UEs that are in limited service mode (LSM) and that cannot be authenticated (...) may
  // still be allowed to establish emergency session by sending the emergency registration request message. (...)
  if ((sec_cfg.integ_algo == security::integrity_algorithm::nia0) &&
      (is_drb() || (is_srb() && sec_cfg.cipher_algo != security::ciphering_algorithm::nea0))) {
    logger.log_error("Integrity algorithm NIA0 is only permitted for SRBs configured with NEA0. is_srb={} NIA{} NEA{}",
                     is_srb(),
                     sec_cfg.integ_algo,
                     sec_cfg.cipher_algo);
  }

  // Evaluate and store integrity indication
  if (integrity_enabled_ == security::integrity_enabled::on) {
    if (!sec_cfg.k_128_int.has_value()) {
      logger.log_error("Cannot enable integrity protection: Integrity key is not configured.");
      return;
    }
    if (!sec_cfg.integ_algo.has_value()) {
      logger.log_error("Cannot enable integrity protection: Integrity algorithm is not configured.");
      return;
    }
  } else {
    srsran_assert(!is_srb(), "Integrity protection cannot be disabled for SRBs.");
  }
  integrity_enabled = integrity_enabled_;

  // Evaluate and store ciphering indication
  ciphering_enabled = ciphering_enabled_;

  auto direction = cfg.direction == pdcp_security_direction::uplink ? security::security_direction::uplink
                                                                    : security::security_direction::downlink;
  sec_engine     = std::make_unique<security::security_engine_impl>(
      sec_cfg, bearer_id, direction, integrity_enabled, ciphering_enabled);

  logger.log_info("Security configured: NIA{} ({}) NEA{} ({}) domain={}",
                  sec_cfg.integ_algo,
                  integrity_enabled,
                  sec_cfg.cipher_algo,
                  ciphering_enabled,
                  sec_cfg.domain);
  if (sec_cfg.k_128_int.has_value()) {
    logger.log_info("128 K_int: {}", sec_cfg.k_128_int.value());
  }
  logger.log_info("128 K_enc: {}", sec_cfg.k_128_enc);
};

/*
 * Status report and data recovery
 */
void pdcp_entity_tx::send_status_report()
{
  if (cfg.status_report_required) {
    logger.log_info("Status report triggered.");
    byte_buffer status_report = status_provider->compile_status_report();
    write_control_pdu_to_lower_layers(std::move(status_report));
  } else {
    logger.log_warning("Status report triggered but not configured.");
  }
}

void pdcp_entity_tx::data_recovery()
{
  srsran_assert(is_drb() && cfg.rlc_mode == pdcp_rlc_mode::am, "Invalid bearer type for data recovery.");
  logger.log_info("Data recovery requested.");

  /*
   * TS 38.323 Sec. 5.4.1:
   * [...] the receiving PDCP entity shall trigger a PDCP status report when:
   * [...] -upper layer requests a PDCP data recovery; [...]
   */
  if (cfg.status_report_required) {
    send_status_report();
  }
  retransmit_all_pdus();
}

void pdcp_entity_tx::reset()
{
  st = {};
  tx_window->clear();
  logger.log_debug("Entity was reset. {}", st);
}

void pdcp_entity_tx::retransmit_all_pdus()
{
  if (!cfg.discard_timer.has_value()) {
    logger.log_debug("Cannot retransmit. No discard timer configured.");
    return;
  }
  if (!is_am()) {
    logger.log_error("Cannot retransmit. Not an AM bearer.");
    return;
  }

  // Since we are retransmitting, rewind tx_trans to tx_next_ack
  st.tx_trans = st.tx_next_ack;

  for (uint32_t count = st.tx_next_ack; count < st.tx_next; count++) {
    if (tx_window->has_sn(count)) {
      pdcp_tx_sdu_info& sdu_info = (*tx_window)[count];

      // Prepare header
      pdcp_data_pdu_header hdr = {};
      hdr.sn                   = SN(sdu_info.count);

      // Pack header
      auto buf_copy = sdu_info.sdu.deep_copy();
      if (not buf_copy.has_value()) {
        logger.log_error("Could not deep copy SDU, dropping SDU and notifying RRC. count={} {}", sdu_info.count, st);
        upper_cn.on_protocol_failure();
        return;
      }

      byte_buffer buf = std::move(buf_copy.value());
      if (not write_data_pdu_header(buf, hdr)) {
        logger.log_error(
            "Could not append PDU header, dropping SDU and notifying RRC. count={} {}", sdu_info.count, st);
        upper_cn.on_protocol_failure();
        return;
      }

      // Perform header compression if required
      // (TODO)

      // Perform integrity protection and ciphering
      expected<byte_buffer> exp_buf = apply_ciphering_and_integrity_protection(std::move(buf), sdu_info.count);
      if (not exp_buf.has_value()) {
        logger.log_error("Could not apply ciphering and integrity protection during retransmissions, dropping SDU and "
                         "notifying RRC. count={} {}",
                         sdu_info.count,
                         st);
        upper_cn.on_protocol_failure();
        return;
      }

      byte_buffer protected_buf = std::move(exp_buf.value());
      write_data_pdu_to_lower_layers(sdu_info.count, std::move(protected_buf), /* is_retx = */ true);
    }
  }
}

/*
 * Notification Helpers
 */
void pdcp_entity_tx::handle_transmit_notification(uint32_t notif_sn)
{
  logger.log_debug("Handling transmit notification for notif_sn={}", notif_sn);
  if (notif_sn >= pdcp_sn_cardinality(cfg.sn_size)) {
    logger.log_error("Invalid transmit notification for notif_sn={} exceeds sn_size={}", notif_sn, cfg.sn_size);
    return;
  }
  uint32_t notif_count = notification_count_estimation(notif_sn);
  if (notif_count < st.tx_trans) {
    logger.log_info(
        "Invalid notification SN, notif_count is too low. notif_sn={} notif_count={} {}", notif_sn, notif_count, st);
    return;
  }
  if (notif_count >= st.tx_next) {
    logger.log_error(
        "Invalid notification SN, notif_count is too high. notif_sn={} notif_count={} {}", notif_sn, notif_count, st);
    return;
  }
  st.tx_trans = notif_count + 1;
  logger.log_debug("Updated tx_trans. {}", st);

  // Stop discard timers if required
  if (!cfg.discard_timer.has_value()) {
    return;
  }

  if (is_um()) {
    stop_discard_timer(notif_count);
  }

#ifdef JBPF_ENABLED
  CALL_JBPF_HOOK(hook_pdcp_dl_handle_tx_notification, notif_sn);
#endif
}

void pdcp_entity_tx::handle_delivery_notification(uint32_t notif_sn)
{
  logger.log_debug("Handling delivery notification for notif_sn={}", notif_sn);
  if (notif_sn >= pdcp_sn_cardinality(cfg.sn_size)) {
    logger.log_error("Invalid delivery notification for notif_sn={} exceeds sn_size={}", notif_sn, cfg.sn_size);
    return;
  }
  uint32_t notif_count = notification_count_estimation(notif_sn);
  if (notif_count >= st.tx_next) {
    logger.log_error("Got notification for invalid COUNT. notif_count={} {}", notif_count, st);
    return;
  }

  // Stop discard timers if required
  if (!cfg.discard_timer.has_value()) {
    return;
  }

  if (is_am()) {
    stop_discard_timer(notif_count);
  } else {
    logger.log_error("Ignored unexpected PDU delivery notification in UM bearer. notif_sn={}", notif_sn);
  }

#ifdef JBPF_ENABLED
  CALL_JBPF_HOOK(hook_pdcp_dl_handle_delivery_notification, notif_sn);
#endif
}

void pdcp_entity_tx::handle_retransmit_notification(uint32_t notif_sn)
{
  if (SRSRAN_UNLIKELY(is_srb())) {
    logger.log_error("Ignored unexpected PDU retransmit notification in SRB. notif_sn={}", notif_sn);
    return;
  }
  if (SRSRAN_UNLIKELY(is_um())) {
    logger.log_error("Ignored unexpected PDU retransmit notification in UM bearer. notif_sn={}", notif_sn);
    return;
  }

  // Nothing to do here
  logger.log_debug("Ignored handling PDU retransmit notification for notif_sn={}", notif_sn);
}

void pdcp_entity_tx::handle_delivery_retransmitted_notification(uint32_t notif_sn)
{
  if (SRSRAN_UNLIKELY(is_srb())) {
    logger.log_error("Ignored unexpected PDU delivery retransmitted notification in SRB. notif_sn={}", notif_sn);
    return;
  }
  if (SRSRAN_UNLIKELY(is_um())) {
    logger.log_error("Ignored unexpected PDU delivery retransmitted notification in UM bearer. notif_sn={}", notif_sn);
    return;
  }

  // TODO: Here we can stop discard timers of successfully retransmitted PDUs once they can be distinguished from
  // origianls (e.g. if they are moved into a separate container upon retransmission).
  // For now those retransmitted PDUs will be cleaned when handling delivery notification for following originals.
  logger.log_debug("Ignored handling PDU delivery retransmitted notification for notif_sn={}", notif_sn);
}

uint32_t pdcp_entity_tx::notification_count_estimation(uint32_t notification_sn)
{
  // Get lower edge of the window. If discard timer is enabled, use the lower edge of the tx_window, i.e. TX_NEXT_ACK.
  // If discard timer is not configured, use TX_TRANS as lower edge of window.
  uint32_t tx_lower;
  if (cfg.discard_timer.has_value()) {
    tx_lower = st.tx_next_ack;
  } else {
    tx_lower = st.tx_trans;
  }

  /*
   * Calculate NOTIFICATION_COUNT. This is adapted from TS 38.331 Sec. 5.2.2 "Receive operation" of the Rx side.
   *
   * - if NOTIFICATION_SN < SN(TX_LOWER) – Window_Size:
   *   - NOTIFICATION_HFN = HFN(TX_LOWER) + 1.
   * - else if NOTIFICATION_SN >= SN(TX_LOWER) + Window_Size:
   *   - NOTIFICATION_HFN = HFN(TX_LOWER) – 1.
   * - else:
   *   - NOTIFICATION_HFN = HFN(TX_LOWER);
   * - NOTIFICATION_COUNT = [NOTIFICATION_HFN, NOTIFICATION_SN].
   */
  uint32_t notification_hfn;
  if ((int64_t)notification_sn < (int64_t)SN(tx_lower) - (int64_t)window_size) {
    notification_hfn = HFN(tx_lower) + 1;
  } else if (notification_sn >= SN(tx_lower) + window_size) {
    notification_hfn = HFN(tx_lower) - 1;
  } else {
    notification_hfn = HFN(tx_lower);
  }
  return COUNT(notification_hfn, notification_sn);
}

/*
 * PDU Helpers
 */
bool pdcp_entity_tx::write_data_pdu_header(byte_buffer& buf, const pdcp_data_pdu_header& hdr) const
{
  // Sanity check: 18-bit SN not allowed for SRBs
  srsran_assert(
      !(is_srb() && cfg.sn_size == pdcp_sn_size::size18bits), "Invalid SN size for SRB. sn_size={}", cfg.sn_size);

  unsigned hdr_len = cfg.sn_size == pdcp_sn_size::size12bits ? 2 : 3;
  auto     view    = buf.reserve_prepend(cfg.sn_size == pdcp_sn_size::size12bits ? 2 : 3);
  if (view.length() != hdr_len) {
    logger.log_error("Not enough space to write header. sn_size={}", cfg.sn_size);
    return false;
  }

  byte_buffer::iterator hdr_writer = buf.begin();
  if (hdr_writer == buf.end()) {
    logger.log_error("Not enough space to write header. sn_size={}", cfg.sn_size);
  }

  // Set D/C if required
  if (is_drb()) {
    // D/C bit field (1).
    *hdr_writer = 0x80;
  } else {
    // No D/C bit field.
    *hdr_writer = 0x00;
  }

  // Add SN
  switch (cfg.sn_size) {
    case pdcp_sn_size::size12bits:
      *hdr_writer |= (hdr.sn & 0x00000f00U) >> 8U;
      hdr_writer++;
      if (hdr_writer == buf.end()) {
        logger.log_error("Not enough space to write header. sn_size={}", cfg.sn_size);
      }
      *hdr_writer = hdr.sn & 0x000000ffU;
      break;
    case pdcp_sn_size::size18bits:
      *hdr_writer |= (hdr.sn & 0x00030000U) >> 16U;
      hdr_writer++;
      if (hdr_writer == buf.end()) {
        logger.log_error("Not enough space to write header. sn_size={}", cfg.sn_size);
      }
      *hdr_writer = (hdr.sn & 0x0000ff00U) >> 8U;
      hdr_writer++;
      if (hdr_writer == buf.end()) {
        logger.log_error("Not enough space to write header. sn_size={}", cfg.sn_size);
      }
      *hdr_writer = hdr.sn & 0x000000ffU;
      break;
    default:
      logger.log_error("Invalid sn_size={}", cfg.sn_size);
      return false;
  }
  return true;
}

/*
 * Timers
 */
void pdcp_entity_tx::stop_discard_timer(uint32_t highest_count)
{
  if (!cfg.discard_timer.has_value()) {
    logger.log_debug("Cannot stop discard timers. No discard timer configured. highest_count={}", highest_count);
    return;
  }
  if (highest_count < st.tx_next_ack || highest_count >= st.tx_next) {
    logger.log_warning("Cannot stop discard timers. highest_count={} is outside tx_window. {}", highest_count, st);
    return;
  }
  logger.log_debug("Stopping discard timers. highest_count={}", highest_count);

  // Stop discard timers and update TX_NEXT_ACK to oldest element in tx_window
  while (st.tx_next_ack <= highest_count) {
    if (tx_window->has_sn(st.tx_next_ack)) {
#ifdef JBPF_ENABLED
      const auto& tx_sdu_info = (*tx_window)[st.tx_next_ack];
      tx_window_bytes -= tx_sdu_info.sdu.length();
#endif
      tx_window->remove_sn(st.tx_next_ack);
      logger.log_debug("Stopped discard timer. count={}", st.tx_next_ack);
    }
    st.tx_next_ack++;
  }

  // Update TX_TRANS if it falls out of the tx_window
  if (st.tx_trans < st.tx_next_ack) {
    st.tx_trans = st.tx_next_ack;
  }
}

void pdcp_entity_tx::discard_pdu(uint32_t count)
{
  if (!cfg.discard_timer.has_value()) {
    logger.log_debug("Cannot discard PDU. No discard timer configured. count={}", count);
    return;
  }
  if (count < st.tx_next_ack || count >= st.tx_next) {
    logger.log_warning("Cannot discard PDU. The PDU is outside tx_window. count={} {}", count, st);
    return;
  }
  if (!tx_window->has_sn(count)) {
    logger.log_warning("Cannot discard PDU. The PDU is missing in tx_window. count={} {}", count, st);
    return;
  }
  logger.log_debug("Discarding PDU. count={}", count);

  // Notify lower layers of the discard. It's the RLC to actually discard, if no segment was transmitted yet.
  lower_dn.on_discard_pdu(SN(count));

#ifdef JBPF_ENABLED
  const auto& tx_sdu_info = (*tx_window)[count];
  tx_window_bytes -= tx_sdu_info.sdu.length();
#endif
  tx_window->remove_sn(count);

  // Update TX_NEXT_ACK to oldest element in tx_window
  while (st.tx_next_ack < st.tx_next && !tx_window->has_sn(st.tx_next_ack)) {
    st.tx_next_ack++;
  }

  // Update TX_TRANS if it falls out of the tx_window
  if (st.tx_trans < st.tx_next_ack) {
    st.tx_trans = st.tx_next_ack;
  }

#ifdef JBPF_ENABLED  
  CALL_JBPF_HOOK(hook_pdcp_dl_discard_pdu, count);
#endif   
}

std::unique_ptr<sdu_window<pdcp_entity_tx::pdcp_tx_sdu_info>> pdcp_entity_tx::create_tx_window(pdcp_sn_size sn_size_)
{
  std::unique_ptr<sdu_window<pdcp_tx_sdu_info>> tx_window_;
  switch (sn_size_) {
    case pdcp_sn_size::size12bits:
      tx_window_ = std::make_unique<sdu_window_impl<pdcp_tx_sdu_info,
                                                    pdcp_window_size(pdcp_sn_size_to_uint(pdcp_sn_size::size12bits)),
                                                    pdcp_bearer_logger>>(logger);
      break;
    case pdcp_sn_size::size18bits:
      tx_window_ = std::make_unique<sdu_window_impl<pdcp_tx_sdu_info,
                                                    pdcp_window_size(pdcp_sn_size_to_uint(pdcp_sn_size::size18bits)),
                                                    pdcp_bearer_logger>>(logger);
      break;
    default:
      srsran_assertion_failure("Cannot create tx_window for unsupported sn_size={}.", pdcp_sn_size_to_uint(sn_size_));
  }
  return tx_window_;
}

// Discard Timer Callback (discardTimer)
void pdcp_entity_tx::discard_callback::operator()(timer_id_t timer_id)
{
  parent->logger.log_debug("Discard timer expired. count={}", discard_count);

  // Add discard to metrics
  parent->metrics_add_discard_timouts(1);

  // Discard PDU
  // NOTE: this will delete the callback. It *must* be the last instruction.
  parent->discard_pdu(discard_count);
}
