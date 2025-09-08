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

#include "srsran/fapi_adaptor/mac/messages/pusch.h"
#include "srsran/fapi_adaptor/uci_part2_correspondence_mapper.h"
#include "srsran/phy/upper/channel_coding/ldpc/ldpc.h"
#include "srsran/ran/sch/sch_constants.h"
#include "srsran/scheduler/result/pusch_info.h"

using namespace srsran;
using namespace fapi_adaptor;

void srsran::fapi_adaptor::convert_pusch_mac_to_fapi(fapi::ul_pusch_pdu&              fapi_pdu,
                                                     const ul_sched_info&             mac_pdu,
                                                     uci_part2_correspondence_mapper& part2_mapper)
{
  fapi::ul_pusch_pdu_builder builder(fapi_pdu);

  convert_pusch_mac_to_fapi(builder, mac_pdu, part2_mapper);
}

/// Fill the optional UCI parameters.
static void fill_optional_uci_parameters(fapi::ul_pusch_pdu_builder&      builder,
                                         const std::optional<uci_info>&   uci,
                                         uci_part2_correspondence_mapper& part2_mapper)
{
  if (!uci) {
    return;
  }
  builder.add_optional_pusch_uci_alpha(uci->alpha);

  if (uci->harq) {
    const uci_info::harq_info& harq = *uci->harq;
    builder.add_optional_pusch_uci_harq(harq.harq_ack_nof_bits, harq.beta_offset_harq_ack);
  }

  if (uci->csi) {
    const uci_info::csi_info& csi = *uci->csi;
    builder.add_optional_pusch_uci_csi1(csi.csi_part1_nof_bits, csi.beta_offset_csi_1);

    // No CSI Part2 for a single antenna.
    if (csi.beta_offset_csi_2 && csi.csi_rep_cfg.pmi_codebook != pmi_codebook_type::one) {
      span<const uci_part2_correspondence_information> uci_correspondence = part2_mapper.map(csi.csi_rep_cfg);
      if (!uci_correspondence.empty()) {
        builder.add_optional_pusch_uci_csi2(*csi.beta_offset_csi_2);

        // Build UCI Part2 correspondence.
        for (const auto& part2 : uci_correspondence) {
          builder.add_uci_part1_part2_corresnpondence_v3(
              part2.priority,
              part2.part1_param_offsets,
              part2.part1_param_sizes,
              part2.part2_map_index,
              (part2.part2_map_scope == 1)
                  ? fapi::uci_part1_to_part2_correspondence_v3::map_scope_type::phy_context
                  : fapi::uci_part1_to_part2_correspondence_v3::map_scope_type::common_context);
        }
      }
    }
  }
}

void srsran::fapi_adaptor::convert_pusch_mac_to_fapi(fapi::ul_pusch_pdu_builder&      builder,
                                                     const ul_sched_info&             mac_pdu,
                                                     uci_part2_correspondence_mapper& part2_mapper)
{
  const pusch_information& pusch_pdu = mac_pdu.pusch_cfg;
  // :TODO: check this handle. It will be better to pass it from the translator, as the adaptor doesn't know how many
  // PDUs are expected.
  unsigned handle = 0;
  builder.set_basic_parameters(pusch_pdu.rnti, handle);

  const bwp_configuration& bwp_cfg = *pusch_pdu.bwp_cfg;
  builder.set_bwp_parameters(bwp_cfg.crbs.length(), bwp_cfg.crbs.start(), bwp_cfg.scs, bwp_cfg.cp);

  builder.set_information_parameters(pusch_pdu.mcs_descr.target_code_rate,
                                     pusch_pdu.mcs_descr.modulation,
                                     pusch_pdu.mcs_index.to_uint(),
                                     pusch_pdu.mcs_table,
                                     pusch_pdu.transform_precoding,
                                     pusch_pdu.n_id,
                                     pusch_pdu.nof_layers);

  const dmrs_information& dmrs_cfg = pusch_pdu.dmrs;
  builder.set_dmrs_parameters(dmrs_cfg.dmrs_symb_pos.to_uint64(),
                              dmrs_cfg.config_type,
                              dmrs_cfg.dmrs_scrambling_id,
                              dmrs_cfg.dmrs_scrambling_id_complement,
                              dmrs_cfg.low_papr_dmrs ? fapi::low_papr_dmrs_type::dependent_cdm_group
                                                     : fapi::low_papr_dmrs_type::independent_cdm_group,
                              pusch_pdu.pusch_dmrs_id,
                              dmrs_cfg.n_scid,
                              dmrs_cfg.num_dmrs_cdm_grps_no_data,
                              dmrs_cfg.dmrs_ports.to_uint64());

  const vrb_alloc& rbs = pusch_pdu.rbs;
  if (rbs.is_type0()) {
    static_vector<uint8_t, fapi::ul_pusch_pdu::RB_BITMAP_SIZE_IN_BYTES> rb_map;
    rb_map.resize(fapi::ul_pusch_pdu::RB_BITMAP_SIZE_IN_BYTES, 0U);
    const rbg_bitmap& mac_rbg_map = rbs.type0();
    for (unsigned i = 0, e = mac_rbg_map.size(); i != e; ++i) {
      rb_map[i / 8] |= uint8_t(mac_rbg_map.test(i) ? 1U : 0U) << i % 8;
    }

    builder.set_allocation_in_frequency_type_0_parameters(
        {rb_map}, pusch_pdu.tx_direct_current_location, pusch_pdu.ul_freq_shift_7p5khz);
  } else {
    const vrb_interval& vrb_int = rbs.type1();
    builder.set_allocation_in_frequency_type_1_parameters(vrb_int.start(),
                                                          vrb_int.length(),
                                                          pusch_pdu.intra_slot_freq_hopping,
                                                          pusch_pdu.tx_direct_current_location,
                                                          pusch_pdu.ul_freq_shift_7p5khz);
  }

  builder.set_allocation_in_time_parameters(pusch_pdu.symbols.start(), pusch_pdu.symbols.length());

  // Sending data through PUSCH is optional, but for now, the MAC does not signal this, use the TB size to catch it.
  srsran_assert(pusch_pdu.tb_size_bytes, "Transport block of null size");

  // Add PUSCH Data.
  builder.add_optional_pusch_data(pusch_pdu.rv_index,
                                  pusch_pdu.harq_id,
                                  pusch_pdu.new_data,
                                  units::bytes{pusch_pdu.tb_size_bytes},
                                  pusch_pdu.num_cb,
                                  {});

  // NOTE: MAC uses the value of the target code rate x[1024], as per TS38.214, Section 6.1.4.1, Table 6.1.4.1-1.
  float              R                  = pusch_pdu.mcs_descr.get_normalised_target_code_rate();
  const units::bytes tb_size_lbrm_bytes = tbs_lbrm_default;
  builder.set_maintenance_v3_frequency_allocation_parameters(
      pusch_pdu.pusch_second_hop_prb,
      get_ldpc_base_graph(R, units::bytes{pusch_pdu.tb_size_bytes}.to_bits()),
      tb_size_lbrm_bytes);

  builder.set_maintenance_v3_dmrs_parameters(static_cast<unsigned>(pusch_pdu.dmrs_hopping_mode));

  // Fill the UCI parameters.
  fill_optional_uci_parameters(builder, mac_pdu.uci, part2_mapper);

  // Set PUSCH context for logging.
  builder.set_context_vendor_specific(pusch_pdu.rnti, static_cast<harq_id_t>(pusch_pdu.harq_id));
}
