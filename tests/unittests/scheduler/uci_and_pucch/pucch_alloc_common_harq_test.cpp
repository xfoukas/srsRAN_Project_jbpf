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
#include <gtest/gtest.h>

using namespace srsran;

////////////    Structs with expected parameters and PUCCH sched INPUT     ////////////

const unsigned NOF_RBS = 52;

namespace pucch_harq_common_test {
// Expected results parameters.
struct expected_output_params_f1 {
  pucch_format      format;
  prb_interval      prbs;
  prb_interval      second_hop_prbs;
  ofdm_symbol_range symbols;
  bool              intra_slot_freq_hop;
  uint8_t           initial_cyclic_shift;
  sr_nof_bits       sr_bits;
  unsigned          harq_ack_nof_bits;
  uint8_t           time_domain_occ;
};

// Parameters for PUCCH scheduler initialization.
struct pucch_params {
  // Input parameters.
  unsigned pucch_res_common;
  unsigned n_cces;
};

// Parameters to be passed to test for PUCCH output assessment.
struct pucch_test_parameters {
  unsigned                  dci_pucch_res_indicator;
  expected_output_params_f1 output_params;
  pucch_params              pucch_input_params;
};

// Dummy function overload of template <typename T> void testing::internal::PrintTo(const T& value, ::std::ostream* os).
// This prevents valgrind from complaining about uninitialized variables.
void PrintTo(const pucch_test_parameters& value, ::std::ostream* os)
{
  return;
}

static expected_output_params_f1 make_expected_output_params_sr_only()
{
  return expected_output_params_f1{.format               = pucch_format::FORMAT_1,
                                   .prbs                 = prb_interval{NOF_RBS - 3, NOF_RBS - 2},
                                   .second_hop_prbs      = prb_interval{0, 0},
                                   .symbols              = ofdm_symbol_range{0, 14},
                                   .intra_slot_freq_hop  = false,
                                   .initial_cyclic_shift = 0,
                                   .sr_bits              = sr_nof_bits::one,
                                   .harq_ack_nof_bits    = 0,
                                   .time_domain_occ      = 0};
}

// Make expected results parameters (default values are for SR only).
expected_output_params_f1 make_expected_output_params_sr_harq()
{
  expected_output_params_f1 out = make_expected_output_params_sr_only();
  out.harq_ack_nof_bits         = 1;
  return out;
}
} // namespace pucch_harq_common_test

///////   Test allocation of PUCCH common resources    ///////

using namespace pucch_harq_common_test;

class test_pucch_harq_common_output : public ::testing::TestWithParam<pucch_test_parameters>
{
public:
  test_pucch_harq_common_output() :
    params{GetParam()},
    t_bench(test_bench_params{.pucch_res_common = params.pucch_input_params.pucch_res_common,
                              .n_cces           = params.pucch_input_params.n_cces}),
    pucch_expected(build_pucch_info(&t_bench.cell_cfg.ul_cfg_common.init_ul_bwp.generic_params,
                                    static_cast<unsigned>(t_bench.cell_cfg.pci),
                                    params.output_params.format,
                                    params.output_params.prbs,
                                    params.output_params.second_hop_prbs,
                                    params.output_params.symbols,
                                    params.output_params.initial_cyclic_shift,
                                    params.output_params.sr_bits,
                                    params.output_params.harq_ack_nof_bits,
                                    params.output_params.time_domain_occ))
  {
  }

protected:
  // Parametrized variables.
  pucch_test_parameters params;
  test_bench            t_bench;
  pucch_info            pucch_expected;
};

// Tests the output of the PUCCH allocator (or PUCCH PDU).
TEST_P(test_pucch_harq_common_output, test_pucch_output_info)
{
  std::optional<unsigned> pucch_res_indicator = t_bench.pucch_alloc.alloc_common_pucch_harq_ack_ue(
      t_bench.res_grid, t_bench.get_main_ue().crnti, t_bench.k0, t_bench.k1, t_bench.dci_info);

  ASSERT_TRUE(pucch_res_indicator.has_value());
  ASSERT_FALSE(t_bench.res_grid[t_bench.k0 + t_bench.k1].result.ul.pucchs.empty());
  ASSERT_TRUE(
      find_pucch_pdu(t_bench.res_grid[t_bench.k0 + t_bench.k1].result.ul.pucchs,
                     [&expected = pucch_expected](const auto& pdu) { return pucch_info_match(expected, pdu); }));
}

// Tests whether PUCCH allocator returns the correct values for the DCI.
TEST_P(test_pucch_harq_common_output, test_pucch_output_for_dci)
{
  std::optional<unsigned> pucch_res_indicator = t_bench.pucch_alloc.alloc_common_pucch_harq_ack_ue(
      t_bench.res_grid, t_bench.get_main_ue().crnti, t_bench.k0, t_bench.k1, t_bench.dci_info);

  ASSERT_TRUE(pucch_res_indicator.has_value());
  ASSERT_EQ(GetParam().dci_pucch_res_indicator, pucch_res_indicator.value());
}

INSTANTIATE_TEST_SUITE_P(
    test_pucch_output_for_dci,
    test_pucch_harq_common_output,
    testing::Values(
        pucch_test_parameters{
            .dci_pucch_res_indicator = 0,
            .output_params           = pucch_harq_common_test::expected_output_params_f1{pucch_format::FORMAT_0,
                                                                               prb_interval{0, 1},
                                                                               prb_interval{NOF_RBS - 1, NOF_RBS},
                                                                               ofdm_symbol_range{12, 14},
                                                                               true,
                                                                               0,
                                                                               sr_nof_bits::no_sr,
                                                                               1,
                                                                               0},
            .pucch_input_params      = pucch_params{0, 1}},
        pucch_test_parameters{.dci_pucch_res_indicator = 0,
                              .output_params           = expected_output_params_f1{pucch_format::FORMAT_0,
                                                                         prb_interval{3, 4},
                                                                         prb_interval{NOF_RBS - 4, NOF_RBS - 3},
                                                                         ofdm_symbol_range{12, 14},
                                                                         true,
                                                                         0,
                                                                         sr_nof_bits::no_sr,
                                                                         1,
                                                                         0},
                              .pucch_input_params      = pucch_params{2, 1}},
        pucch_test_parameters{.dci_pucch_res_indicator = 0,
                              .output_params           = expected_output_params_f1{pucch_format::FORMAT_0,
                                                                         prb_interval{3, 4},
                                                                         prb_interval{NOF_RBS - 4, NOF_RBS - 3},
                                                                         ofdm_symbol_range{12, 14},
                                                                         true,
                                                                         8,
                                                                         sr_nof_bits::no_sr,
                                                                         1,
                                                                         0},
                              .pucch_input_params      = pucch_params{2, 8}},
        pucch_test_parameters{.dci_pucch_res_indicator = 0,
                              .output_params           = expected_output_params_f1{pucch_format::FORMAT_1,
                                                                         prb_interval{4, 5},
                                                                         prb_interval{NOF_RBS - 5, NOF_RBS - 4},
                                                                         ofdm_symbol_range{4, 14},
                                                                         true,
                                                                         0,
                                                                         sr_nof_bits::no_sr,
                                                                         1,
                                                                         0},
                              .pucch_input_params      = pucch_params{10, 0}},
        pucch_test_parameters{.dci_pucch_res_indicator = 0,
                              .output_params           = expected_output_params_f1{pucch_format::FORMAT_1,
                                                                         prb_interval{4, 5},
                                                                         prb_interval{NOF_RBS - 5, NOF_RBS - 4},
                                                                         ofdm_symbol_range{4, 14},
                                                                         true,
                                                                         6,
                                                                         sr_nof_bits::no_sr,
                                                                         1,
                                                                         0},
                              .pucch_input_params      = pucch_params{10, 8}},
        pucch_test_parameters{.dci_pucch_res_indicator = 0,
                              .output_params           = expected_output_params_f1{pucch_format::FORMAT_1,
                                                                         prb_interval{0, 1},
                                                                         prb_interval{NOF_RBS - 1, NOF_RBS},
                                                                         ofdm_symbol_range{0, 14},
                                                                         true,
                                                                         0,
                                                                         sr_nof_bits::no_sr,
                                                                         1,
                                                                         0},
                              .pucch_input_params      = pucch_params{11, 0}},
        pucch_test_parameters{.dci_pucch_res_indicator = 0,
                              .output_params           = expected_output_params_f1{pucch_format::FORMAT_1,
                                                                         prb_interval{1, 2},
                                                                         prb_interval{NOF_RBS - 2, NOF_RBS - 1},
                                                                         ofdm_symbol_range{0, 14},
                                                                         true,
                                                                         0,
                                                                         sr_nof_bits::no_sr,
                                                                         1,
                                                                         0},
                              .pucch_input_params      = pucch_params{11, 8}},
        pucch_test_parameters{.dci_pucch_res_indicator = 0,
                              .output_params           = expected_output_params_f1{pucch_format::FORMAT_1,
                                                                         prb_interval{13, 14},
                                                                         prb_interval{NOF_RBS - 14, NOF_RBS - 13},
                                                                         ofdm_symbol_range{0, 14},
                                                                         true,
                                                                         0,
                                                                         sr_nof_bits::no_sr,
                                                                         1,
                                                                         0},
                              .pucch_input_params      = pucch_params{15, 0}},
        pucch_test_parameters{.dci_pucch_res_indicator = 0,
                              .output_params           = expected_output_params_f1{pucch_format::FORMAT_1,
                                                                         prb_interval{13, 14},
                                                                         prb_interval{NOF_RBS - 14, NOF_RBS - 13},
                                                                         ofdm_symbol_range{0, 14},
                                                                         true,
                                                                         6,
                                                                         sr_nof_bits::no_sr,
                                                                         1,
                                                                         0},
                              .pucch_input_params      = pucch_params{15, 8}}),
    [](const ::testing::TestParamInfo<test_pucch_harq_common_output::ParamType>& info_) {
      return fmt::format("pucch_res_common_{}_n_cces_{}",
                         info_.param.pucch_input_params.pucch_res_common,
                         info_.param.pucch_input_params.n_cces);
    });

///////   Test multiple allocation of common PUCCH resources    ///////

class test_pucch_harq_common_multiple_allocation : public ::testing::Test
{
public:
  test_pucch_harq_common_multiple_allocation() : t_bench(test_bench_params{.pucch_res_common = 0, .n_cces = 1}) {}

protected:
  // Parameters that are passed by the routing to run the tests.
  test_bench t_bench;
};

TEST_F(test_pucch_harq_common_multiple_allocation, test_pucch_double_alloc)
{
  const std::optional<unsigned> pucch_res_indicator = t_bench.pucch_alloc.alloc_common_pucch_harq_ack_ue(
      t_bench.res_grid, t_bench.get_main_ue().crnti, t_bench.k0, t_bench.k1, t_bench.dci_info);
  ASSERT_TRUE(pucch_res_indicator.has_value());

  // If we allocate the same UE twice, the scheduler is expected to fail, as we don't support PUCCH multiplexing on
  // PUCCH common resources.
  std::optional<unsigned> pucch_res_indicator_1 = t_bench.pucch_alloc.alloc_common_pucch_harq_ack_ue(
      t_bench.res_grid, t_bench.get_main_ue().crnti, t_bench.k0, t_bench.k1, t_bench.dci_info);
  ASSERT_FALSE(pucch_res_indicator_1.has_value());
}

TEST_F(test_pucch_harq_common_multiple_allocation, test_pucch_out_of_resources)
{
  // For this specific n_cce value (1) and for d_pri = {0,...,7}, we get 8 r_pucch values. This is the maximum number of
  // UEs we can allocate.
  for (uint16_t n_ue = 0; n_ue != 8; ++n_ue) {
    t_bench.add_ue();
    du_ue_index_t                 ue_idx = to_du_ue_index(static_cast<uint16_t>(t_bench.get_main_ue().ue_index) + n_ue);
    const std::optional<unsigned> pucch_res_indicator = t_bench.pucch_alloc.alloc_common_pucch_harq_ack_ue(
        t_bench.res_grid, t_bench.get_ue(ue_idx).crnti, t_bench.k0, t_bench.k1, t_bench.dci_info);
    ASSERT_TRUE(pucch_res_indicator.has_value());
  }

  // If we allocate an extra UE, the scheduler is expected to fail.
  const std::optional<unsigned> pucch_res_indicator_1 = t_bench.pucch_alloc.alloc_common_pucch_harq_ack_ue(
      t_bench.res_grid, t_bench.get_main_ue().crnti, t_bench.k0, t_bench.k1, t_bench.dci_info);
  ASSERT_FALSE(pucch_res_indicator_1.has_value());
}

TEST_F(test_pucch_harq_common_multiple_allocation, test_on_full_grid)
{
  t_bench.pucch_alloc.alloc_common_pucch_harq_ack_ue(
      t_bench.res_grid, t_bench.get_main_ue().crnti, t_bench.k0, t_bench.k1, t_bench.dci_info);

  ASSERT_FALSE(t_bench.res_grid[t_bench.k0 + t_bench.k1].result.ul.pucchs.empty());
  const pucch_info& pucch_pdu_benchmark = t_bench.res_grid[t_bench.k0 + t_bench.k1].result.ul.pucchs.back();

  // Increase the slot; this is to verify the PUCCH allocation over a full grid yields the same PUCCH PDU as over an
  // empty one.
  t_bench.slot_indication(++t_bench.sl_tx);

  // Fill the entire grid and verify the PUCCH gets allocated anyway.
  t_bench.fill_all_grid(t_bench.sl_tx + t_bench.k1);

  const std::optional<unsigned> pucch_res_indicator_1 = t_bench.pucch_alloc.alloc_common_pucch_harq_ack_ue(
      t_bench.res_grid, t_bench.get_main_ue().crnti, t_bench.k0, t_bench.k1, t_bench.dci_info);
  ASSERT_TRUE(pucch_res_indicator_1.has_value());

  ASSERT_FALSE(t_bench.res_grid[t_bench.k0 + t_bench.k1].result.ul.pucchs.empty());

  ASSERT_EQ(0, pucch_res_indicator_1.value());
  ASSERT_TRUE(
      find_pucch_pdu(t_bench.res_grid[t_bench.k0 + t_bench.k1].result.ul.pucchs,
                     [&expected = pucch_pdu_benchmark](const auto& pdu) { return pucch_info_match(expected, pdu); }));
}
