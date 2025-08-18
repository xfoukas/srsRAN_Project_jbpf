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

#include "lib/du/du_high/du_manager/du_ue/du_bearer.h"
#include "lib/du/du_high/du_manager/du_ue/du_ue_bearer_manager.h"
#include "tests/unittests/du_manager/du_manager_test_helpers.h"
#include "srsran/du/du_cell_config_helpers.h"
#include "srsran/support/test_utils.h"
#include <gtest/gtest.h>
#include <numeric>

using namespace srsran;
using namespace srs_du;

namespace {

class dummy_rlc_rlf_notifier : public rlc_tx_upper_layer_control_notifier
{
public:
  void on_protocol_failure() override {}

  void on_max_retx() override {}
};

} // namespace

class du_ue_bearer_manager_test : public ::testing::Test
{
protected:
  du_ue_bearer_manager_test()
  {
    du_mng = std::make_unique<du_manager_test_bench>(
        std::vector<du_cell_config>{config_helpers::make_default_du_cell_config()});
    dummy_slice_info = s_nssai_t{.sst = slice_service_type{1}};
  }

  void SetUp() override
  {
    // init test's logger
    srslog::init();
  }

  void TearDown() override
  {
    // flush logger after each test
    srslog::flush();
  }

  std::unique_ptr<du_manager_test_bench> du_mng = std::make_unique<du_manager_test_bench>(
      std::vector<du_cell_config>{config_helpers::make_default_du_cell_config()});
  dummy_teid_pool        teid_pool;
  dummy_rlc_rlf_notifier rlf_notifier;
  s_nssai_t              dummy_slice_info = s_nssai_t{.sst = slice_service_type{1}};

  std::unique_ptr<du_ue_drb> create_dummy_drb(drb_id_t drb_id, lcid_t lcid)
  {
    std::array<up_transport_layer_info, 1> ul_tnls = {
        up_transport_layer_info{transport_layer_address::create_from_string("127.0.0.1"), gtpu_teid_t{0}}};
    return create_drb(drb_creation_info{to_du_ue_index(0),
                                        to_du_cell_index(0),
                                        drb_id,
                                        lcid,
                                        five_qi_t{9},
                                        rlc_config{},
                                        f1u_config{},
                                        false,
                                        ul_tnls,
                                        teid_pool,
                                        du_mng->params,
                                        rlf_notifier});
  }
};

TEST_F(du_ue_bearer_manager_test, when_no_drbs_allocated_lcid_is_min)
{
  du_ue_bearer_manager bearers;
  ASSERT_EQ(bearers.allocate_lcid(), LCID_MIN_DRB);
}

TEST_F(du_ue_bearer_manager_test, when_all_drbs_are_allocated_then_no_lcid_is_available)
{
  std::vector<unsigned> lcids(MAX_NOF_DRBS);
  std::iota(lcids.begin(), lcids.end(), (unsigned)LCID_MIN_DRB);
  std::shuffle(lcids.begin(), lcids.end(), test_rgen::get());

  du_ue_bearer_manager bearers;
  for (unsigned i = 0; i != MAX_NOF_DRBS; ++i) {
    drb_id_t drb_id = (drb_id_t)((unsigned)drb_id_t::drb1 + i);
    bearers.add_drb(create_dummy_drb(drb_id, uint_to_lcid(lcids[i])));
  }
  ASSERT_FALSE(bearers.allocate_lcid().has_value());
}

TEST_F(du_ue_bearer_manager_test, when_there_is_a_hole_in_allocated_lcids_then_allocate_lcid_method_finds_it)
{
  std::vector<unsigned> lcids(MAX_NOF_DRBS);
  std::iota(lcids.begin(), lcids.end(), (unsigned)LCID_MIN_DRB);
  std::shuffle(lcids.begin(), lcids.end(), test_rgen::get());
  lcid_t lcid_hole = uint_to_lcid(lcids.back());
  lcids.pop_back();
  std::shuffle(lcids.begin(), lcids.end(), test_rgen::get());

  du_ue_bearer_manager bearers;
  for (unsigned i = 0; i != lcids.size(); ++i) {
    drb_id_t drb_id = (drb_id_t)((unsigned)drb_id_t::drb1 + i);
    bearers.add_drb(create_dummy_drb(drb_id, uint_to_lcid(lcids[i])));
  }
  std::optional<lcid_t> found_lcid = bearers.allocate_lcid();
  ASSERT_TRUE(found_lcid.has_value());
  ASSERT_EQ(*found_lcid, lcid_hole);
}
