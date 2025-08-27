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

#include "tests/unittests/scheduler/test_utils/scheduler_test_suite.h"
#include "uci_test_utils.h"
#include "srsran/support/test_utils.h"
#include <gtest/gtest.h>

using namespace srsran;

/////////////  Test SR scheduling   /////////////

class uci_sr_scheduler_tester : public ::testing::TestWithParam<sr_periodicity>
{
public:
  uci_sr_scheduler_tester() :
    sr_period(GetParam()),
    sr_offset(test_rgen::uniform_int<unsigned>(0, sr_periodicity_to_slot(GetParam()) - 1)),
    t_bench{test_bench_params{.pucch_res_common = pucch_res_common,
                              .n_cces           = n_cces,
                              .period           = sr_period,
                              .offset           = sr_offset}}
  {
    csi_offset = std::get<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
                     t_bench.get_main_ue().get_pcell().cfg().csi_meas_cfg()->csi_report_cfg_list[0].report_cfg_type)
                     .report_slot_offset;

    csi_period = std::get<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
                     t_bench.get_main_ue().get_pcell().cfg().csi_meas_cfg()->csi_report_cfg_list[0].report_cfg_type)
                     .report_slot_period;

    // In the slots with SR only, the expected format is Format 1.
    auto& format1_sr           = pucch_sr_only_test.format_params.emplace<pucch_format_1>();
    pucch_sr_only_test.crnti   = to_rnti(0x4601);
    pucch_sr_only_test.bwp_cfg = &t_bench.cell_cfg.ul_cfg_common.init_ul_bwp.generic_params;

    pucch_sr_only_test.resources.prbs             = prb_interval{49, 50};
    pucch_sr_only_test.resources.second_hop_prbs  = prb_interval{0, 0};
    pucch_sr_only_test.resources.symbols          = ofdm_symbol_range{0, 14};
    pucch_sr_only_test.uci_bits.sr_bits           = sr_nof_bits::one;
    pucch_sr_only_test.uci_bits.harq_ack_nof_bits = 0;

    format1_sr.initial_cyclic_shift = 0;
    format1_sr.time_domain_occ      = 0;

    format1_sr.group_hopping   = pucch_group_hopping::NEITHER;
    format1_sr.n_id_hopping    = t_bench.cell_cfg.pci;
    format1_sr.slot_repetition = pucch_repetition_tx_slot::no_multi_slot;

    // In the slots with SR + CSI, the expected format is Format 2.
    auto& format2_sr_csi                          = pucch_sr_csi_test.format_params.emplace<pucch_format_2>();
    pucch_sr_csi_test.crnti                       = to_rnti(0x4601);
    pucch_sr_csi_test.bwp_cfg                     = &t_bench.cell_cfg.ul_cfg_common.init_ul_bwp.generic_params;
    pucch_sr_csi_test.resources.prbs              = prb_interval{2, 3};
    pucch_sr_csi_test.resources.second_hop_prbs   = prb_interval{0, 0};
    pucch_sr_csi_test.resources.symbols           = ofdm_symbol_range{12, 14};
    pucch_sr_csi_test.uci_bits.harq_ack_nof_bits  = 0;
    pucch_sr_csi_test.uci_bits.sr_bits            = sr_nof_bits::one;
    pucch_sr_csi_test.uci_bits.csi_part1_nof_bits = 4;

    format2_sr_csi.max_code_rate     = max_pucch_code_rate::dot_25;
    format2_sr_csi.n_id_scambling    = t_bench.cell_cfg.pci;
    format2_sr_csi.n_id_0_scrambling = t_bench.cell_cfg.pci;
  }

protected:
  // Parametrized variables.
  sr_periodicity sr_period;
  unsigned       sr_offset;
  // Helper variables.
  csi_report_periodicity csi_period;
  unsigned               csi_offset;
  unsigned               pucch_res_common{11};
  unsigned               n_cces{0};
  test_bench             t_bench;
  // Expected PUCCH grants to be verified in the test.
  pucch_info pucch_sr_only_test;
  pucch_info pucch_sr_csi_test;
};

TEST_P(uci_sr_scheduler_tester, test_different_periods)
{
  // Check at the allocation for at least 2 the size of the resource grid.
  const unsigned NOF_SLOTS_TO_TEST =
      std::max(sr_periodicity_to_slot(sr_period) * 4, static_cast<unsigned>(t_bench.res_grid.RING_ALLOCATOR_SIZE) * 2);

  // Randomize initial slot, as the UCI scheduler will be called only after the UE is added.
  const unsigned starting_slot = test_rgen::uniform_int<unsigned>(0, 1000U);
  for (unsigned sl_cnt = starting_slot; sl_cnt < starting_slot + NOF_SLOTS_TO_TEST; ++sl_cnt) {
    t_bench.uci_sched.run_slot(t_bench.res_grid);
    if ((t_bench.sl_tx - sr_offset).to_uint() % sr_periodicity_to_slot(sr_period) == 0) {
      ASSERT_EQ(1, t_bench.res_grid[0].result.ul.pucchs.size());
      // The scheduler allocates:
      // - SR only on slots that are for SR only.
      // - CSI + SR on slots that are for CSI + SR.
      if ((t_bench.sl_tx - csi_offset).to_uint() % csi_report_periodicity_to_uint(csi_period) == 0) {
        ASSERT_TRUE(
            find_pucch_pdu(t_bench.res_grid[0].result.ul.pucchs, [&expected = pucch_sr_csi_test](const auto& pdu) {
              return pucch_info_match(expected, pdu);
            }));
      } else {
        ASSERT_TRUE(
            find_pucch_pdu(t_bench.res_grid[0].result.ul.pucchs, [&expected = pucch_sr_only_test](const auto& pdu) {
              return pucch_info_match(expected, pdu);
            }));
      }
    }
    // Update the slot indicator.
    t_bench.slot_indication(++t_bench.sl_tx);
  }
}

INSTANTIATE_TEST_SUITE_P(test_sr_sched_different_periods_offsets,
                         uci_sr_scheduler_tester,
                         testing::Values(sr_periodicity::sl_1,
                                         sr_periodicity::sl_2,
                                         sr_periodicity::sl_4,
                                         sr_periodicity::sl_5,
                                         sr_periodicity::sl_8,
                                         sr_periodicity::sl_16,
                                         sr_periodicity::sl_20,
                                         sr_periodicity::sl_40,
                                         sr_periodicity::sl_80,
                                         sr_periodicity::sl_160,
                                         sr_periodicity::sl_320,
                                         sr_periodicity::sl_640));

/////////////  Test CSI scheduling   /////////////

class uci_csi_scheduler_tester : public ::testing::TestWithParam<csi_report_periodicity>
{
public:
  uci_csi_scheduler_tester() :
    csi_period(GetParam()),
    csi_offset(test_rgen::uniform_int<unsigned>(0, csi_report_periodicity_to_uint(GetParam()) - 1)),
    t_bench{test_bench_params{.csi_period = csi_period, .csi_offset = csi_offset}}
  {
    sr_period = sr_periodicity_to_slot(
        t_bench.get_main_ue().get_pcell().cfg().init_bwp().ul_ded->pucch_cfg.value().sr_res_list[0].period);

    sr_offset = t_bench.get_main_ue().get_pcell().cfg().init_bwp().ul_ded->pucch_cfg.value().sr_res_list[0].offset;

    // In the slots with SR + CSI, the expected format is Format 2.
    auto& format2_csi_and_sr                          = pucch_csi_and_sr_test.format_params.emplace<pucch_format_2>();
    pucch_csi_and_sr_test.crnti                       = to_rnti(0x4601);
    pucch_csi_and_sr_test.bwp_cfg                     = &t_bench.cell_cfg.ul_cfg_common.init_ul_bwp.generic_params;
    pucch_csi_and_sr_test.resources.prbs              = prb_interval{2, 3};
    pucch_csi_and_sr_test.resources.second_hop_prbs   = prb_interval{0, 0};
    pucch_csi_and_sr_test.resources.symbols           = ofdm_symbol_range{12, 14};
    pucch_csi_and_sr_test.uci_bits.harq_ack_nof_bits  = 0;
    pucch_csi_and_sr_test.uci_bits.sr_bits            = sr_nof_bits::one;
    pucch_csi_and_sr_test.uci_bits.csi_part1_nof_bits = 4;

    format2_csi_and_sr.max_code_rate     = max_pucch_code_rate::dot_25;
    format2_csi_and_sr.n_id_scambling    = t_bench.cell_cfg.pci;
    format2_csi_and_sr.n_id_0_scrambling = t_bench.cell_cfg.pci;

    // In the slots with CSI only, the expected format is Format 2.
    auto& format2_csi                               = pucch_csi_only_test.format_params.emplace<pucch_format_2>();
    pucch_csi_only_test.crnti                       = to_rnti(0x4601);
    pucch_csi_only_test.bwp_cfg                     = &t_bench.cell_cfg.ul_cfg_common.init_ul_bwp.generic_params;
    pucch_csi_only_test.resources.prbs              = prb_interval{2, 3};
    pucch_csi_only_test.resources.second_hop_prbs   = prb_interval{0, 0};
    pucch_csi_only_test.resources.symbols           = ofdm_symbol_range{12, 14};
    pucch_csi_only_test.uci_bits.harq_ack_nof_bits  = 0;
    pucch_csi_only_test.uci_bits.sr_bits            = sr_nof_bits::no_sr;
    pucch_csi_only_test.uci_bits.csi_part1_nof_bits = 4;

    format2_csi.max_code_rate     = max_pucch_code_rate::dot_25;
    format2_csi.n_id_scambling    = t_bench.cell_cfg.pci;
    format2_csi.n_id_0_scrambling = t_bench.cell_cfg.pci;
  }

protected:
  // Parametrized variables.
  csi_report_periodicity csi_period;
  unsigned               csi_offset;
  // Helper variables.
  unsigned   sr_period;
  unsigned   sr_offset{0};
  test_bench t_bench;
  // Expected PUCCH grants to be verified in the test.
  pucch_info pucch_csi_and_sr_test;
  pucch_info pucch_csi_only_test;
};

TEST_P(uci_csi_scheduler_tester, test_different_periods)
{
  // Check at the allocation for at least 2 the size of the resource grid.
  const unsigned NOF_SLOTS_TO_TEST = std::max(csi_report_periodicity_to_uint(csi_period) * 8,
                                              static_cast<unsigned>(t_bench.res_grid.max_ul_slot_alloc_delay) * 2);

  // Randomize initial slot, as the UCI scheduler will be called only after the UE is added.
  const unsigned starting_slot = test_rgen::uniform_int<unsigned>(0, 1000U);
  for (unsigned sl_cnt = starting_slot; sl_cnt < starting_slot + NOF_SLOTS_TO_TEST; ++sl_cnt) {
    t_bench.uci_sched.run_slot(t_bench.res_grid);
    if ((t_bench.sl_tx - csi_offset).to_uint() % csi_report_periodicity_to_uint(csi_period) == 0) {
      ASSERT_EQ(1, t_bench.res_grid[0].result.ul.pucchs.size());
      // The scheduler allocates:
      // - CSI only on slots that are for CSI only.
      // - CSI + SR on slots that are for CSI + SR.
      if ((t_bench.sl_tx - sr_offset).to_uint() % sr_period == 0) {
        ASSERT_TRUE(
            find_pucch_pdu(t_bench.res_grid[0].result.ul.pucchs, [&expected = pucch_csi_and_sr_test](const auto& pdu) {
              return pucch_info_match(expected, pdu);
            }));
      } else {
        ASSERT_TRUE(
            find_pucch_pdu(t_bench.res_grid[0].result.ul.pucchs, [&expected = pucch_csi_only_test](const auto& pdu) {
              return pucch_info_match(expected, pdu);
            }));
      }
    }
    // Update the slot indicator.
    t_bench.slot_indication(++t_bench.sl_tx);
  }
}

INSTANTIATE_TEST_SUITE_P(test_csi_sched_different_periods_offsets,
                         uci_csi_scheduler_tester,
                         testing::Values(csi_report_periodicity::slots4,
                                         csi_report_periodicity::slots5,
                                         csi_report_periodicity::slots8,
                                         csi_report_periodicity::slots10,
                                         csi_report_periodicity::slots16,
                                         csi_report_periodicity::slots20,
                                         csi_report_periodicity::slots40,
                                         csi_report_periodicity::slots80,
                                         csi_report_periodicity::slots160,
                                         csi_report_periodicity::slots320));

/////////////  Test UCI with UE reconfiguration   /////////////

class uci_scheduler_reconf_tester : public ::testing::TestWithParam<bool>
{
public:
  uci_scheduler_reconf_tester() : t_bench{test_bench_params{.no_csi = GetParam()}} {}

protected:
  // Helper variables.
  unsigned   sr_period_slots{sr_periodicity_to_slot(test_bench_params().period)};
  unsigned   sr_offset{test_bench_params().offset};
  test_bench t_bench;
};

TEST_P(uci_scheduler_reconf_tester, after_ue_reconf_uci_doesnt_stopped_being_scheduled)
{
  const unsigned NOF_SLOTS_TO_TEST = sr_period_slots * 4;

  for (unsigned sl_cnt = 0; sl_cnt < NOF_SLOTS_TO_TEST; ++sl_cnt) {
    t_bench.uci_sched.run_slot(t_bench.res_grid);
    if ((t_bench.sl_tx - sr_offset).to_uint() % sr_period_slots == 0) {
      ASSERT_EQ(1, t_bench.res_grid[0].result.ul.pucchs.size());
    }
    // Update the slot indicator.
    t_bench.slot_indication(++t_bench.sl_tx);
  }

  // This is the reconfiguration of the UE; we are interested in checking that the UCI scheduler does not crash
  // after the reconfiguration, more than the testing that the configuration has been changed.
  t_bench.uci_sched.reconf_ue(t_bench.get_main_ue().get_pcell().cfg(), t_bench.get_main_ue().get_pcell().cfg());

  // After reconfiguration, the UCI scheduler should still be able to schedule the UCI.
  for (unsigned sl_cnt = 0; sl_cnt < NOF_SLOTS_TO_TEST; ++sl_cnt) {
    t_bench.uci_sched.run_slot(t_bench.res_grid);
    if ((t_bench.sl_tx - sr_offset).to_uint() % sr_period_slots == 0) {
      ASSERT_EQ(1, t_bench.res_grid[0].result.ul.pucchs.size());
    }
    // Update the slot indicator.
    t_bench.slot_indication(++t_bench.sl_tx);
  }
}

INSTANTIATE_TEST_SUITE_P(test_with_and_without_csi, uci_scheduler_reconf_tester, testing::Values(false, true));
