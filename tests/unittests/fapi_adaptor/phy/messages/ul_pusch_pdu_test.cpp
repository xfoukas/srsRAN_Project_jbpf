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

#include "../../../fapi/message_builder_helpers.h"
#include "srsran/fapi_adaptor/phy/messages/pusch.h"
#include "srsran/fapi_adaptor/uci_part2_correspondence_generator.h"
#include <gtest/gtest.h>
#include <random>

using namespace srsran;
using namespace fapi_adaptor;
using namespace unittest;

// Random generator.
static std::mt19937 rgen;

TEST(fapi_phy_ul_pusch_adaptor_test, valid_pdu_pass)
{
  fapi::ul_pusch_pdu fapi_pdu = build_valid_ul_pusch_pdu();

  unsigned sfn  = 1U;
  unsigned slot = 2U;

  std::uniform_int_distribution<unsigned> nof_antenna_ports_dist(1, 4);
  unsigned                                nof_antenna_ports = nof_antenna_ports_dist(rgen);
  auto                                    uci_part2_tools   = fapi_adaptor::generate_uci_part2_correspondence(1);

  uplink_pdu_slot_repository::pusch_pdu pdu;
  convert_pusch_fapi_to_phy(pdu,
                            fapi_pdu,
                            sfn,
                            slot,
                            nof_antenna_ports,
                            *std::get<std::unique_ptr<uci_part2_correspondence_repository>>(uci_part2_tools));

  const pusch_processor::pdu_t& phy_pdu = pdu.pdu;
  ASSERT_EQ(slot_point(to_numerology_value(fapi_pdu.scs), sfn, slot), phy_pdu.slot);
  ASSERT_EQ(fapi_pdu.start_symbol_index, phy_pdu.start_symbol_index);
  ASSERT_EQ(fapi_pdu.nr_of_symbols, phy_pdu.nof_symbols);
  ASSERT_EQ(to_value(fapi_pdu.rnti), phy_pdu.rnti);
  ASSERT_EQ(fapi_pdu.bwp_start, phy_pdu.bwp_start_rb);
  ASSERT_EQ(fapi_pdu.bwp_size, phy_pdu.bwp_size_rb);
  ASSERT_EQ(fapi_pdu.cp, phy_pdu.cp);
  ASSERT_EQ(fapi_pdu.qam_mod_order, phy_pdu.mcs_descr.modulation);
  ASSERT_EQ(fapi_pdu.nid_pusch, phy_pdu.n_id);
  ASSERT_EQ(fapi_pdu.num_layers, phy_pdu.nof_tx_layers);
  ASSERT_EQ(nof_antenna_ports, phy_pdu.rx_ports.size());

  if (fapi_pdu.tx_direct_current_location < 3300) {
    ASSERT_TRUE(phy_pdu.dc_position.has_value());
    ASSERT_EQ(fapi_pdu.tx_direct_current_location, phy_pdu.dc_position.value());
  } else {
    ASSERT_FALSE(phy_pdu.dc_position.has_value());
  }

  // DM-RS.
  for (unsigned i = 0; i != 14; ++i) {
    ASSERT_EQ(((fapi_pdu.ul_dmrs_symb_pos >> i) & 1U) == 1U, phy_pdu.dmrs_symbol_mask.test(i));
  }
  if (fapi_pdu.transform_precoding) {
    const auto& dmrs_config = std::get<pusch_processor::dmrs_transform_precoding_configuration>(phy_pdu.dmrs);
    ASSERT_EQ(fapi_pdu.pusch_dmrs_identity, dmrs_config.n_rs_id);
  } else {
    const auto& dmrs_config = std::get<pusch_processor::dmrs_configuration>(phy_pdu.dmrs);
    ASSERT_EQ(dmrs_type((fapi_pdu.dmrs_type == fapi::dmrs_cfg_type::type_1) ? dmrs_type::options::TYPE1
                                                                            : dmrs_type::options::TYPE2),
              dmrs_config.dmrs);
    ASSERT_EQ(fapi_pdu.pusch_dmrs_scrambling_id, dmrs_config.scrambling_id);
    ASSERT_EQ(fapi_pdu.nscid, dmrs_config.n_scid);
    ASSERT_EQ(fapi_pdu.num_dmrs_cdm_grps_no_data, dmrs_config.nof_cdm_groups_without_data);
  }
  ASSERT_EQ(fapi_pdu.pusch_maintenance_v3.tb_size_lbrm_bytes, phy_pdu.tbs_lbrm);

  // RB allocation.
  vrb_bitmap vrb_bitmap(fapi_pdu.bwp_size);
  for (unsigned vrb_index = 0, vrb_index_end = fapi_pdu.bwp_size; vrb_index != vrb_index_end; ++vrb_index) {
    unsigned byte = vrb_index / 8;
    unsigned bit  = vrb_index % 8;
    if ((fapi_pdu.rb_bitmap[byte] >> bit) & 1U) {
      vrb_bitmap.set(vrb_index);
    }
  }

  rb_allocation alloc = (fapi_pdu.resource_alloc == fapi::resource_allocation_type::type_1)
                            ? rb_allocation::make_type1(fapi_pdu.rb_start, fapi_pdu.rb_size, {})
                            : rb_allocation::make_type0(vrb_bitmap, {});

  ASSERT_EQ(alloc, phy_pdu.freq_alloc);

  // Codeword.
  ASSERT_TRUE(phy_pdu.codeword.has_value());
  ASSERT_EQ(fapi_pdu.pusch_data.rv_index, phy_pdu.codeword.value().rv);
  ASSERT_EQ(fapi_pdu.pusch_data.new_data, phy_pdu.codeword.value().new_data);
  ASSERT_EQ(fapi_pdu.pusch_maintenance_v3.ldpc_base_graph, phy_pdu.codeword.value().ldpc_base_graph);
  ASSERT_EQ(fapi_pdu.pusch_data.tb_size.value(), pdu.tb_size.value());
  ASSERT_EQ(fapi_pdu.pusch_data.harq_process_id, pdu.harq_id);
}
