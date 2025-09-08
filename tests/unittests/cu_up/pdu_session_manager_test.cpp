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

#include "pdu_session_manager_test.h"
#include "cu_up_test_helpers.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace srs_cu_up;

/// PDU session handling tests (creation/deletion)
TEST_F(pdu_session_manager_test, when_valid_pdu_session_setup_item_session_can_be_added)
{
  // no sessions added yet
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // prepare request
  pdu_session_id_t psi    = uint_to_pdu_session_id(1);
  drb_id_t         drb_id = uint_to_drb_id(1);
  qos_flow_id_t    qfi    = uint_to_qos_flow_id(8);

  e1ap_pdu_session_res_to_setup_item pdu_session_setup_item =
      generate_pdu_session_res_to_setup_item(psi, drb_id, qfi, uint_to_five_qi(9));

  // attempt to add session
  pdu_session_setup_result setup_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);

  // check successful outcome
  ASSERT_TRUE(setup_result.success);
  ASSERT_EQ(setup_result.gtp_tunnel.gtp_teid.value(), 1);

  const std::string tp_address_expect = "127.0.0.2"; // address of dummy gateway
  ASSERT_EQ(setup_result.gtp_tunnel.tp_address.to_string(), tp_address_expect);
  ASSERT_EQ(setup_result.drb_setup_results[0].gtp_tunnel.gtp_teid.value(), 1);
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);

  // attempt to remove non-existing session
  pdu_session_mng->remove_pdu_session(uint_to_pdu_session_id(2));

  // check successful outcome (unchanged)
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);

  // attempt to remove existing session
  pdu_session_mng->remove_pdu_session(uint_to_pdu_session_id(1));

  // check successful outcome (unchanged)
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);
}

TEST_F(pdu_session_manager_test, when_pdu_session_with_same_id_is_setup_session_cant_be_added)
{
  // no sessions added yet
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // prepare request
  pdu_session_id_t psi    = uint_to_pdu_session_id(1);
  drb_id_t         drb_id = uint_to_drb_id(1);
  qos_flow_id_t    qfi    = uint_to_qos_flow_id(8);

  e1ap_pdu_session_res_to_setup_item pdu_session_setup_item =
      generate_pdu_session_res_to_setup_item(psi, drb_id, qfi, uint_to_five_qi(9));

  // attempt to add session
  pdu_session_setup_result setup_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);

  // check successful outcome
  ASSERT_TRUE(setup_result.success);
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);

  // attempt to add the same session again
  setup_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);

  // check unsuccessful outcome
  ASSERT_FALSE(setup_result.success);
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
}

TEST_F(pdu_session_manager_test, when_unexisting_pdu_session_is_modified_operation_failed)
{
  // no sessions added yet
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // attempt to modify unexisting PDU session
  e1ap_pdu_session_res_to_modify_item pdu_session_modify_item;
  pdu_session_modify_item.pdu_session_id = uint_to_pdu_session_id(1);

  pdu_session_modification_result modification_result =
      pdu_session_mng->modify_pdu_session(pdu_session_modify_item, false);

  // check successful outcome
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);
}

/// PDU session handling tests (creation/deletion)
TEST_F(pdu_session_manager_test, drb_create_modify_remove)
{
  // no sessions added yet
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // prepare request
  pdu_session_id_t psi    = uint_to_pdu_session_id(1);
  drb_id_t         drb_id = uint_to_drb_id(1);
  qos_flow_id_t    qfi    = uint_to_qos_flow_id(8);

  e1ap_pdu_session_res_to_setup_item pdu_session_setup_item =
      generate_pdu_session_res_to_setup_item(psi, drb_id, qfi, uint_to_five_qi(9));

  // attempt to add session
  pdu_session_setup_result setup_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);

  // check successful outcome
  ASSERT_TRUE(setup_result.success);
  ASSERT_EQ(setup_result.pdu_session_id, psi);
  ASSERT_EQ(setup_result.drb_setup_results.size(), 1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->drb_id, drb_id);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->qos_flow_results.size(), 1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->qos_flow_results.begin()->qos_flow_id, qfi);

  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
  ASSERT_FALSE(gtpu_rx_demux->created_teid_list.empty());
  gtpu_rx_demux->created_teid_list.pop_front();
  ASSERT_TRUE(gtpu_rx_demux->created_teid_list.empty());

  ASSERT_FALSE(f1u_gw->created_ul_teid_list.empty());
  gtpu_teid_t ul_teid = f1u_gw->created_ul_teid_list.front();
  f1u_gw->created_ul_teid_list.pop_front();
  ASSERT_TRUE(f1u_gw->created_ul_teid_list.empty());

  // prepare modification request (to remove bearers)
  e1ap_pdu_session_res_to_modify_item pdu_session_modify_item =
      generate_pdu_session_res_to_modify_item_to_remove_drb(psi, drb_id);

  // Add invalid DRB to remove
  drb_id_t invalid_drb_to_remove = uint_to_drb_id(0x0f);
  pdu_session_modify_item.drb_to_rem_list_ng_ran.push_back(invalid_drb_to_remove);

  // attempt to remove bearers
  pdu_session_modification_result modification_result =
      pdu_session_mng->modify_pdu_session(pdu_session_modify_item, false);

  // check successful outcome
  ASSERT_TRUE(setup_result.success);

  // validate pdu session is not disconnected from GTP-U gateway
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
  ASSERT_TRUE(gtpu_rx_demux->removed_teid_list.empty());

  // validate bearer is disconnected from F1-U gateway
  ASSERT_FALSE(f1u_gw->removed_ul_teid_list.empty());
  ASSERT_EQ(f1u_gw->removed_ul_teid_list.front(), ul_teid);
  f1u_gw->removed_ul_teid_list.pop_front();
  ASSERT_TRUE(f1u_gw->removed_ul_teid_list.empty());
}

/// Create a DRB with only one QFI, but the QFI is already mapped
TEST_F(pdu_session_manager_test, drb_create_with_one_qfi_which_is_already_mapped)
{
  // no sessions added yet
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // prepare request
  pdu_session_id_t psi     = uint_to_pdu_session_id(1);
  drb_id_t         drb_id1 = uint_to_drb_id(1);
  drb_id_t         drb_id2 = uint_to_drb_id(2);
  qos_flow_id_t    qfi     = uint_to_qos_flow_id(8);

  e1ap_pdu_session_res_to_setup_item pdu_session_setup_item =
      generate_pdu_session_res_to_setup_item(psi, drb_id1, qfi, uint_to_five_qi(9));

  // attempt to add session
  pdu_session_setup_result setup_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);

  // check successful outcome
  ASSERT_TRUE(setup_result.success);
  ASSERT_EQ(setup_result.pdu_session_id, psi);
  ASSERT_EQ(setup_result.drb_setup_results.size(), 1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->drb_id, drb_id1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->qos_flow_results.size(), 1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->qos_flow_results.begin()->qos_flow_id, qfi);

  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
  ASSERT_FALSE(gtpu_rx_demux->created_teid_list.empty());
  gtpu_rx_demux->created_teid_list.pop_front();
  ASSERT_TRUE(gtpu_rx_demux->created_teid_list.empty());

  ASSERT_FALSE(f1u_gw->created_ul_teid_list.empty());
  f1u_gw->created_ul_teid_list.pop_front();
  ASSERT_TRUE(f1u_gw->created_ul_teid_list.empty());

  // prepare modification request adding a new DRB and map it to a QFI that is already mapped
  e1ap_pdu_session_res_to_modify_item pdu_session_modify_item =
      generate_pdu_session_res_to_modify_item_to_setup_drb(psi, drb_id2, {qfi}, uint_to_five_qi(9));

  // attempt to perform the modification
  pdu_session_modification_result mod_result = pdu_session_mng->modify_pdu_session(pdu_session_modify_item, false);

  // check the result
  EXPECT_TRUE(mod_result.success);
  ASSERT_EQ(mod_result.drb_setup_results.size(), 1);
  EXPECT_FALSE(mod_result.drb_setup_results[0].success);
  EXPECT_EQ(mod_result.drb_setup_results[0].cause, e1ap_cause_t{e1ap_cause_radio_network_t::unspecified});
  ASSERT_EQ(mod_result.drb_setup_results[0].qos_flow_results.size(), 1);
  EXPECT_FALSE(mod_result.drb_setup_results[0].qos_flow_results[0].success);
  EXPECT_EQ(mod_result.drb_setup_results[0].qos_flow_results[0].cause,
            e1ap_cause_t{e1ap_cause_radio_network_t::multiple_qos_flow_id_instances});

  // validate pdu session is not disconnected from GTP-U gateway
  EXPECT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
  EXPECT_TRUE(gtpu_rx_demux->removed_teid_list.empty());

  // validate the dangling bearer was not created and removed from F1-U gateway
  EXPECT_EQ(f1u_gw->removed_ul_teid_list.size(), 0);
}

/// Create a DRB with only one QFI, but the QFI is already mapped
TEST_F(pdu_session_manager_test, drb_create_with_unknown_five_qi)
{
  // no sessions added yet
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // prepare request
  pdu_session_id_t psi     = uint_to_pdu_session_id(1);
  drb_id_t         drb_id1 = uint_to_drb_id(1);
  qos_flow_id_t    qfi     = uint_to_qos_flow_id(8);

  e1ap_pdu_session_res_to_setup_item pdu_session_setup_item =
      generate_pdu_session_res_to_setup_item(psi, drb_id1, qfi, uint_to_five_qi(8));

  // attempt to add session adding a new DRB and map it to a 5QI that is unknown
  pdu_session_setup_result setup_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);

  // check successful outcome
  ASSERT_TRUE(setup_result.success);
  ASSERT_EQ(setup_result.pdu_session_id, psi);
  ASSERT_EQ(setup_result.drb_setup_results.size(), 1);

  EXPECT_FALSE(setup_result.drb_setup_results[0].success);
  EXPECT_EQ(setup_result.drb_setup_results[0].cause, e1ap_cause_t{e1ap_cause_radio_network_t::not_supported_5qi_value});
  ASSERT_EQ(setup_result.drb_setup_results[0].qos_flow_results.size(), 0);
}

/// Create a DRB with two QFIs, of which one QFI is already mapped
TEST_F(pdu_session_manager_test, drb_create_with_two_qfi_of_which_one_is_already_mapped)
{
  // no sessions added yet
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // prepare request
  pdu_session_id_t psi     = uint_to_pdu_session_id(1);
  drb_id_t         drb_id1 = uint_to_drb_id(1);
  drb_id_t         drb_id2 = uint_to_drb_id(2);
  qos_flow_id_t    qfi1    = uint_to_qos_flow_id(8);
  qos_flow_id_t    qfi2    = uint_to_qos_flow_id(9);

  e1ap_pdu_session_res_to_setup_item pdu_session_setup_item =
      generate_pdu_session_res_to_setup_item(psi, drb_id1, qfi1, uint_to_five_qi(9));

  // attempt to add session
  pdu_session_setup_result setup_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);

  // check successful outcome
  ASSERT_TRUE(setup_result.success);
  ASSERT_EQ(setup_result.pdu_session_id, psi);
  ASSERT_EQ(setup_result.drb_setup_results.size(), 1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->drb_id, drb_id1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->qos_flow_results.size(), 1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->qos_flow_results.begin()->qos_flow_id, qfi1);

  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
  ASSERT_FALSE(gtpu_rx_demux->created_teid_list.empty());
  gtpu_rx_demux->created_teid_list.pop_front();
  ASSERT_TRUE(gtpu_rx_demux->created_teid_list.empty());

  ASSERT_FALSE(f1u_gw->created_ul_teid_list.empty());
  f1u_gw->created_ul_teid_list.pop_front();
  ASSERT_TRUE(f1u_gw->created_ul_teid_list.empty());

  // prepare modification request adding a new DRB and map it to a QFI that is already mapped
  e1ap_pdu_session_res_to_modify_item pdu_session_modify_item =
      generate_pdu_session_res_to_modify_item_to_setup_drb(psi, drb_id2, {qfi1, qfi2}, uint_to_five_qi(9));

  // attempt to perform the modification
  pdu_session_modification_result mod_result = pdu_session_mng->modify_pdu_session(pdu_session_modify_item, false);

  // check the result
  EXPECT_TRUE(mod_result.success);
  ASSERT_EQ(mod_result.drb_setup_results.size(), 1);
  EXPECT_TRUE(mod_result.drb_setup_results[0].success); // success, since at least one QFI mapping was valid
  EXPECT_EQ(mod_result.drb_setup_results[0].cause, e1ap_cause_t{e1ap_cause_radio_network_t::unspecified});
  ASSERT_EQ(mod_result.drb_setup_results[0].qos_flow_results.size(), 2);
  EXPECT_FALSE(mod_result.drb_setup_results[0].qos_flow_results[0].success); // the first was invalid
  EXPECT_EQ(mod_result.drb_setup_results[0].qos_flow_results[0].cause,
            e1ap_cause_t{e1ap_cause_radio_network_t::multiple_qos_flow_id_instances});
  EXPECT_TRUE(mod_result.drb_setup_results[0].qos_flow_results[1].success); // the second was valid
  EXPECT_EQ(mod_result.drb_setup_results[0].qos_flow_results[1].cause,
            e1ap_cause_t{e1ap_cause_radio_network_t::unspecified});

  // validate pdu session is not disconnected from GTP-U gateway
  EXPECT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
  EXPECT_TRUE(gtpu_rx_demux->removed_teid_list.empty());

  // validate the dangling bearer was not removed from F1-U gateway
  EXPECT_TRUE(f1u_gw->removed_ul_teid_list.empty());
}

TEST_F(pdu_session_manager_test, dtor_rm_all_sessions_and_bearers)
{
  // no sessions added yet
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // prepare request
  pdu_session_id_t psi    = uint_to_pdu_session_id(1);
  drb_id_t         drb_id = uint_to_drb_id(1);
  qos_flow_id_t    qfi    = uint_to_qos_flow_id(8);

  e1ap_pdu_session_res_to_setup_item pdu_session_setup_item =
      generate_pdu_session_res_to_setup_item(psi, drb_id, qfi, uint_to_five_qi(9));

  // attempt to add session
  pdu_session_setup_result setup_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);

  // check successful outcome
  ASSERT_TRUE(setup_result.success);
  ASSERT_EQ(setup_result.pdu_session_id, psi);
  ASSERT_EQ(setup_result.drb_setup_results.size(), 1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->drb_id, drb_id);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->qos_flow_results.size(), 1);
  ASSERT_EQ(setup_result.drb_setup_results.begin()->qos_flow_results.begin()->qos_flow_id, qfi);

  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
  ASSERT_FALSE(gtpu_rx_demux->created_teid_list.empty());
  gtpu_teid_t teid = gtpu_rx_demux->created_teid_list.front();
  gtpu_rx_demux->created_teid_list.pop_front();
  ASSERT_TRUE(gtpu_rx_demux->created_teid_list.empty());

  ASSERT_FALSE(f1u_gw->created_ul_teid_list.empty());
  gtpu_teid_t ul_teid = f1u_gw->created_ul_teid_list.front();
  f1u_gw->created_ul_teid_list.pop_front();
  ASSERT_TRUE(f1u_gw->created_ul_teid_list.empty());

  // delete pdu_session_mng, all remaining sessions and bearers shall be removed and detached from all gateways
  pdu_session_mng.reset();

  // validate pdu session is disconnected from GTP-U gateway
  ASSERT_FALSE(gtpu_rx_demux->removed_teid_list.empty());
  ASSERT_EQ(gtpu_rx_demux->removed_teid_list.front(), teid);
  gtpu_rx_demux->removed_teid_list.pop_front();
  ASSERT_TRUE(gtpu_rx_demux->removed_teid_list.empty());

  // validate bearer is disconnected from F1-U gateway
  ASSERT_FALSE(f1u_gw->removed_ul_teid_list.empty());
  ASSERT_EQ(f1u_gw->removed_ul_teid_list.front(), ul_teid);
  f1u_gw->removed_ul_teid_list.pop_front();
  ASSERT_TRUE(f1u_gw->removed_ul_teid_list.empty());
}

/// PDU session handling tests (creation/deletion)
TEST_F(pdu_session_manager_test, when_new_ul_info_is_requested_f1u_is_disconnected)
{
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 0);

  // prepare request
  pdu_session_id_t psi    = uint_to_pdu_session_id(1);
  drb_id_t         drb_id = uint_to_drb_id(1);
  qos_flow_id_t    qfi    = uint_to_qos_flow_id(8);

  e1ap_pdu_session_res_to_setup_item pdu_session_setup_item =
      generate_pdu_session_res_to_setup_item(psi, drb_id, qfi, uint_to_five_qi(9));

  pdu_session_setup_result set_result = pdu_session_mng->setup_pdu_session(pdu_session_setup_item);
  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
  drb_setup_result drb_setup_res = set_result.drb_setup_results[0];
  ASSERT_EQ(drb_setup_res.gtp_tunnel.gtp_teid, 0x1);

  // prepare modification request (request new UL TNL info)
  e1ap_pdu_session_res_to_modify_item pdu_session_modify_item =
      generate_pdu_session_res_to_modify_item_to_modify_drb(psi, drb_id);

  pdu_session_modification_result mod_result  = pdu_session_mng->modify_pdu_session(pdu_session_modify_item, true);
  drb_setup_result                drb_mod_res = mod_result.drb_modification_results[0];
  ASSERT_EQ(drb_mod_res.gtp_tunnel.gtp_teid, 0x2);

  ASSERT_EQ(pdu_session_mng->get_nof_pdu_sessions(), 1);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
