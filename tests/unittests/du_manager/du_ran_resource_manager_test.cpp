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

#include "lib/du/du_high/du_manager/ran_resource_management/du_ran_resource_manager_impl.h"
#include "srsran/du/du_cell_config_helpers.h"
#include "srsran/du/du_high/du_qos_config_helpers.h"
#include "srsran/support/test_utils.h"
#include "fmt/ostream.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace srs_du;

class du_ran_resource_manager_tester_base
{
protected:
  explicit du_ran_resource_manager_tester_base(cell_config_builder_params params_,
                                               du_cell_config             du_cfg_param_,
                                               unsigned                   max_pucch_grants = 31) :
    params(params_),
    du_cfg_param(du_cfg_param_),
    cell_cfg_list({du_cfg_param_}),
    qos_cfg_list(config_helpers::make_default_du_qos_config_list(/* warn_on_drop */ true, 1000)),
    default_ue_cell_cfg(du_cfg_param.ue_ded_serv_cell_cfg),
    res_mng(std::make_unique<du_ran_resource_manager_impl>(
        cell_cfg_list,
        scheduler_expert_config{.ue = {.max_pucchs_per_slot = max_pucch_grants}},
        srb_cfg_list,
        qos_cfg_list,
        dummy_test_mode_cfg))
  {
    if (params.csi_rs_enabled) {
      default_csi_pucch_res_cfg = std::get<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
          default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type);
    }
  }

  ue_ran_resource_configurator* create_ue(du_ue_index_t ue_index)
  {
    auto result = res_mng->create_ue_resource_configurator(ue_index, to_du_cell_index(0), true);
    if (not result.has_value()) {
      return nullptr;
    }
    ues.emplace(ue_index, std::move(result.value()));
    return &ues[ue_index];
  }

  static f1ap_ue_context_update_request srb1_creation_req(du_ue_index_t ue_index)
  {
    f1ap_ue_context_update_request req;
    req.ue_index = ue_index;
    req.srbs_to_setup.resize(1);
    req.srbs_to_setup[0] = srb_id_t::srb1;
    return req;
  }

  unsigned get_config_sr_period() const
  {
    return sr_periodicity_to_slot(default_ue_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0].period);
  }

  unsigned get_config_csi_period() const
  {
    return csi_report_periodicity_to_uint(default_csi_pucch_res_cfg.report_slot_period);
  }

  bool has_ue_csi_cfg(const serving_cell_config& serv_cell_cfg) const
  {
    return serv_cell_cfg.csi_meas_cfg.has_value() and
           not serv_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list.empty() and
           std::holds_alternative<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
               serv_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type);
  }

  const csi_report_config::periodic_or_semi_persistent_report_on_pucch&
  get_ue_csi_cfg(const serving_cell_config& serv_cell_cfg) const
  {
    srsran_assert(has_ue_csi_cfg(serv_cell_cfg), "CSI configuration not found");

    return std::get<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
        serv_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type);
  }

  unsigned get_nof_sr_slot_offsets() const
  {
    const unsigned slots_per_frame = NOF_SUBFRAMES_PER_FRAME * get_nof_slots_per_subframe(cell_cfg_list[0].scs_common);
    const auto&    sr_res_list     = default_ue_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list;
    unsigned       nof_offsets     = 0;
    for (unsigned i = 0; i != sr_res_list.size(); ++i) {
      const unsigned sr_period_slots = sr_periodicity_to_slot(sr_res_list[i].period);
      if (cell_cfg_list[0].tdd_ul_dl_cfg_common.has_value()) {
        for (unsigned j = 0; j != sr_period_slots; ++j) {
          if (get_active_tdd_ul_symbols(
                  *cell_cfg_list[0].tdd_ul_dl_cfg_common, j % slots_per_frame, cyclic_prefix::NORMAL)
                  .length() == NOF_OFDM_SYM_PER_SLOT_NORMAL_CP) {
            nof_offsets++;
          }
        }
      } else {
        nof_offsets += sr_period_slots;
      }
    }
    // Note: right now we are using two PUCCH resources for SR.
    return nof_offsets * 2;
  }

  bool verify_pucch_cfg(const pucch_config& pucch_cfg, std::optional<unsigned> csi_pucch_res)
  {
    const du_cell_config& du_cfg                = cell_cfg_list[0];
    constexpr unsigned    nof_sr_f1_res_per_ue  = 1U;
    constexpr unsigned    nof_csi_f2_res_per_ue = 1U;
    bool                  pucch_checker =
        pucch_cfg.pucch_res_list.size() == du_cfg.pucch_cfg.nof_ue_pucch_f0_or_f1_res_harq.to_uint() +
                                               du_cfg.pucch_cfg.nof_ue_pucch_f2_or_f3_or_f4_res_harq.to_uint() +
                                               nof_sr_f1_res_per_ue + nof_csi_f2_res_per_ue;

    // Check whether the SR resource point to the correct one (we give a range where the SR resource is located), each
    // UE can have different values within this range.
    pucch_checker = pucch_checker and
                    pucch_cfg.sr_res_list.front().pucch_res_id.cell_res_id >=
                        du_cfg.pucch_cfg.nof_ue_pucch_f0_or_f1_res_harq.to_uint() and
                    pucch_cfg.sr_res_list.front().pucch_res_id.cell_res_id <
                        du_cfg.pucch_cfg.nof_ue_pucch_f0_or_f1_res_harq.to_uint() + du_cfg.pucch_cfg.nof_sr_resources;

    // We always put the CSI PUCCH resource at the end of the list.
    if (csi_pucch_res.has_value()) {
      pucch_checker = pucch_checker and
                      csi_pucch_res.value() >= du_cfg.pucch_cfg.nof_ue_pucch_f0_or_f1_res_harq.to_uint() +
                                                   du_cfg.pucch_cfg.nof_sr_resources +
                                                   du_cfg.pucch_cfg.nof_ue_pucch_f2_or_f3_or_f4_res_harq.to_uint();
    }

    return pucch_checker;
  }

  cell_config_builder_params                                             params;
  du_cell_config                                                         du_cfg_param;
  du_test_mode_config                                                    dummy_test_mode_cfg{};
  std::vector<du_cell_config>                                            cell_cfg_list;
  std::map<srb_id_t, du_srb_config>                                      srb_cfg_list;
  std::map<five_qi_t, du_qos_config>                                     qos_cfg_list;
  const serving_cell_config                                              default_ue_cell_cfg;
  srsran::csi_report_config::periodic_or_semi_persistent_report_on_pucch default_csi_pucch_res_cfg;
  std::unique_ptr<du_ran_resource_manager>                               res_mng;
  slotted_array<ue_ran_resource_configurator, MAX_NOF_DU_UES>            ues;
};

struct params {
  duplex_mode duplx_mode;
};

class du_ran_resource_manager_tester : public du_ran_resource_manager_tester_base,
                                       public ::testing::TestWithParam<params>
{
protected:
  explicit du_ran_resource_manager_tester(
      cell_config_builder_params params_ = {.dl_f_ref_arfcn = GetParam().duplx_mode == duplex_mode::FDD ? 365000U
                                                                                                        : 520002U}) :
    du_ran_resource_manager_tester_base(params_, config_helpers::make_default_du_cell_config(params_))
  {
    srsran_assert(default_ue_cell_cfg.csi_meas_cfg.has_value() and
                      not default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list.empty() and
                      std::holds_alternative<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
                          default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type),
                  "CSI report configuration is required for this unittest;");
  }
};

TEST_P(du_ran_resource_manager_tester, when_ue_resource_config_is_created_then_pcell_is_configured)
{
  const du_ue_index_t                 ue_idx1 = to_du_ue_index(0);
  const ue_ran_resource_configurator* ue_res  = create_ue(ue_idx1);

  ASSERT_NE(ue_res, nullptr);
  ASSERT_FALSE(ue_res->resource_alloc_failed());
  ASSERT_EQ(ue_res->value().cell_group.cells.size(), 1);
  ASSERT_TRUE(ue_res->value().cell_group.cells.contains(0));
  ASSERT_TRUE(ue_res->value().srbs.empty());
  ASSERT_TRUE(ue_res->value().drbs.empty());
  ASSERT_EQ(ue_res->value().cell_group.cells[0].serv_cell_cfg.cell_index, to_du_cell_index(0));
  ASSERT_EQ(ue_res->value().cell_group.cells[0].serv_cell_idx, SERVING_CELL_PCELL_IDX);
  ASSERT_FALSE(ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list.empty());
  ASSERT_FALSE(ue_res->value().cell_group.mcg_cfg.scheduling_request_config.empty());
}

TEST_P(du_ran_resource_manager_tester, when_srb1_is_added_then_ue_resource_config_is_updated)
{
  const du_ue_index_t           ue_idx1 = to_du_ue_index(0);
  ue_ran_resource_configurator* ue_res  = create_ue(ue_idx1);
  ASSERT_NE(ue_res, nullptr);
  auto resp = ue_res->update(to_du_cell_index(0), srb1_creation_req(ue_idx1));

  ASSERT_FALSE(resp.failed());
  ASSERT_EQ(ue_res->value().srbs.size(), 1);
  ASSERT_TRUE(ue_res->value().srbs.contains(srb_id_t::srb1));
  ASSERT_EQ(ue_res->value().srbs[srb_id_t::srb1].rlc_cfg.mode, rlc_mode::am);
}

TEST_P(du_ran_resource_manager_tester, when_multiple_ues_are_created_then_they_use_different_sr_offsets)
{
  const unsigned sr_period       = get_config_sr_period();
  const unsigned slots_per_frame = NOF_SUBFRAMES_PER_FRAME * get_nof_slots_per_subframe(cell_cfg_list[0].scs_common);
  const unsigned nof_avail_sr_offsets = this->get_nof_sr_slot_offsets();
  du_ue_index_t  next_ue_index        = to_du_ue_index(0);

  // > Created UEs have unique (PUCCH resource, SR offset) pairs.
  std::set<std::pair<unsigned, unsigned>> sr_offsets;
  for (unsigned i = 0; i != nof_avail_sr_offsets; ++i) {
    const ue_ran_resource_configurator* ue_res = create_ue(next_ue_index);
    ASSERT_NE(ue_res, nullptr);
    ASSERT_FALSE(ue_res->resource_alloc_failed());
    const auto& sr_res_list =
        ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list;
    ASSERT_FALSE(sr_res_list.empty());
    ASSERT_EQ(sr_periodicity_to_slot(sr_res_list[0].period), sr_period);
    if (cell_cfg_list[0].tdd_ul_dl_cfg_common.has_value()) {
      ASSERT_TRUE(get_active_tdd_ul_symbols(*cell_cfg_list[0].tdd_ul_dl_cfg_common,
                                            sr_res_list[0].offset % slots_per_frame,
                                            cyclic_prefix::NORMAL)
                      .length() == NOF_OFDM_SYM_PER_SLOT_NORMAL_CP);
    }
    ASSERT_EQ(sr_offsets.count(std::make_pair(sr_res_list[0].pucch_res_id.cell_res_id, sr_res_list[0].offset)), 0);
    sr_offsets.insert(std::make_pair(sr_res_list[0].pucch_res_id.cell_res_id, sr_res_list[0].offset));

    // Check if PUCCH config is correctly updated.
    const serving_cell_config serving_cell_cfg = ue_res->value().cell_group.cells[0].serv_cell_cfg;
    std::optional<unsigned>   csi_pucch_res{};
    const bool                has_csi_cfg = serving_cell_cfg.csi_meas_cfg.has_value() and
                             not serving_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list.empty() and
                             std::holds_alternative<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
                                 serving_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type);
    if (has_csi_cfg) {
      csi_pucch_res.emplace(std::get<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
                                serving_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type)
                                .pucch_csi_res_list.front()
                                .pucch_res_id.cell_res_id);
    }
    ASSERT_TRUE(verify_pucch_cfg(
        ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg.value(), csi_pucch_res));

    next_ue_index = to_du_ue_index((unsigned)next_ue_index + 1);
  }

  {
    // > No more SR offsets available. UE Resource Allocation fails.
    const ue_ran_resource_configurator* ue_res_no_resources = create_ue(next_ue_index);
    ASSERT_NE(ue_res_no_resources, nullptr);
    ASSERT_TRUE(ue_res_no_resources->resource_alloc_failed());
    ASSERT_FALSE(
        ue_res_no_resources->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg.has_value());
    ASSERT_TRUE(
        ue_res_no_resources->value().cell_group.cells[0].serv_cell_cfg.csi_meas_cfg->csi_report_cfg_list.empty());
    ues.erase(next_ue_index);
  }

  // Removing one UE, should make one SR offset available.
  const du_ue_index_t ue_idx_to_rem      = to_du_ue_index(test_rgen::uniform_int<unsigned>(0, ues.size() - 1));
  const unsigned      rem_pucch_resource = ues[ue_idx_to_rem]
                                          ->cell_group.cells[0]
                                          .serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0]
                                          .pucch_res_id.cell_res_id;
  const unsigned rem_sr_offset =
      ues[ue_idx_to_rem]->cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0].offset;
  ues.erase(ue_idx_to_rem);
  next_ue_index                              = to_du_ue_index((unsigned)next_ue_index + 1);
  const ue_ran_resource_configurator* ue_res = create_ue(next_ue_index);
  ASSERT_NE(ue_res, nullptr);
  ASSERT_FALSE(ue_res->resource_alloc_failed());
  ASSERT_EQ(rem_pucch_resource,
            ue_res->value()
                .cell_group.cells[0]
                .serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0]
                .pucch_res_id.cell_res_id);
  ASSERT_EQ(rem_sr_offset,
            ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0].offset);
}

INSTANTIATE_TEST_SUITE_P(du_ran_resource_manager_tester,
                         du_ran_resource_manager_tester,
                         ::testing::Values(params{duplex_mode::FDD}, params{duplex_mode::TDD}));

/////////     Test DU RAN resource manager for multiple UE PUCCH configurations      /////////

namespace du_test_multiple_pucch_cfg {

struct pucch_cfg_builder_params {
  unsigned nof_res_f1_harq = 3;
  unsigned nof_res_f2_harq = 6;
  unsigned nof_harq_cfg    = 1;
  unsigned nof_res_sr      = 2;
  unsigned nof_res_csi     = 1;
};

struct pucch_cnt_builder_params {
  unsigned               nof_res_sr               = 2;
  unsigned               nof_res_csi              = 1;
  unsigned               max_allowed_pucch_grants = 4;
  sr_periodicity         sr_period                = sr_periodicity::sl_40;
  csi_report_periodicity csi_period               = csi_report_periodicity::slots80;
};

std::ostream& operator<<(std::ostream& out, const pucch_cnt_builder_params& params)
{
  out << fmt::format("nof_res_sr={}, nof_res_csi={}, max_pucch_grants={}, sr_period_slots={}, csi_period_slots={}",
                     params.nof_res_sr,
                     params.nof_res_csi,
                     params.max_allowed_pucch_grants,
                     sr_periodicity_to_slot(params.sr_period),
                     csi_report_periodicity_to_uint(params.csi_period));
  return out;
}

} // namespace du_test_multiple_pucch_cfg

using namespace du_test_multiple_pucch_cfg;

static du_cell_config make_custom_du_cell_config(const pucch_cfg_builder_params& pucch_params_)
{
  du_cell_config du_cfg                             = config_helpers::make_default_du_cell_config();
  auto&          pucch_params                       = du_cfg.pucch_cfg;
  pucch_params.nof_ue_pucch_f0_or_f1_res_harq       = pucch_params_.nof_res_f1_harq;
  pucch_params.nof_ue_pucch_f2_or_f3_or_f4_res_harq = pucch_params_.nof_res_f2_harq;
  pucch_params.nof_sr_resources                     = pucch_params_.nof_res_sr;
  pucch_params.nof_csi_resources                    = pucch_params_.nof_res_csi;
  pucch_params.nof_cell_harq_pucch_res_sets         = pucch_params_.nof_harq_cfg;
  auto& f1_params                                   = std::get<pucch_f1_params>(pucch_params.f0_or_f1_params);
  f1_params.nof_cyc_shifts                          = pucch_nof_cyclic_shifts::six;
  f1_params.occ_supported                           = true;

  return du_cfg;
}

static const auto* get_pucch_resource_with_id(const pucch_config& pucch_cfg, unsigned res_id)
{
  return std::find_if(pucch_cfg.pucch_res_list.begin(),
                      pucch_cfg.pucch_res_list.end(),
                      [res_id](const pucch_resource& res) { return res.res_id.cell_res_id == res_id; });
}

class du_ran_res_mng_multiple_cfg_tester : public du_ran_resource_manager_tester_base,
                                           public ::testing::TestWithParam<pucch_cfg_builder_params>
{
protected:
  explicit du_ran_res_mng_multiple_cfg_tester() :
    du_ran_resource_manager_tester_base(cell_config_builder_params{}, make_custom_du_cell_config(GetParam()))
  {
    srsran_assert(default_ue_cell_cfg.csi_meas_cfg.has_value() and
                      not default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list.empty() and
                      std::holds_alternative<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
                          default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type),
                  "CSI report configuration is required for this unittest;");
  }

  // Retrieve the interval [ ID_start, ID_stop] with the initial and final IDs of the UE's PUCCH resources used for
  // HARQ from the UE's PUCCH-Config.
  interval<unsigned, true> get_pucch_res_id_interval(const pucch_config& pucch_cfg, pucch_format format) const
  {
    const unsigned pucch_res_set_id = format == pucch_format::FORMAT_1 ? 0U : 1U;
    const auto&    pucch_res_set    = pucch_cfg.pucch_res_set[pucch_res_set_id].pucch_res_id_list;
    const unsigned expected_pucch_res_set_size =
        format == pucch_format::FORMAT_1 ? cell_cfg_list[0].pucch_cfg.nof_ue_pucch_f0_or_f1_res_harq.to_uint()
                                         : cell_cfg_list[0].pucch_cfg.nof_ue_pucch_f2_or_f3_or_f4_res_harq.to_uint();
    if (expected_pucch_res_set_size != pucch_res_set.size()) {
      return {};
    }

    const auto* pucch_res_id_start_it = get_pucch_resource_with_id(pucch_cfg, pucch_res_set.front().cell_res_id);
    const auto* pucch_res_id_stop_it  = get_pucch_resource_with_id(pucch_cfg, pucch_res_set.back().cell_res_id);
    if (pucch_res_id_start_it != pucch_cfg.pucch_res_list.end() and
        pucch_res_id_stop_it != pucch_cfg.pucch_res_list.end()) {
      return {pucch_res_id_start_it->res_id.cell_res_id, pucch_res_id_stop_it->res_id.cell_res_id};
    }

    return {};
  }

  // Retrieve the expected interval [ ID_start, ID_stop] with the initial and final IDs of the UE's PUCCH resources used
  // for HARQ.
  interval<unsigned, true> get_expected_pucch_res_id_interval(unsigned ue_idx, pucch_format format) const
  {
    const unsigned expected_nof_pucch_res =
        format == pucch_format::FORMAT_1 ? cell_cfg_list[0].pucch_cfg.nof_ue_pucch_f0_or_f1_res_harq.to_uint()
                                         : cell_cfg_list[0].pucch_cfg.nof_ue_pucch_f2_or_f3_or_f4_res_harq.to_uint();

    if (expected_nof_pucch_res == 0) {
      return interval<unsigned, true>{};
    }

    const unsigned nof_harq_cfgs = cell_cfg_list[0].pucch_cfg.nof_cell_harq_pucch_res_sets;
    const unsigned f2_res_idx_offset =
        format == pucch_format::FORMAT_1
            ? 0U
            : cell_cfg_list[0].pucch_cfg.nof_ue_pucch_f0_or_f1_res_harq.to_uint() * nof_harq_cfgs +
                  cell_cfg_list[0].pucch_cfg.nof_sr_resources;
    return {f2_res_idx_offset + (ue_idx % nof_harq_cfgs) * expected_nof_pucch_res,
            f2_res_idx_offset + (ue_idx % nof_harq_cfgs) * expected_nof_pucch_res + expected_nof_pucch_res - 1};
  }

  // Get the number of available resources and offsets for SR/CSI. The return value is the minimum(available_SR_offset *
  // available_SR_resources, available_CSI_offset * available_CSI_resources). The booleans indicates whether the
  // resources and offsets are limited by the SR (bool_0 == true) or CSI (bool_1 == true), or both (bool_0 == true and
  // bool_1 == true).
  std::tuple<unsigned, bool, bool> get_nof_available_resources() const
  {
    const unsigned slots_per_frame = NOF_SUBFRAMES_PER_FRAME * get_nof_slots_per_subframe(cell_cfg_list[0].scs_common);
    const auto&    sr_res_list     = default_ue_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list;
    unsigned       nof_sr_offsets  = 0;
    unsigned       nof_csi_offsets = 0;

    // Get the available offsets for SR.
    const unsigned sr_period_slots = sr_periodicity_to_slot(sr_res_list.front().period);
    if (cell_cfg_list[0].tdd_ul_dl_cfg_common.has_value()) {
      for (unsigned j = 0; j != sr_period_slots; ++j) {
        if (get_active_tdd_ul_symbols(
                *cell_cfg_list[0].tdd_ul_dl_cfg_common, j % slots_per_frame, cyclic_prefix::NORMAL)
                .length() == NOF_OFDM_SYM_PER_SLOT_NORMAL_CP) {
          nof_sr_offsets++;
        }
      }
    } else {
      nof_sr_offsets += sr_period_slots;
    }
    const unsigned nof_avail_sr_res = nof_sr_offsets * cell_cfg_list[0].pucch_cfg.nof_sr_resources;

    // Get the available offsets for CSI.
    const unsigned csi_period_slots = csi_report_periodicity_to_uint(default_csi_pucch_res_cfg.report_slot_period);
    if (cell_cfg_list[0].tdd_ul_dl_cfg_common.has_value()) {
      for (unsigned j = 0; j != csi_period_slots; ++j) {
        if (get_active_tdd_ul_symbols(
                *cell_cfg_list[0].tdd_ul_dl_cfg_common, j % slots_per_frame, cyclic_prefix::NORMAL)
                .length() == NOF_OFDM_SYM_PER_SLOT_NORMAL_CP) {
          nof_csi_offsets++;
        }
      }
    } else {
      nof_csi_offsets += csi_period_slots;
    }

    const unsigned nof_avail_csi_res = nof_csi_offsets * cell_cfg_list[0].pucch_cfg.nof_csi_resources;

    // Note: right now we are using two PUCCH resources for SR.
    return {std::min(nof_avail_sr_res, nof_avail_csi_res),
            nof_avail_sr_res <= nof_avail_csi_res,
            nof_avail_csi_res <= nof_avail_sr_res};
  }
};

TEST_P(du_ran_res_mng_multiple_cfg_tester, test_correct_resource_creation_indexing)
{
  const unsigned sr_period       = get_config_sr_period();
  const unsigned csi_period      = get_config_csi_period();
  const unsigned slots_per_frame = NOF_SUBFRAMES_PER_FRAME * get_nof_slots_per_subframe(cell_cfg_list[0].scs_common);
  std::tuple<unsigned, bool, bool> avail_res     = get_nof_available_resources();
  du_ue_index_t                    next_ue_index = to_du_ue_index(0);

  // > Created UEs have unique (PUCCH resource, SR offset) pairs.
  std::set<std::pair<unsigned, unsigned>> sr_offsets;
  std::set<std::pair<unsigned, unsigned>> csi_offsets;
  for (unsigned i = 0; i != std::get<0>(avail_res); ++i) {
    const ue_ran_resource_configurator* ue_res = create_ue(next_ue_index);
    ASSERT_NE(ue_res, nullptr);
    ASSERT_FALSE(ue_res->resource_alloc_failed());

    // Check if the SR has been assigned to the UE.
    const auto& sr_res_list =
        ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list;
    ASSERT_FALSE(sr_res_list.empty());
    ASSERT_EQ(sr_periodicity_to_slot(sr_res_list[0].period), sr_period);
    // Make sure the SR is in a fully-UL slot.
    if (cell_cfg_list[0].tdd_ul_dl_cfg_common.has_value()) {
      ASSERT_TRUE(get_active_tdd_ul_symbols(*cell_cfg_list[0].tdd_ul_dl_cfg_common,
                                            sr_res_list[0].offset % slots_per_frame,
                                            cyclic_prefix::NORMAL)
                      .length() == NOF_OFDM_SYM_PER_SLOT_NORMAL_CP);
    }
    ASSERT_EQ(sr_offsets.count(std::make_pair(sr_res_list[0].pucch_res_id.cell_res_id, sr_res_list[0].offset)), 0);
    sr_offsets.insert(std::make_pair(sr_res_list[0].pucch_res_id.cell_res_id, sr_res_list[0].offset));

    // Check if the CSI has been assigned to the UE.
    ASSERT_TRUE(has_ue_csi_cfg(ue_res->value().cell_group.cells[0].serv_cell_cfg));
    const auto& ue_csi_cfg = get_ue_csi_cfg(ue_res->value().cell_group.cells[0].serv_cell_cfg);
    ASSERT_FALSE(ue_csi_cfg.pucch_csi_res_list.empty());
    const unsigned ue_csi_pucch_res_id = ue_csi_cfg.pucch_csi_res_list.front().pucch_res_id.cell_res_id;
    const unsigned ue_csi_pucch_offset = ue_csi_cfg.report_slot_offset;
    ASSERT_EQ(csi_period, csi_report_periodicity_to_uint(ue_csi_cfg.report_slot_period));
    // Make sure the CSI is in a fully-UL slot.
    if (cell_cfg_list[0].tdd_ul_dl_cfg_common.has_value()) {
      ASSERT_TRUE(get_active_tdd_ul_symbols(*cell_cfg_list[0].tdd_ul_dl_cfg_common,
                                            ue_csi_pucch_offset % slots_per_frame,
                                            cyclic_prefix::NORMAL)
                      .length() == NOF_OFDM_SYM_PER_SLOT_NORMAL_CP);
    }
    ASSERT_EQ(csi_offsets.count(std::make_pair(ue_csi_pucch_res_id, ue_csi_pucch_offset)), 0);
    csi_offsets.insert(std::make_pair(ue_csi_pucch_res_id, ue_csi_pucch_offset));

    // Check if PUCCH resources for HARQ indexing is distributed over different HARQ configurations.
    const interval<unsigned, true> expected_f1 =
        get_expected_pucch_res_id_interval(static_cast<unsigned>(next_ue_index), srsran::pucch_format::FORMAT_1);
    const interval<unsigned, true> actual_f1 = get_pucch_res_id_interval(
        ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config.value().init_ul_bwp.pucch_cfg.value(),
        srsran::pucch_format::FORMAT_1);
    const interval<unsigned, true> expected_f2 =
        get_expected_pucch_res_id_interval(static_cast<unsigned>(next_ue_index), srsran::pucch_format::FORMAT_2);
    const interval<unsigned, true> actual_f2 = get_pucch_res_id_interval(
        ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config.value().init_ul_bwp.pucch_cfg.value(),
        srsran::pucch_format::FORMAT_2);

    ASSERT_TRUE(expected_f1.start() == actual_f1.start() and expected_f1.stop() == actual_f1.stop());
    ASSERT_TRUE(expected_f2.start() == actual_f2.start() and expected_f2.stop() == actual_f2.stop());

    next_ue_index = to_du_ue_index((unsigned)next_ue_index + 1);
  }

  {
    // > No more SR offsets available. UE Resource Allocation fails.
    const ue_ran_resource_configurator* empty_ue_res = create_ue(next_ue_index);
    ASSERT_NE(empty_ue_res, nullptr);
    ASSERT_TRUE(empty_ue_res->resource_alloc_failed());
    ues.erase(next_ue_index);
  }

  // Remove 1 UE and verify if the new resource can be allocated to another UE.
  const du_ue_index_t ue_idx_to_rem         = to_du_ue_index(test_rgen::uniform_int<unsigned>(0, ues.size() - 1));
  const unsigned      rem_sr_pucch_resource = ues[ue_idx_to_rem]
                                             ->cell_group.cells[0]
                                             .serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0]
                                             .pucch_res_id.cell_res_id;
  const unsigned rem_sr_offset =
      ues[ue_idx_to_rem]->cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0].offset;
  const unsigned rem_csi_pucch_resource_id = get_ue_csi_cfg(ues[ue_idx_to_rem]->cell_group.cells[0].serv_cell_cfg)
                                                 .pucch_csi_res_list.front()
                                                 .pucch_res_id.cell_res_id;
  const unsigned rem_csi_offset =
      get_ue_csi_cfg(ues[ue_idx_to_rem]->cell_group.cells[0].serv_cell_cfg).report_slot_offset;

  ues.erase(ue_idx_to_rem);
  next_ue_index                              = to_du_ue_index((unsigned)next_ue_index + 1);
  const ue_ran_resource_configurator* ue_res = create_ue(next_ue_index);
  ASSERT_NE(ue_res, nullptr);
  ASSERT_FALSE(ue_res->resource_alloc_failed());

  // If the resources and offset were limited by the SR, then check if a new SR can be allocated.
  const bool nof_ue_limited_by_sr_resources = std::get<1>(avail_res);
  if (nof_ue_limited_by_sr_resources) {
    ASSERT_EQ(rem_sr_pucch_resource,
              ue_res->value()
                  .cell_group.cells[0]
                  .serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0]
                  .pucch_res_id.cell_res_id);
    ASSERT_EQ(
        rem_sr_offset,
        ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list[0].offset);
  }

  // If the resources and offset were limited by the CSI, then check if a new CSI can be allocated.
  const bool nof_ue_limited_by_csi_resources = std::get<2>(avail_res);
  if (nof_ue_limited_by_csi_resources) {
    ASSERT_EQ(rem_csi_pucch_resource_id,
              get_ue_csi_cfg(ue_res->value().cell_group.cells[0].serv_cell_cfg)
                  .pucch_csi_res_list.front()
                  .pucch_res_id.cell_res_id);
    ASSERT_EQ(rem_csi_offset, get_ue_csi_cfg(ue_res->value().cell_group.cells[0].serv_cell_cfg).report_slot_offset);
  }
}

INSTANTIATE_TEST_SUITE_P(different_f1_f2_resources,
                         du_ran_res_mng_multiple_cfg_tester,
                         // clang-format off
                         //                                   nof:  f1  |  f2  | harq | sr | csi
                         //                                   nof:  f1  |  f2  | cfg  | sr | csi
                         ::testing::Values(pucch_cfg_builder_params{ 3,     6,     1,    4,   1 },
                                           pucch_cfg_builder_params{ 3,     6,     3,    6,   3 },
                                           pucch_cfg_builder_params{ 8,     8,     5,    4,   7 },
                                           pucch_cfg_builder_params{ 8,     3,     1,    1,   1 },
                                           pucch_cfg_builder_params{ 5,     7,     7,    4,   9 },
                                           pucch_cfg_builder_params{ 8,     8,     8,    4,   10 })
                         // clang-format on
);

static du_cell_config
make_custom_du_cell_config_for_pucch_cnt(const pucch_cnt_builder_params&                            pucch_params_,
                                         const config_helpers::cell_config_builder_params_extended& params = {})
{
  du_cell_config du_cfg          = config_helpers::make_default_du_cell_config(params);
  auto&          pucch_params    = du_cfg.pucch_cfg;
  pucch_params.nof_sr_resources  = pucch_params_.nof_res_sr;
  pucch_params.nof_csi_resources = pucch_params_.nof_res_csi;
  auto& f1_params                = std::get<pucch_f1_params>(pucch_params.f0_or_f1_params);
  f1_params.nof_cyc_shifts       = pucch_nof_cyclic_shifts::six;
  f1_params.occ_supported        = true;

  du_cfg.ue_ded_serv_cell_cfg.ul_config.value().init_ul_bwp.pucch_cfg->sr_res_list[0].period = pucch_params_.sr_period;
  if (du_cfg.ue_ded_serv_cell_cfg.csi_meas_cfg.has_value()) {
    std::get<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
        du_cfg.ue_ded_serv_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type)
        .report_slot_period = pucch_params_.csi_period;
  }

  return du_cfg;
}

class du_ran_res_mng_pucch_cnt_tester : public du_ran_resource_manager_tester_base,
                                        public ::testing::TestWithParam<pucch_cnt_builder_params>
{
protected:
  explicit du_ran_res_mng_pucch_cnt_tester(cell_config_builder_params params_ = {.dl_f_ref_arfcn = 520002U}) :
    du_ran_resource_manager_tester_base(params_,
                                        make_custom_du_cell_config_for_pucch_cnt(GetParam(), params_),
                                        GetParam().max_allowed_pucch_grants)
  {
    srsran_assert(default_ue_cell_cfg.csi_meas_cfg.has_value() and
                      not default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list.empty() and
                      std::holds_alternative<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
                          default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type),
                  "CSI report configuration is required for this unittest;");
    lcm_csi_sr_period =
        std::lcm(sr_periodicity_to_slot(GetParam().sr_period), csi_report_periodicity_to_uint(GetParam().csi_period));
    pucch_cnts.resize(lcm_csi_sr_period, 0);
  }

  unsigned              lcm_csi_sr_period;
  std::vector<unsigned> pucch_cnts;
};

TEST_P(du_ran_res_mng_pucch_cnt_tester, test_du_pucch_cnt)
{
  du_ue_index_t next_ue_index = to_du_ue_index(0);

  // > Created UEs have unique (PUCCH resource, SR offset) pairs.
  std::set<std::pair<unsigned, unsigned>> sr_offsets;
  std::set<std::pair<unsigned, unsigned>> csi_offsets;
  for (unsigned i = 0; i != 1000; ++i) {
    const ue_ran_resource_configurator* ue_res = create_ue(next_ue_index);
    if (ue_res == nullptr) {
      break;
    }
    if (ue_res->resource_alloc_failed()) {
      ues.erase(next_ue_index);
      break;
    }

    // Check if the SR has been assigned to the UE.
    const auto& sr_res_list =
        ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list;
    ASSERT_FALSE(sr_res_list.empty());
    ASSERT_EQ(sr_offsets.count(std::make_pair(sr_res_list[0].pucch_res_id.cell_res_id, sr_res_list[0].offset)), 0);
    sr_offsets.insert(std::make_pair(sr_res_list[0].pucch_res_id.cell_res_id, sr_res_list[0].offset));
    unsigned sr_offset = sr_res_list[0].offset;

    // Check if the CSI has been assigned to the UE.
    ASSERT_TRUE(has_ue_csi_cfg(ue_res->value().cell_group.cells[0].serv_cell_cfg));
    const auto& ue_csi_cfg = get_ue_csi_cfg(ue_res->value().cell_group.cells[0].serv_cell_cfg);
    ASSERT_FALSE(ue_csi_cfg.pucch_csi_res_list.empty());
    const unsigned csi_pucch_res_id = ue_csi_cfg.pucch_csi_res_list.front().pucch_res_id.cell_res_id;
    const unsigned csi_offset       = ue_csi_cfg.report_slot_offset;
    ASSERT_EQ(csi_offsets.count(std::make_pair(csi_pucch_res_id, csi_offset)), 0);
    csi_offsets.insert(std::make_pair(csi_pucch_res_id, csi_offset));

    // Check the offsets results in the SR and CSI not exceeding the max PUCCH grants.
    std::set<unsigned> csi_sr_offset_for_pucch_cnt;
    for (unsigned sr_off = sr_offset; sr_off < lcm_csi_sr_period; sr_off += get_config_sr_period()) {
      csi_sr_offset_for_pucch_cnt.emplace(sr_off);
    }

    for (unsigned csi_off = csi_offset; csi_off < lcm_csi_sr_period; csi_off += get_config_csi_period()) {
      csi_sr_offset_for_pucch_cnt.emplace(csi_off);
    }

    for (auto offset : csi_sr_offset_for_pucch_cnt) {
      srsran_assert(offset < static_cast<unsigned>(pucch_cnts.size()),
                    "Index exceeds the size of the PUCCH grants vector");
      ++pucch_cnts[offset];
    }

    for (auto pucch_slot_cnt : pucch_cnts) {
      ASSERT_TRUE(pucch_slot_cnt <= GetParam().max_allowed_pucch_grants);
    }

    next_ue_index = to_du_ue_index((unsigned)next_ue_index + 1);
  }

  // Erase a random UE and attempt.
  const du_ue_index_t ue_idx_to_rem = to_du_ue_index(test_rgen::uniform_int<unsigned>(0, ues.size() - 1));
  ues.erase(ue_idx_to_rem);

  // Attempt a new allocation and verify it is successful.
  next_ue_index                              = to_du_ue_index((unsigned)next_ue_index + 1);
  const ue_ran_resource_configurator* ue_res = create_ue(next_ue_index);
  ASSERT_NE(ue_res, nullptr);
  ASSERT_FALSE(ue_res->resource_alloc_failed());
}

INSTANTIATE_TEST_SUITE_P(
    parametrized_test_for_du_pucch_cnt,
    du_ran_res_mng_pucch_cnt_tester,
    // clang-format off
       ::testing::Values(
       pucch_cnt_builder_params{ 2, 2, 5, srsran::sr_periodicity::sl_10, srsran::csi_report_periodicity::slots10 },
       pucch_cnt_builder_params{ 2, 2, 5, srsran::sr_periodicity::sl_20, srsran::csi_report_periodicity::slots20 },
       pucch_cnt_builder_params{ 2, 2, 5, srsran::sr_periodicity::sl_20, srsran::csi_report_periodicity::slots20 },
       pucch_cnt_builder_params{ 7, 4, 11, srsran::sr_periodicity::sl_20, srsran::csi_report_periodicity::slots40 },
       pucch_cnt_builder_params{ 10, 5, 15,  srsran::sr_periodicity::sl_40, srsran::csi_report_periodicity::slots80 },
       pucch_cnt_builder_params{ 19, 20, 29, srsran::sr_periodicity::sl_40, srsran::csi_report_periodicity::slots80 }
       )
    // clang-format on
);

class du_ran_res_mng_pucch_cnt_sr_only_tester : public du_ran_resource_manager_tester_base,
                                                public ::testing::TestWithParam<pucch_cnt_builder_params>
{
protected:
  explicit du_ran_res_mng_pucch_cnt_sr_only_tester(cell_config_builder_params params_ = {.dl_f_ref_arfcn = 520002U,
                                                                                         .csi_rs_enabled = false}) :
    du_ran_resource_manager_tester_base(params_,
                                        make_custom_du_cell_config_for_pucch_cnt(GetParam(), params_),
                                        GetParam().max_allowed_pucch_grants)
  {
    pucch_cnts.resize(sr_periodicity_to_slot(GetParam().sr_period), 0);
  }

  std::vector<unsigned> pucch_cnts;
};

TEST_P(du_ran_res_mng_pucch_cnt_sr_only_tester, test_du_pucch_cnt_sr_only)
{
  du_ue_index_t next_ue_index = to_du_ue_index(0);

  // > Created UEs have unique (PUCCH resource, SR offset) pairs.
  std::set<std::pair<unsigned, unsigned>> sr_offsets;
  for (unsigned i = 0; i != 1000; ++i) {
    const ue_ran_resource_configurator* ue_res = create_ue(next_ue_index);
    if (ue_res == nullptr) {
      break;
    }
    if (ue_res->resource_alloc_failed()) {
      ues.erase(next_ue_index);
      break;
    }

    // Check if the SR has been assigned to the UE.
    const auto& sr_res_list =
        ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg->sr_res_list;
    ASSERT_FALSE(sr_res_list.empty());
    ASSERT_EQ(sr_offsets.count(std::make_pair(sr_res_list[0].pucch_res_id.cell_res_id, sr_res_list[0].offset)), 0);
    sr_offsets.insert(std::make_pair(sr_res_list[0].pucch_res_id.cell_res_id, sr_res_list[0].offset));
    unsigned sr_offset = sr_res_list[0].offset;

    srsran_assert(sr_offset < static_cast<unsigned>(pucch_cnts.size()),
                  "Index exceeds the size of the PUCCH grants vector");
    ++pucch_cnts[sr_offset];

    for (auto pucch_slot_cnt : pucch_cnts) {
      ASSERT_TRUE(pucch_slot_cnt <= GetParam().max_allowed_pucch_grants);
    }

    next_ue_index = to_du_ue_index((unsigned)next_ue_index + 1);
  }

  // Erase a random UE and attempt.
  const du_ue_index_t ue_idx_to_rem = to_du_ue_index(test_rgen::uniform_int<unsigned>(0, ues.size() - 1));
  ues.erase(ue_idx_to_rem);

  // Attempt a new allocation and verify it is successful.
  next_ue_index                              = to_du_ue_index((unsigned)next_ue_index + 1);
  const ue_ran_resource_configurator* ue_res = create_ue(next_ue_index);
  ASSERT_NE(ue_res, nullptr);
  ASSERT_FALSE(ue_res->resource_alloc_failed());
}

INSTANTIATE_TEST_SUITE_P(
    parametrized_test_for_du_pucch_cnt,
    du_ran_res_mng_pucch_cnt_sr_only_tester,
    // clang-format off
       ::testing::Values(
       pucch_cnt_builder_params{ 5, 0, 5, srsran::sr_periodicity::sl_10, srsran::csi_report_periodicity::slots10 },
       pucch_cnt_builder_params{ 5, 0, 5, srsran::sr_periodicity::sl_20, srsran::csi_report_periodicity::slots10 },
       pucch_cnt_builder_params{ 11, 0, 11, srsran::sr_periodicity::sl_20, srsran::csi_report_periodicity::slots10 },
       pucch_cnt_builder_params{ 15, 0, 15,  srsran::sr_periodicity::sl_40, srsran::csi_report_periodicity::slots10 },
       pucch_cnt_builder_params{ 29, 0, 29, srsran::sr_periodicity::sl_40, srsran::csi_report_periodicity::slots10 }
       )
    // clang-format on
);

/////////     Test the DU RAN resource manager for both PUCCH and SRS resources      /////////

// This helper builds a DU cell configuration with a custom PUCCH and SRS configuration. The input parameter
// pucch_has_more_res_than_srs indicates whether the PUCCH or SRS configuration allows more resources than the other.
// This way, we can test the allocation and de-allocation of resources when the DU rejects a UE due to lack of PUCCH or
// SRS resources.
static du_cell_config make_custom_pucch_srs_du_cell_config(bool pucch_has_more_res_than_srs)
{
  du_cell_config du_cfg                             = config_helpers::make_default_du_cell_config();
  auto&          pucch_params                       = du_cfg.pucch_cfg;
  pucch_params.nof_ue_pucch_f0_or_f1_res_harq       = 6U;
  pucch_params.nof_ue_pucch_f2_or_f3_or_f4_res_harq = 6U;
  pucch_params.nof_sr_resources                     = pucch_has_more_res_than_srs ? 10U : 1U;
  pucch_params.nof_csi_resources                    = pucch_has_more_res_than_srs ? 10U : 1U;
  pucch_params.nof_cell_harq_pucch_res_sets         = 1U;
  auto& f1_params                                   = std::get<pucch_f1_params>(pucch_params.f0_or_f1_params);
  f1_params.nof_cyc_shifts                          = pucch_nof_cyclic_shifts::no_cyclic_shift;
  f1_params.occ_supported                           = false;

  auto& tdd_cfg                              = du_cfg.tdd_ul_dl_cfg_common.emplace();
  tdd_cfg.pattern1.dl_ul_tx_period_nof_slots = 10;
  tdd_cfg.pattern1.nof_dl_slots              = 7;
  tdd_cfg.pattern1.nof_dl_symbols            = 10;
  tdd_cfg.pattern1.nof_ul_slots              = 2;
  tdd_cfg.pattern1.nof_ul_symbols            = 0;

  du_cfg.ue_ded_serv_cell_cfg.ul_config.value().init_ul_bwp.pucch_cfg.value().sr_res_list.front().period =
      srsran::sr_periodicity::sl_10;

  auto& srs_cfg = du_cfg.srs_cfg;

  // Generates a random SRS configuration.
  srs_cfg.tx_comb                   = tx_comb_size::n2;
  srs_cfg.max_nof_symbols           = 1U;
  srs_cfg.nof_symbols               = srs_nof_symbols::n1;
  srs_cfg.cyclic_shift_reuse_factor = nof_cyclic_shifts::no_cyclic_shift;
  srs_cfg.sequence_id_reuse_factor  = 1U;
  srs_cfg.srs_period.emplace(du_cfg.tdd_ul_dl_cfg_common.has_value() ? srs_periodicity::sl10 : srs_periodicity::sl1);

  pucch_params.max_nof_symbols = NOF_OFDM_SYM_PER_SLOT_NORMAL_CP - srs_cfg.max_nof_symbols.to_uint();
  f1_params.nof_symbols        = std::min(f1_params.nof_symbols.to_uint(), pucch_params.max_nof_symbols.to_uint());

  return du_cfg;
}

class du_ran_res_mng_pucch_srs_tester : public du_ran_resource_manager_tester_base,
                                        public ::testing::TestWithParam<bool>
{
protected:
  explicit du_ran_res_mng_pucch_srs_tester() :
    du_ran_resource_manager_tester_base(cell_config_builder_params{}, make_custom_pucch_srs_du_cell_config(GetParam()))
  {
    srsran_assert(default_ue_cell_cfg.csi_meas_cfg.has_value() and
                      not default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list.empty() and
                      std::holds_alternative<csi_report_config::periodic_or_semi_persistent_report_on_pucch>(
                          default_ue_cell_cfg.csi_meas_cfg.value().csi_report_cfg_list[0].report_cfg_type),
                  "CSI report configuration is required for this unittest;");
  }
};

TEST_P(du_ran_res_mng_pucch_srs_tester, when_alloc_fail_ue_has_no_srs_and_no_pucch_cfgs)
{
  // This tests how the DU handles the RAN resource allocation failure due to lack of PUCCH or SRS resources.
  for (unsigned i = 0; i != MAX_NOF_DU_UES; ++i) {
    du_ue_index_t                       next_ue_index = to_du_ue_index(i);
    const ue_ran_resource_configurator* ue_res        = create_ue(next_ue_index);
    ASSERT_NE(ue_res, nullptr);
    if (ue_res->resource_alloc_failed()) {
      ASSERT_FALSE(ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg.has_value());
      ASSERT_TRUE(ue_res->value().cell_group.cells[0].serv_cell_cfg.csi_meas_cfg->csi_report_cfg_list.empty());
      ASSERT_FALSE(ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.srs_cfg.has_value());
      break;
    } else {
      ASSERT_TRUE(ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.pucch_cfg.has_value());
      ASSERT_FALSE(ue_res->value().cell_group.cells[0].serv_cell_cfg.csi_meas_cfg->csi_report_cfg_list.empty());
      ASSERT_TRUE(ue_res->value().cell_group.cells[0].serv_cell_cfg.ul_config->init_ul_bwp.srs_cfg.has_value());
    }
  }
}

INSTANTIATE_TEST_SUITE_P(different_f1_f2_resources, du_ran_res_mng_pucch_srs_tester, ::testing::Values(true, false));
