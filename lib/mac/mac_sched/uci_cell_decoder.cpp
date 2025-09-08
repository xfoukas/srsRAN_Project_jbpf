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

#include "uci_cell_decoder.h"
#include "srsran/ran/csi_report/csi_report_on_pucch_helpers.h"
#include "srsran/scheduler/resource_grid_util.h"
#include "srsran/scheduler/result/pucch_info.h"
#include "srsran/scheduler/result/pusch_info.h"

using namespace srsran;

/// \brief Size, in number of slots, of the ring buffer used to store the pending UCIs to be decoded. This size
/// should account for potential latencies in the PHY in forwarding the decoded UCI to the MAC.
static size_t get_ring_size(const sched_cell_configuration_request_message& cell_cfg)
{
  // Estimation of the time it takes the UL lower-layers to process and forward CRC/UCI indications.
  // Note: The size of this ring has to be larger than that of the test mode internal buffer.
  static constexpr unsigned MAX_UL_PHY_DELAY = 80;
  // Note: The history ring size has to be a multiple of the TDD frame size in slots.
  // Number of slots managed by this container.
  return get_allocator_ring_size_gt_min(get_max_slot_ul_alloc_delay(cell_cfg.ntn_cs_koffset) + MAX_UL_PHY_DELAY);
}

uci_cell_decoder::uci_cell_decoder(const sched_cell_configuration_request_message& cell_cfg,
                                   const du_rnti_table&                            rnti_table_,
                                   rlf_detector&                                   rlf_hdlr_) :
  rnti_table(rnti_table_),
  cell_index(cell_cfg.cell_index),
  rlf_handler(rlf_hdlr_),
  logger(srslog::fetch_basic_logger("MAC")),
  expected_uci_report_grid(get_ring_size(cell_cfg))
{
}

static auto convert_mac_harq_bits_to_sched_harq_values(bool harq_status,
                                                       const bounded_bitset<uci_constants::MAX_NOF_HARQ_BITS>& payload)
{
  harq_ack_report_list harqs(payload.size(),
                             harq_status ? mac_harq_ack_report_status::nack : mac_harq_ack_report_status::dtx);
  if (harq_status) {
    for (unsigned i = 0, e = harqs.size(); i != e; ++i) {
      if (payload.test(i)) {
        harqs[i] = srsran::mac_harq_ack_report_status::ack;
      }
    }
  }
  return harqs;
}

static csi_report_data decode_csi(const bounded_bitset<uci_constants::MAX_NOF_CSI_PART1_OR_PART2_BITS>& payload,
                                  const csi_report_configuration&                                       csi_rep_cfg)
{
  // Convert UCI CSI1 bits to "csi_report_packed".
  csi_report_packed csi_bits(payload.size());
  for (unsigned k = 0; k != csi_bits.size(); ++k) {
    csi_bits.set(k, payload.test(k));
  }

  return csi_report_unpack_pucch(csi_bits, csi_rep_cfg);
}

static std::optional<csi_report_data> decode_csi_bits(const mac_uci_pdu::pucch_f2_or_f3_or_f4_type& pucch,
                                                      const csi_report_configuration&               csi_rep_cfg)
{
  // TODO: Handle CSI part 2.
  return decode_csi(pucch.csi_part1_info->payload, csi_rep_cfg);
}

static std::optional<csi_report_data> decode_csi_bits(const mac_uci_pdu::pusch_type&  pusch,
                                                      const csi_report_configuration& csi_rep_cfg)
{
  // TODO: Revisit this logic since its valid only for periodic CSI reporting and for both Type I and Type II reports
  //  configured for PUCCH but transmitted on PUSCH"
  // TODO: Handle CSI part 2.
  return decode_csi(pusch.csi_part1_info->payload, csi_rep_cfg);
}

uci_indication uci_cell_decoder::decode_uci(const mac_uci_indication_message& msg)
{
  // Convert MAC UCI indication to srsRAN scheduler UCI indication.
  uci_indication ind{};
  ind.slot_rx    = msg.sl_rx;
  ind.cell_index = cell_index;
  for (const auto& mac_uci : msg.ucis) {
    uci_indication::uci_pdu& uci_pdu = ind.ucis.emplace_back();
    uci_pdu.crnti                    = mac_uci.rnti;
    uci_pdu.ue_index                 = rnti_table[mac_uci.rnti];
    if (uci_pdu.ue_index == INVALID_DU_UE_INDEX) {
      ind.ucis.pop_back();
      logger.info("rnti={}: Discarding UCI PDU. Cause: No UE with provided RNTI exists.", uci_pdu.crnti);
      continue;
    }

    if (const auto* pucch_f0f1 = std::get_if<mac_uci_pdu::pucch_f0_or_f1_type>(&mac_uci.pdu)) {
      uci_indication::uci_pdu::uci_pucch_f0_or_f1_pdu pdu{};

      pdu.ul_sinr_dB          = pucch_f0f1->ul_sinr_dB;
      pdu.time_advance_offset = pucch_f0f1->time_advance_offset;
      pdu.sr_detected         = false;
      if (pucch_f0f1->sr_info.has_value()) {
        pdu.sr_detected = pucch_f0f1->sr_info.value().detected;
      }
      if (pucch_f0f1->harq_info.has_value()) {
        // NOTES:
        // - We report to the scheduler only the UCI HARQ-ACKs that contain either an ACK or NACK; we ignore the
        // UCIs with DTX. In that case, the scheduler will not receive the notification and the HARQ will eventually
        // retransmit the packet.
        // - This is to handle the case of simultaneous SR + HARQ UCI, for which we receive 2 UCI PDUs from the PHY,
        // 1 for SR + HARQ, 1 for HARQ only; note that only the SR + HARQ UCI is filled by the UE, meaning that we
        // expect the received HARQ-only UCI to be DTX. If reported to the scheduler, the UCI with HARQ-ACK only would
        // be erroneously treated as a NACK (as the scheduler only accepts ACK or NACK).

        // NOTE: There is a potential error that need to be handled below, which occurs when there's the 2-bit report
        // {DTX, (N)ACK}; if this were reported, we would skip the first bit (i.e. DTX) and report the second (i.e.
        // (N)ACK). Since in the scheduler the HARQ-ACK bits for a given UCI are processed in sequence, the
        // notification of the second bit of {DTX, (N)ACK} would be seen by the scheduler as the first bit of the
        // expected 2-bit reporting. To prevent this, we assume that PUCCH Format 0 or 1 UCI is valid if none of the 1
        // or 2 bits report is DTX (not detected).

        const auto& harq_pdus = pucch_f0f1->harq_info.value().harqs;
        pdu.harqs.resize(harq_pdus.size());
        for (unsigned i = 0, e = pdu.harqs.size(); i != e; ++i) {
          switch (harq_pdus[i]) {
            case uci_pucch_f0_or_f1_harq_values::ack:
              pdu.harqs[i] = mac_harq_ack_report_status::ack;
              break;
            case uci_pucch_f0_or_f1_harq_values::nack:
              pdu.harqs[i] = mac_harq_ack_report_status::nack;
              break;
            default:
              pdu.harqs[i] = mac_harq_ack_report_status::dtx;
          }

          // Report ACK for RLF detection purposes.
          rlf_handler.handle_ack(uci_pdu.ue_index, cell_index, pdu.harqs[i] == mac_harq_ack_report_status::ack);
        }
      }
      uci_pdu.pdu.emplace<uci_indication::uci_pdu::uci_pucch_f0_or_f1_pdu>(pdu);
    } else if (const auto* pusch = std::get_if<mac_uci_pdu::pusch_type>(&mac_uci.pdu)) {
      auto& pdu = uci_pdu.pdu.emplace<uci_indication::uci_pdu::uci_pusch_pdu>();
      if (pusch->harq_info.has_value()) {
        pdu.harqs =
            convert_mac_harq_bits_to_sched_harq_values(pusch->harq_info.value().is_valid, pusch->harq_info->payload);

        // Report ACK for RLF detection purposes.
        for (mac_harq_ack_report_status harq : pdu.harqs) {
          rlf_handler.handle_ack(uci_pdu.ue_index, cell_index, harq == mac_harq_ack_report_status::ack);
        }
      }

      if (pusch->csi_part1_info.has_value()) {
        if (pusch->csi_part1_info->is_valid) {
          // Decode CSI bits given the CSI report config previously stored in the grid.
          const auto& slot_ucis = expected_uci_report_grid[to_grid_index(msg.sl_rx)];

          // Search for CSI report config with matching RNTI.
          for (const auto& expected_slot_uci : slot_ucis) {
            if (expected_slot_uci.rnti == uci_pdu.crnti) {
              pdu.csi = decode_csi_bits(*pusch, expected_slot_uci.csi_rep_cfg);
              break;
            }
          }
          if (not pdu.csi.has_value()) {
            logger.warning("cell={} ue={} rnti={}: Discarding CSI report. Cause: Unable to find CSI report config.",
                           fmt::underlying(cell_index),
                           fmt::underlying(uci_pdu.ue_index),
                           uci_pdu.crnti);
          }
        } else {
          pdu.csi = csi_report_data{.valid = false};
        }

        // NOTE: The RLF detection based on CSI is used when the UE only transmits PUCCHs; if the UE transmit PUSCHs,
        // the RLF detection will be based on the PUSCH CRC. However, if the PUSCH UCI has a correctly decoded CSI, we
        // need to reset the CSI KOs counter.
        if (pusch->csi_part1_info->is_valid) {
          rlf_handler.handle_csi(uci_pdu.ue_index, cell_index, true);
        }
      }

    } else if (const auto* pucch_f2f3f4 = std::get_if<mac_uci_pdu::pucch_f2_or_f3_or_f4_type>(&mac_uci.pdu)) {
      auto& pdu = uci_pdu.pdu.emplace<uci_indication::uci_pdu::uci_pucch_f2_or_f3_or_f4_pdu>();

      pdu.ul_sinr_dB          = pucch_f2f3f4->ul_sinr_dB;
      pdu.time_advance_offset = pucch_f2f3f4->time_advance_offset;
      if (pucch_f2f3f4->sr_info.has_value()) {
        pdu.sr_info = pucch_f2f3f4->sr_info.value();
      }
      if (pucch_f2f3f4->harq_info.has_value()) {
        pdu.harqs = convert_mac_harq_bits_to_sched_harq_values(pucch_f2f3f4->harq_info.value().is_valid,
                                                               pucch_f2f3f4->harq_info->payload);

        // Report ACK for RLF detection purposes.
        for (mac_harq_ack_report_status harq_st : pdu.harqs) {
          rlf_handler.handle_ack(uci_pdu.ue_index, cell_index, harq_st == mac_harq_ack_report_status::ack);
        }
      }

      // Check if the UCI has been correctly decoded.
      if (pucch_f2f3f4->csi_part1_info.has_value()) {
        if (pucch_f2f3f4->csi_part1_info->is_valid) {
          // Decode CSI bits given the CSI report config previously stored in the grid.
          const auto& slot_ucis = expected_uci_report_grid[to_grid_index(msg.sl_rx)];

          // Search for CSI report config with matching RNTI.
          for (const auto& expected_slot_uci : slot_ucis) {
            if (expected_slot_uci.rnti == uci_pdu.crnti) {
              pdu.csi = decode_csi_bits(*pucch_f2f3f4, expected_slot_uci.csi_rep_cfg);
              break;
            }
          }
          if (not pdu.csi.has_value()) {
            logger.warning("cell={} ue={} rnti={}: Discarding CSI report. Cause: Unable to find CSI report config.",
                           fmt::underlying(cell_index),
                           fmt::underlying(uci_pdu.ue_index),
                           uci_pdu.crnti);
          }
        } else {
          pdu.csi = csi_report_data{.valid = false};
        }

        // We consider any status other than "crc_pass" as non-decoded CSI.
        rlf_handler.handle_csi(uci_pdu.ue_index, cell_index, pucch_f2f3f4->csi_part1_info->is_valid);
      }
    }
  }

  return ind;
}

void uci_cell_decoder::store_uci(slot_point                uci_sl,
                                 span<const pucch_info>    scheduled_pucchs,
                                 span<const ul_sched_info> scheduled_puschs)
{
  auto& slot_ucis = expected_uci_report_grid[to_grid_index(uci_sl)];
  slot_ucis.clear();

  for (const pucch_info& pucch : scheduled_pucchs) {
    if (pucch.csi_rep_cfg.has_value()) {
      uci_context& uci_ctx = slot_ucis.emplace_back();
      uci_ctx.rnti         = pucch.crnti;
      uci_ctx.csi_rep_cfg  = *pucch.csi_rep_cfg;
    }
  }
  for (const ul_sched_info& pusch : scheduled_puschs) {
    if (pusch.uci.has_value() and pusch.uci->csi.has_value()) {
      uci_context& uci_ctx = slot_ucis.emplace_back();
      uci_ctx.rnti         = pusch.pusch_cfg.rnti;
      uci_ctx.csi_rep_cfg  = pusch.uci->csi->csi_rep_cfg;
    }
  }
}
