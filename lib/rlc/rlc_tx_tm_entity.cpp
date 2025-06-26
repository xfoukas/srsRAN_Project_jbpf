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

#include "rlc_tx_tm_entity.h"

#ifdef JBPF_ENABLED
#include "jbpf_srsran_hooks.h"

#define CALL_JBPF_HOOK(hook_fn, ...) {                           \
  struct jbpf_rlc_ctx_info jbpf_ctx = {0};  \
  jbpf_ctx.ctx_id = 0; \
  jbpf_ctx.gnb_du_id = (uint64_t)gnb_du_id;\
  jbpf_ctx.du_ue_index = ue_index;\
  jbpf_ctx.is_srb = rb_id.is_srb();\
  jbpf_ctx.rb_id = rb_id.is_srb() ? srb_id_to_uint(rb_id.get_srb_id()) \
                                   : drb_id_to_uint(rb_id.get_drb_id());\
  jbpf_ctx.direction = JBPF_DL; \
  jbpf_ctx.rlc_mode = JBPF_RLC_MODE_TM; \
  jbpf_ctx.u.tm_tx.sdu_queue_info = {                             \
      true, \
      sdu_queue.get_state().n_sdus,                             \
      sdu_queue.get_state().n_bytes};                           \
  hook_fn(&jbpf_ctx, ##__VA_ARGS__); \
}
#endif

using namespace srsran;

rlc_tx_tm_entity::rlc_tx_tm_entity(gnb_du_id_t                          du_id_,
                                   du_ue_index_t                        ue_index_,
                                   rb_id_t                              rb_id_,
                                   const rlc_tx_tm_config&              config,
                                   rlc_tx_upper_layer_data_notifier&    upper_dn_,
                                   rlc_tx_upper_layer_control_notifier& upper_cn_,
                                   rlc_tx_lower_layer_notifier&         lower_dn_,
                                   rlc_metrics_aggregator&              metrics_agg_,
                                   rlc_pcap&                            pcap_,
                                   task_executor&                       pcell_executor_,
                                   task_executor&                       ue_executor_,
                                   timer_manager&                       timers) :
  rlc_tx_entity(du_id_,
                ue_index_,
                rb_id_,
                upper_dn_,
                upper_cn_,
                lower_dn_,
                metrics_agg_,
                pcap_,
                pcell_executor_,
                ue_executor_,
                timers),
  cfg(config),
  sdu_queue(cfg.queue_size, cfg.queue_size_bytes, logger),
  pcap_context(ue_index_, rb_id_, /* is_uplink */ false)
{
  metrics_low.metrics_set_mode(rlc_mode::tm);
  logger.log_info("RLC TM created. {}", cfg);

#ifdef JBPF_ENABLED
  CALL_JBPF_HOOK(hook_rlc_dl_creation);
#endif
}

// TS 38.322 v16.2.0 Sec. 5.2.1.1
void rlc_tx_tm_entity::handle_sdu(byte_buffer sdu_buf, bool is_retx)
{
  rlc_sdu sdu_;

  sdu_.buf = std::move(sdu_buf);

  // Sanity check for PDCP ReTx in RLC TM
  if (SRSRAN_UNLIKELY(is_retx)) {
    logger.log_error("Ignored unexpected PDCP retransmission flag in RLC TM SDU");
  }

  size_t sdu_len = sdu_.buf.length();
  if (sdu_queue.write(sdu_)) {
    logger.log_info(sdu_.buf.begin(), sdu_.buf.end(), "TX SDU. sdu_len={} {}", sdu_len, sdu_queue.get_state());
    metrics_high.metrics_add_sdus(1, sdu_len);
    handle_changed_buffer_state();
  } else {
    logger.log_info("Dropped SDU. sdu_len={} {}", sdu_len, sdu_queue.get_state());
    metrics_high.metrics_add_lost_sdus(1);
  }

#ifdef JBPF_ENABLED
  CALL_JBPF_HOOK(hook_rlc_dl_new_sdu, sdu_.buf.length(), 
      sdu_.pdcp_sn.has_value() ? sdu_.pdcp_sn.value() : 0, false);
#endif

}

// TS 38.322 v16.2.0 Sec. 5.4
void rlc_tx_tm_entity::discard_sdu(uint32_t pdcp_sn)
{
#ifdef JBPF_ENABLED
  CALL_JBPF_HOOK(hook_rlc_dl_discard_sdu, pdcp_sn, false);
#endif

  logger.log_warning("Ignoring invalid attempt to discard SDU in TM. pdcp_sn={}", pdcp_sn);
  metrics_high.metrics_add_discard_failure(1);
}

// TS 38.322 v16.2.0 Sec. 5.2.1.1
size_t rlc_tx_tm_entity::pull_pdu(span<uint8_t> mac_sdu_buf)
{
  size_t grant_len = mac_sdu_buf.size();
  logger.log_debug("MAC opportunity. grant_len={}", grant_len);
  std::chrono::time_point<std::chrono::steady_clock> pull_begin;
  if (metrics_high.is_enabled()) {
    pull_begin = std::chrono::steady_clock::now();
  }

  // Get a new SDU, if none is currently being transmitted
  if (sdu.buf.empty()) {
    if (not sdu_queue.read(sdu)) {
      logger.log_debug("SDU queue empty. grant_len={}", grant_len);
      return 0;
    }
    logger.log_debug("Read SDU. pdcp_sn={} sdu_len={}", sdu.pdcp_sn, sdu.buf.length());
  }

  size_t sdu_len = sdu.buf.length();
  if (sdu_len > grant_len) {
    logger.log_info("SDU exeeds provided space. front_len={} grant_len={}", sdu_len, grant_len);
    metrics_low.metrics_add_small_alloc(1);
    return 0;
  }

  // Notify the upper layer about the beginning of the transfer of the current SDU
  if (sdu.pdcp_sn.has_value()) {

#ifdef JBPF_ENABLED
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() -
                                                                      sdu.time_of_arrival);
    CALL_JBPF_HOOK(hook_rlc_dl_sdu_send_started, sdu.pdcp_sn.value(), false, 
                   (uint64_t)latency.count());
#endif

    // The desired_buf_size is irrelevant for TM. Nevertheless we put the size of the SDU queue here.
    upper_dn.on_transmitted_sdu(sdu.pdcp_sn.value(), cfg.queue_size_bytes);
  }

  // In TM there is no header, just pass the plain SDU
  auto* out_it = mac_sdu_buf.begin();
  for (span<uint8_t> seg : sdu.buf.segments()) {
    out_it = std::copy(seg.begin(), seg.end(), out_it);
  }
  size_t pdu_len = out_it - mac_sdu_buf.begin();
  logger.log_info(mac_sdu_buf.data(), pdu_len, "TX PDU. pdu_len={} grant_len={}", pdu_len, grant_len);

  // Release SDU
  sdu.buf.clear();

  // Push PDU into PCAP.
  pcap.push_pdu(pcap_context, mac_sdu_buf.subspan(0, pdu_len));

  // Update metrics
  metrics_low.metrics_add_pdus_no_segmentation(1, sdu_len);
  if (metrics_low.is_enabled()) {
    std::chrono::time_point pull_end   = std::chrono::steady_clock::now();
    auto                    pull_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(pull_end - pull_begin);
    metrics_low.metrics_add_pdu_latency_ns(pull_delta.count());
  }

#ifdef JBPF_ENABLED
  auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() -
                                                                      sdu.time_of_arrival);
  CALL_JBPF_HOOK(hook_rlc_dl_sdu_send_completed, sdu.pdcp_sn.has_value() ? sdu.pdcp_sn.value() : 0, false,
    (uint64_t)latency.count());
  CALL_JBPF_HOOK(hook_rlc_dl_tx_pdu, JBPF_RLC_PDUTYPE_DATA, (uint32_t)pdu_len);
#endif

  return pdu_len;
}

void rlc_tx_tm_entity::handle_changed_buffer_state()
{
  if (not pending_buffer_state.test_and_set(std::memory_order_seq_cst)) {
    logger.log_debug("Triggering buffer state update to lower layer");
    // Redirect handling of status to pcell_executor
    if (not pcell_executor.defer([this]() { update_mac_buffer_state(); })) {
      logger.log_error("Failed to enqueue buffer state update");
    }
  } else {
    logger.log_debug("Avoiding redundant buffer state update to lower layer");
  }
}

void rlc_tx_tm_entity::update_mac_buffer_state()
{
  pending_buffer_state.clear(std::memory_order_seq_cst);
  unsigned bs = get_buffer_state();
  logger.log_debug("Sending buffer state update to lower layer. bs={}", bs);
  lower_dn.on_buffer_state_update(bs);
}

uint32_t rlc_tx_tm_entity::get_buffer_state()
{
  return sdu_queue.get_state().n_bytes + sdu.buf.length();
}
