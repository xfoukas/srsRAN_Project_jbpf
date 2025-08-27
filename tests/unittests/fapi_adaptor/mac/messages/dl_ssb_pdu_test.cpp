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

#include "helpers.h"
#include "srsran/fapi_adaptor/mac/messages/ssb.h"
#include "srsran/mac/mac_cell_result.h"
#include "srsran/ran/ssb/pbch_mib_pack.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace fapi_adaptor;
using namespace unittests;

static uint32_t generate_bch_payload(const dl_ssb_pdu& mac_pdu, uint32_t sfn, bool hrf, subcarrier_spacing scs_common)
{
  // Prepare message parameters.
  pbch_mib_message msg = {.sfn                   = sfn,
                          .hrf                   = hrf,
                          .scs_common            = scs_common,
                          .subcarrier_offset     = mac_pdu.subcarrier_offset,
                          .dmrs_typeA_pos        = mac_pdu.mib_data.dmrs_typeA_pos,
                          .pdcch_config_sib1     = mac_pdu.mib_data.pdcch_config_sib1,
                          .cell_barred           = mac_pdu.mib_data.cell_barred,
                          .intrafreq_reselection = mac_pdu.mib_data.intra_freq_reselection,
                          .ssb_block_index       = mac_pdu.ssb_index};

  // Generate payload.
  return pbch_mib_pack(msg);
}

TEST(mac_fapi_ssb_pdu_conversor_test, valid_pdu_should_pass)
{
  srsran::dl_ssb_pdu pdu = build_valid_dl_ssb_pdu();

  fapi::dl_ssb_pdu fapi_pdu;
  slot_point       slot(1, 0);
  convert_ssb_mac_to_fapi(fapi_pdu, pdu, slot_point(1, 0));

  ASSERT_EQ(pdu.pci, fapi_pdu.phys_cell_id);
  ASSERT_EQ(static_cast<unsigned>(pdu.pss_to_sss_epre), static_cast<unsigned>(fapi_pdu.beta_pss_profile_nr));
  ASSERT_EQ(pdu.ssb_index, fapi_pdu.ssb_block_index);
  ASSERT_EQ(pdu.subcarrier_offset.to_uint(), fapi_pdu.ssb_subcarrier_offset);
  ASSERT_EQ(pdu.offset_to_pointA.to_uint(), fapi_pdu.ssb_offset_pointA.to_uint());

  // Maintenance v3 parameters.
  ASSERT_EQ(static_cast<unsigned>(pdu.ssb_case), static_cast<unsigned>(fapi_pdu.ssb_maintenance_v3.case_type));
  ASSERT_EQ(pdu.L_max, fapi_pdu.ssb_maintenance_v3.L_max);
  ASSERT_EQ(static_cast<unsigned>(pdu.scs), static_cast<unsigned>(fapi_pdu.ssb_maintenance_v3.scs));

  // MIB.
  ASSERT_EQ(fapi::bch_payload_type::phy_timing_info, fapi_pdu.bch_payload_flag);
  ASSERT_EQ(generate_bch_payload(pdu, slot.sfn(), slot.is_odd_hrf(), slot.scs()) >> 8,
            fapi_pdu.bch_payload.bch_payload);
}
