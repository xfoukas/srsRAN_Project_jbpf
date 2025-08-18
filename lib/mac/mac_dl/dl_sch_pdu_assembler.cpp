/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
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

#include "dl_sch_pdu_assembler.h"
#include "cell_dl_harq_buffer_pool.h"
#include "srsran/mac/mac_pdu_format.h"
#include "srsran/ran/pdsch/pdsch_constants.h"
#include "srsran/scheduler/result/pdsch_info.h"
#include "srsran/support/error_handling.h"
#include "srsran/support/format/fmt_to_c_str.h"

using namespace srsran;

/// Minimum size required to fit a MAC subheader and SDU.
static constexpr size_t min_mac_subhdr_and_sdu_space_required(lcid_t lcid)
{
  /// Note: Do not attempt to build an SDU if there is not enough space for the MAC subheader, min payload size and
  /// potential RLC header.
  constexpr size_t RLC_HEADER_SIZE_ESTIM = 2;

  return MIN_MAC_SDU_SUBHEADER_SIZE + 1 + (lcid != LCID_SRB0 ? RLC_HEADER_SIZE_ESTIM : 0);
}

dl_sch_pdu::mac_sdu_encoder::mac_sdu_encoder(dl_sch_pdu& pdu_, lcid_t lcid_, unsigned max_sdu_size_) :
  pdu(&pdu_),
  lcid(lcid_),
  // Predict whether a MAC subheader with 8-bit or 16-bit L field is required based on the MAC opportunity size.
  // Note: This L value may be revisited later, if the opportunity was larger than 256 bytes and the actual SDU
  // provided by upper layers is shorter than 256 bytes.
  subhr_len(get_mac_sdu_subheader_size(max_sdu_size_)),
  max_sdu_size(max_sdu_size_)
{
}

span<uint8_t> dl_sch_pdu::mac_sdu_encoder::sdu_buffer() const
{
  if (pdu == nullptr) {
    return {};
  }
  unsigned sdu_offset = pdu->byte_offset + subhr_len;
  return pdu->pdu.subspan(sdu_offset, max_sdu_size);
}

unsigned dl_sch_pdu::mac_sdu_encoder::encode_sdu(unsigned sdu_bytes_written)
{
  srsran_assert(pdu != nullptr, "encode_sdu called for invalid MAC grant");
  if (sdu_bytes_written == 0 or sdu_bytes_written > max_sdu_size) {
    return 0;
  }

  // Check if we need to change the subheader size because the MAC SDU was smaller than expected.
  // Note: This is important because some basebands reject the message if the PDU could be enclosed by the shorter
  // subheader.
  if (get_mac_sdu_subheader_size(sdu_bytes_written) != subhr_len) {
    srsran_sanity_check(subhr_len == MAX_MAC_SDU_SUBHEADER_SIZE, "Unexpected subheader size change");

    // Shift bytes to the left by 1 position.
    uint8_t* new_start = pdu->pdu.data() + pdu->byte_offset + MIN_MAC_SDU_SUBHEADER_SIZE;
    uint8_t* old_start = new_start + 1;
    std::copy(old_start, old_start + sdu_bytes_written, new_start);

    // Update the subheader size.
    subhr_len = MIN_MAC_SDU_SUBHEADER_SIZE;
  }

  // Encode MAC SubHeader.
  const bool F_bit = subhr_len > MIN_MAC_SDU_SUBHEADER_SIZE;
  pdu->encode_subheader(F_bit, lcid, subhr_len, sdu_bytes_written);

  // Advance byte offset to account for MAC SDU.
  pdu->byte_offset += sdu_bytes_written;

  return subhr_len + sdu_bytes_written;
}

dl_sch_pdu::mac_sdu_encoder dl_sch_pdu::get_sdu_encoder(lcid_t lcid, unsigned sdu_payload_len_estimate)
{
  if (sdu_payload_len_estimate == 0) {
    return mac_sdu_encoder{};
  }

  unsigned rem_grant_space = pdu.size() - byte_offset;
  if (rem_grant_space < min_mac_subhdr_and_sdu_space_required(lcid)) {
    // No space available for the smallest possible MAC SDU.
    return mac_sdu_encoder{};
  }

  // Space to be used to encode the MAC subheader + SDU (expected sizes).
  unsigned space_estim = std::min(rem_grant_space, get_mac_sdu_required_bytes(sdu_payload_len_estimate));
  // Space to encode expected MAC SDU without subheader.
  unsigned sdu_space_estim = get_mac_sdu_payload_size(space_estim);

  return mac_sdu_encoder{*this, lcid, sdu_space_estim};
}

unsigned dl_sch_pdu::add_sdu(lcid_t lcid, span<uint8_t> sdu)
{
  srsran_assert(not sdu.empty(), "Trying to add an empty SDU");

  mac_sdu_encoder sdu_enc = get_sdu_encoder(lcid, sdu.size());
  if (not sdu_enc.valid()) {
    return 0;
  }

  // Copy SDU payload.
  std::copy(sdu.begin(), sdu.end(), sdu_enc.sdu_buffer().begin());

  // Encode subheader.
  return sdu_enc.encode_sdu(sdu.size());
}

unsigned dl_sch_pdu::add_sdu(lcid_t lcid, const byte_buffer& sdu)
{
  srsran_assert(not sdu.empty(), "Trying to add an empty SDU");

  size_t sdu_len = sdu.length();

  mac_sdu_encoder sdu_enc = get_sdu_encoder(lcid, sdu_len);
  if (not sdu_enc.valid()) {
    return 0;
  }

  // Copy SDU payload.
  span<uint8_t> sdu_buf = sdu_enc.sdu_buffer();
  unsigned      offset  = 0;
  for (span<const uint8_t> seg : sdu.segments()) {
    std::memcpy(sdu_buf.data() + offset, seg.data(), seg.size());
    offset += seg.size();
  }
  srsran_sanity_check(offset == sdu_len, "Error while copying SDU payload");

  // Encode subheader.
  return sdu_enc.encode_sdu(sdu_len);
}

void dl_sch_pdu::add_ue_con_res_id(const ue_con_res_id_t& con_res_payload)
{
  lcid_dl_sch_t lcid        = lcid_dl_sch_t::UE_CON_RES_ID;
  unsigned      header_len  = 1;
  unsigned      payload_len = lcid.sizeof_ce();

  // Encode header and payload.
  encode_subheader(false, lcid_dl_sch_t::UE_CON_RES_ID, header_len, payload_len);

  // Encode UE ConRes ID.
  std::copy(con_res_payload.begin(), con_res_payload.end(), pdu.data() + byte_offset);
  byte_offset += con_res_payload.size();
}

void dl_sch_pdu::add_tag_cmd(const ta_cmd_ce_payload& ce_payload)
{
  const lcid_dl_sch_t lcid        = lcid_dl_sch_t::TA_CMD;
  const unsigned      header_len  = 1;
  const unsigned      payload_len = lcid.sizeof_ce();

  // Encode header and payload.
  encode_subheader(false, lcid_dl_sch_t::TA_CMD, header_len, payload_len);

  // Encode Timing Advance Command.
  pdu[byte_offset++] = (ce_payload.tag_id.value() & 0xc0U) | (ce_payload.ta_cmd & 0x3fU);
}

void dl_sch_pdu::add_padding(unsigned len)
{
  // 1 Byte R/LCID MAC subheader.
  unsigned header_len = 1;
  unsigned sdu_len    = len - 1;
  encode_subheader(false, lcid_dl_sch_t::PADDING, header_len, sdu_len);

  // Padding bits.
  std::fill(pdu.data() + byte_offset, pdu.data() + byte_offset + sdu_len, 0);
  byte_offset += sdu_len;
}

void dl_sch_pdu::encode_subheader(bool F_bit, lcid_dl_sch_t lcid, unsigned header_len, unsigned payload_len)
{
  pdu[byte_offset++] = ((F_bit ? 1U : 0U) << 6U) | (lcid.value() & 0x3fU);
  if (header_len == 3) {
    // 3 Byte R/F/LCID/L MAC subheader with 16-bit L field.
    pdu[byte_offset++] = (payload_len & 0xff00U) >> 8U;
    pdu[byte_offset++] = (payload_len & 0xffU);
    return;
  }
  if (header_len == 2) {
    // 2 Byte R/F/LCID/L MAC subheader with 8-bit L field.
    pdu[byte_offset++] = payload_len & 0xffU;
    return;
  }
  if (header_len == 1) {
    // Do nothing.
    return;
  }
  report_fatal_error("Error while packing PDU. Unsupported header length ({})", header_len);
}

// /////////////////////////

class dl_sch_pdu_assembler::pdu_log_builder
{
public:
  pdu_log_builder(du_ue_index_t         ue_index_,
                  rnti_t                rnti_,
                  units::bytes          tbs_,
                  fmt::memory_buffer&   fmtbuf_,
                  srslog::basic_logger& logger_) :
    ue_index(ue_index_), rnti(rnti_), tbs(tbs_), logger(logger_), fmtbuf(fmtbuf_), enabled(logger.info.enabled())
  {
    fmtbuf.clear();
  }

  void add_sdu(lcid_t lcid, unsigned len)
  {
    if (not enabled) {
      return;
    }
    if (lcid != current_sdu_lcid) {
      if (current_sdu_lcid != lcid_t::INVALID_LCID) {
        fmt::format_to(std::back_inserter(fmtbuf),
                       "{}SDU: lcid={} nof_sdus={} total_size={}",
                       separator(),
                       fmt::underlying(current_sdu_lcid),
                       nof_sdus,
                       sum_bytes);
      }
      current_sdu_lcid = lcid;
      nof_sdus         = 1;
      sum_bytes        = units::bytes{len};
    } else {
      ++nof_sdus;
      sum_bytes += units::bytes{len};
    }
  }

  void add_conres_id(const ue_con_res_id_t& conres)
  {
    if (not enabled) {
      return;
    }
    fmt::format_to(std::back_inserter(fmtbuf), "{}CON_RES: id={:x}", separator(), fmt::join(conres, ""));
  }

  void add_ta_cmd(const ta_cmd_ce_payload& ce_payload)
  {
    if (not enabled) {
      return;
    }
    fmt::format_to(std::back_inserter(fmtbuf),
                   "{}TA_CMD: tag_id={}, ta_cmd={}",
                   separator(),
                   ce_payload.tag_id.value(),
                   ce_payload.ta_cmd);
  }

  void log()
  {
    if (not enabled) {
      return;
    }

    // Log pending LCID SDUs.
    if (current_sdu_lcid != lcid_t::INVALID_LCID) {
      fmt::format_to(std::back_inserter(fmtbuf),
                     "{}SDU: lcid={} nof_sdus={} total_size={}",
                     separator(),
                     fmt::underlying(current_sdu_lcid),
                     nof_sdus,
                     sum_bytes);
    }

    logger.info("DL PDU: ue={} rnti={} size={}: {}", fmt::underlying(ue_index), rnti, tbs, to_c_str(fmtbuf));
  }

private:
  const char* separator() const { return fmtbuf.size() == 0 ? "" : ", "; }

  du_ue_index_t ue_index;
  rnti_t        rnti;
  units::bytes  tbs;

  srslog::basic_logger& logger;
  fmt::memory_buffer&   fmtbuf;
  const bool            enabled;

  lcid_t       current_sdu_lcid = lcid_t::INVALID_LCID;
  unsigned     nof_sdus         = 0;
  units::bytes sum_bytes{0U};
};

// /////////////////////////

dl_sch_pdu_assembler::dl_sch_pdu_assembler(mac_dl_ue_repository&     ue_mng_,
                                           cell_dl_harq_buffer_pool& cell_dl_harq_buffers) :
  ue_mng(ue_mng_), harq_buffers(cell_dl_harq_buffers), logger(srslog::fetch_basic_logger("MAC"))
{
}

/// Buffer passed to the lower layers when HARQ allocation fails.
static const std::vector<uint8_t> zero_buffer(MAX_DL_PDU_LENGTH, 0);

/// Returns a shared_transport_block that holds a buffer filled with zeros.
static shared_transport_block make_shared_zero_buffer(size_t sz)
{
  return shared_transport_block(span<const uint8_t>(zero_buffer.data(), sz));
}

shared_transport_block dl_sch_pdu_assembler::assemble_newtx_pdu(rnti_t                rnti,
                                                                harq_id_t             h_id,
                                                                unsigned              tb_idx,
                                                                const dl_msg_tb_info& tb_info,
                                                                unsigned              tb_size_bytes)
{
  du_ue_index_t ue_idx = ue_mng.get_ue_index(rnti);
  if (ue_idx == INVALID_DU_UE_INDEX) {
    logger.error("DL rnti={} h_id={}: Failed to assemble MAC PDU. Cause: C-RNTI has no associated UE id.",
                 rnti,
                 fmt::underlying(h_id));
    return make_shared_zero_buffer(tb_size_bytes);
  }

  auto shared_buffer = harq_buffers.allocate_dl_harq_buffer(ue_idx, h_id);
  if (not shared_buffer or shared_buffer->get_buffer().size() < tb_size_bytes) {
    logger.warning("DL ue={} rnti={} h_id={}: Failed to assemble MAC PDU. Cause: No HARQ buffers available",
                   fmt::underlying(ue_idx),
                   rnti,
                   fmt::underlying(h_id));
    return make_shared_zero_buffer(tb_size_bytes);
  }
  dl_sch_pdu ue_pdu(shared_buffer->get_buffer().first(tb_size_bytes));

  pdu_log_builder pdu_logger{ue_idx, rnti, units::bytes{tb_size_bytes}, fmtbuf, logger};

  // Encode added subPDUs.
  for (const dl_msg_lc_info& sched_lch : tb_info.lc_chs_to_sched) {
    if (sched_lch.lcid.is_sdu()) {
      assemble_sdus(ue_pdu, rnti, sched_lch, pdu_logger);
    } else {
      assemble_ce(ue_pdu, rnti, sched_lch, pdu_logger);
    }
  }

  // Add Padding if required.
  unsigned current_size = ue_pdu.nof_bytes();
  if (current_size < tb_size_bytes) {
    ue_pdu.add_padding(tb_size_bytes - current_size);
  } else if (current_size > tb_size_bytes) {
    logger.error("ERROR: Allocated subPDUs exceed TB size ({} > {})", current_size, tb_size_bytes);
    return make_shared_zero_buffer(tb_size_bytes);
  }

  pdu_logger.log();

  return shared_buffer->transfer_to_buffer_view(ue_pdu.nof_bytes());
}

void dl_sch_pdu_assembler::assemble_sdus(dl_sch_pdu&           ue_pdu,
                                         rnti_t                rnti,
                                         const dl_msg_lc_info& lc_grant_info,
                                         pdu_log_builder&      pdu_logger)
{
  // Fetch RLC Bearer.
  const lcid_t        lcid   = lc_grant_info.lcid.to_lcid();
  mac_sdu_tx_builder* bearer = ue_mng.get_lc_sdu_builder(rnti, lcid);
  srsran_sanity_check(bearer != nullptr, "Scheduler is allocating inexistent bearers");

  const unsigned total_space =
      std::min(get_mac_sdu_required_bytes(lc_grant_info.sched_bytes), ue_pdu.nof_empty_bytes());
  unsigned rem_bytes        = total_space;
  size_t   min_mac_sdu_size = min_mac_subhdr_and_sdu_space_required(lcid);
  while (rem_bytes >= min_mac_sdu_size) {
    // Get MAC opportunity size based on the remaining bytes to encode for this LCID.
    const unsigned mac_opportunity_size = get_mac_sdu_payload_size(rem_bytes);

    // Setup a SDU encoder.
    dl_sch_pdu::mac_sdu_encoder sdu_enc = ue_pdu.get_sdu_encoder(lcid, mac_opportunity_size);
    if (not sdu_enc.valid()) {
      logger.info("ue={} rnti={} lcid={}: Insufficient MAC opportunity size={}. Remaining space in PDU={}",
                  fmt::underlying(ue_mng.get_ue_index(rnti)),
                  rnti,
                  fmt::underlying(lcid),
                  mac_opportunity_size,
                  rem_bytes);
      break;
    }

    // Fetch MAC Tx SDU from upper layers and write inplace, into the buffer provided by the SDU encoder.
    size_t sdu_actual_len = bearer->on_new_tx_sdu(sdu_enc.sdu_buffer());
    if (sdu_actual_len == 0) {
      // The RLC Tx window is full or the RLC buffer is empty.
      logger.debug("ue={} rnti={} lcid={}: Unable to encode MAC SDU in MAC opportunity of size={}.",
                   fmt::underlying(ue_mng.get_ue_index(rnti)),
                   rnti,
                   fmt::underlying(lcid),
                   mac_opportunity_size);
      break;
    }

    // Encode MAC subheader + SDU.
    size_t subh_and_sdu_size = sdu_enc.encode_sdu(sdu_actual_len);
    if (subh_and_sdu_size == 0) {
      logger.error("ue={} rnti={} lcid={}: Scheduled SDU with size={} cannot fit in scheduled DL grant",
                   fmt::underlying(ue_mng.get_ue_index(rnti)),
                   rnti,
                   fmt::underlying(lc_grant_info.lcid.to_lcid()),
                   lc_grant_info.sched_bytes);
      break;
    }
    srsran_assert(rem_bytes >= subh_and_sdu_size, "Too many bytes were packed in MAC SDU");

    // Log SDU.
    pdu_logger.add_sdu(lc_grant_info.lcid.to_lcid(), subh_and_sdu_size);

    rem_bytes -= subh_and_sdu_size;
  }

  if (rem_bytes == total_space) {
    // No SDU was encoded for this LCID.
    // Causes for failure to create MAC SDU include: RLC Tx window is full, mismatch between the logical channel buffer
    // states in the scheduler and RLC bearers, or the MAC opportunity is too small.
    if (rem_bytes < min_mac_sdu_size) {
      logger.warning("ue={} rnti={} lcid={}: Skipping MAC SDU encoding into PDU of {} bytes ({} available). Cause: "
                     "Allocated SDU size={} is too small.",
                     fmt::underlying(ue_mng.get_ue_index(rnti)),
                     rnti,
                     fmt::underlying(lc_grant_info.lcid.to_lcid()),
                     ue_pdu.capacity(),
                     ue_pdu.nof_empty_bytes(),
                     lc_grant_info.sched_bytes);
    } else {
      logger.info("ue={} rnti={} lcid={}: Skipping MAC SDU encoding. Cause: RLC could not encode any SDU",
                  fmt::underlying(ue_mng.get_ue_index(rnti)),
                  rnti,
                  fmt::underlying(lc_grant_info.lcid.to_lcid()));
    }
  }
}

void dl_sch_pdu_assembler::assemble_ce(dl_sch_pdu&           ue_pdu,
                                       rnti_t                rnti,
                                       const dl_msg_lc_info& subpdu,
                                       pdu_log_builder&      pdu_logger)
{
  switch (subpdu.lcid.value()) {
    case lcid_dl_sch_t::UE_CON_RES_ID: {
      ue_con_res_id_t conres = ue_mng.get_con_res_id(rnti);
      std::copy(conres.begin(), conres.end(), conres.begin());
      ue_pdu.add_ue_con_res_id(conres);
      pdu_logger.add_conres_id(conres);
    } break;
    case lcid_dl_sch_t::TA_CMD: {
      srsran_assert(std::holds_alternative<ta_cmd_ce_payload>(subpdu.ce_payload) == true,
                    "Invalid MAC CE payload for lcid={}",
                    subpdu.lcid.value());
      const auto ce_payload = std::get<ta_cmd_ce_payload>(subpdu.ce_payload);
      ue_pdu.add_tag_cmd(ce_payload);
      pdu_logger.add_ta_cmd(ce_payload);
    } break;
    default:
      report_fatal_error("Invalid MAC CE lcid={}", subpdu.lcid);
  }
}

shared_transport_block
dl_sch_pdu_assembler::assemble_retx_pdu(rnti_t rnti, harq_id_t h_id, unsigned tb_idx, unsigned tbs_bytes)
{
  du_ue_index_t ue_idx = ue_mng.get_ue_index(rnti);
  if (ue_idx == INVALID_DU_UE_INDEX) {
    logger.error("DL rnti={} h_id={}: Failed to assemble MAC PDU. Cause: C-RNTI has no associated UE id.",
                 rnti,
                 fmt::underlying(h_id));
    return make_shared_zero_buffer(tbs_bytes);
  }

  auto shared_buffer = harq_buffers.allocate_dl_harq_buffer(ue_idx, h_id);
  if (not shared_buffer or shared_buffer->get_buffer().size() < tbs_bytes) {
    logger.warning("DL ue={} rnti={} h_id={}: Failed to assemble MAC PDU. Cause: No HARQ buffers available",
                   fmt::underlying(ue_mng.get_ue_index(rnti)),
                   rnti,
                   fmt::underlying(h_id));
    return make_shared_zero_buffer(tbs_bytes);
  }

  return shared_buffer->transfer_to_buffer_view(tbs_bytes);
}
