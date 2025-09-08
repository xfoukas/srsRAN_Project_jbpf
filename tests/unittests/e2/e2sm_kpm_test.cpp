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

#include "common/e2ap_asn1_packer.h"
#include "lib/e2/common/e2ap_asn1_utils.h"
#include "lib/pcap/dlt_pcap_impl.h"
#include "tests/unittests/e2/common/e2_test_helpers.h"
#include "srsran/support/async/async_test_utils.h"
#include "srsran/support/executors/task_worker.h"
#include "srsran/support/test_utils.h"
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

using namespace srsran;
using namespace asn1::e2sm;

// Helper global variables to pass pcap_writer to all tests.
bool      g_enable_pcap = false;
dlt_pcap* g_pcap        = nullptr;

class e2_entity_test_with_pcap : public e2_test_base_with_pcap
{
protected:
  dlt_pcap* external_pcap_writer;

  void SetUp() override
  {
    external_pcap_writer = GetParam();

    srslog::fetch_basic_logger("TEST").set_level(srslog::basic_levels::debug);
    srslog::init();

    cfg                  = config_helpers::make_default_e2ap_config();
    cfg.e2sm_kpm_enabled = true;

    gw                       = std::make_unique<dummy_sctp_association_sdu_notifier>();
    pcap                     = std::make_unique<dummy_e2ap_pcap>();
    e2_client                = std::make_unique<dummy_e2_connection_client>();
    du_metrics               = std::make_unique<dummy_e2_du_metrics>();
    f1ap_ue_id_mapper        = std::make_unique<dummy_f1ap_ue_id_translator>();
    factory                  = timer_factory{timers, task_worker};
    du_rc_param_configurator = std::make_unique<dummy_du_configurator>();
    e2agent                  = create_e2_du_agent(
        cfg, *e2_client, &(*du_metrics), &(*f1ap_ue_id_mapper), &(*du_rc_param_configurator), factory, task_worker);
    if (external_pcap_writer) {
      packer = std::make_unique<e2ap_asn1_packer>(*gw, e2agent->get_e2_interface(), *external_pcap_writer);
    } else {
      packer = std::make_unique<e2ap_asn1_packer>(*gw, e2agent->get_e2_interface(), *pcap);
    }
  }

  void TearDown() override
  {
    // flush logger after each test
    srslog::flush();
    pcap->close();
  }
};

class e2sm_kpm_indication : public e2_test_base_with_pcap
{
protected:
  dlt_pcap* external_pcap_writer;

  void SetUp() override
  {
    external_pcap_writer = GetParam();

    srslog::fetch_basic_logger("TEST").set_level(srslog::basic_levels::debug);
    srslog::init();

    cfg                  = config_helpers::make_default_e2ap_config();
    cfg.e2sm_kpm_enabled = true;

    du_metrics       = std::make_unique<dummy_e2_du_metrics>();
    du_meas_provider = std::make_unique<dummy_e2sm_kpm_du_meas_provider>();
    e2sm_kpm_packer  = std::make_unique<e2sm_kpm_asn1_packer>(*du_meas_provider);
    e2sm_kpm_iface   = std::make_unique<e2sm_kpm_impl>(test_logger, *e2sm_kpm_packer, *du_meas_provider);
    gw               = std::make_unique<dummy_sctp_association_sdu_notifier>();
    pcap             = std::make_unique<dummy_e2ap_pcap>();
    e2               = std::make_unique<dummy_e2_interface>();
    if (external_pcap_writer) {
      packer = std::make_unique<e2ap_asn1_packer>(*gw, *e2, *external_pcap_writer);
    } else {
      packer = std::make_unique<e2ap_asn1_packer>(*gw, *e2, *pcap);
    }
  }

  void TearDown() override
  {
    // Flush logger after each test.
    srslog::flush();
    pcap->close();
  }
};

void get_presence_starting_with_cond_satisfied(const std::vector<uint32_t>& presence,
                                               const std::vector<uint32_t>& cond_satisfied,
                                               std::vector<uint32_t>&       cond_presence)
{
  cond_presence = presence;
  if (cond_satisfied.size() == 0) {
    return;
  }
  for (unsigned i = 0; i < cond_satisfied.size(); i++) {
    if (cond_satisfied[i]) {
      break;
    } else {
      cond_presence[i] = 0;
    }
  }
}

void get_presence_starting_with_cond_satisfied(const std::vector<std::vector<uint32_t>>& presence,
                                               const std::vector<std::vector<uint32_t>>& cond_satisfied,
                                               std::vector<std::vector<uint32_t>>&       cond_presence)
{
  cond_presence = presence;
  if (cond_satisfied.size() == 0) {
    return;
  }
  uint32_t nof_time_slots = cond_satisfied.size();
  uint32_t nof_ues        = cond_satisfied[0].size();
  for (unsigned ue_idx = 0; ue_idx < nof_ues; ue_idx++) {
    for (unsigned i = 0; i < nof_time_slots; i++) {
      if (cond_satisfied[i][ue_idx]) {
        break;
      } else {
        cond_presence[i][ue_idx] = 0;
      }
    }
  }
}

std::vector<uint32_t> get_reported_ues(const std::vector<std::vector<uint32_t>>& cond_presence)
{
  uint32_t nof_time_slots = cond_presence.size();
  uint32_t nof_ues        = cond_presence[0].size();
  // Get indexes of UEs that report values at least once.
  std::vector<uint32_t> reported_ues;
  for (unsigned ue_idx = 0; ue_idx < nof_ues; ue_idx++) {
    for (unsigned i = 0; i < nof_time_slots; i++) {
      if (cond_presence[i][ue_idx]) {
        reported_ues.push_back(ue_idx);
        break;
      }
    }
  }
  return reported_ues;
}

// E2 Setup Request is needed for Wireshark to correctly decode the subsequent Subscription Requests
TEST_P(e2_entity_test_with_pcap, e2sm_kpm_generates_ran_func_desc)
{
  // We need this test to generate E2 Setup Request, so Wireshark can decode the following RIC indication messages.
  test_logger.info("Launch e2 setup request procedure with task worker...");
  e2agent->start();

  // Save E2 Setup Request
  packer->handle_message(e2_client->last_tx_e2_pdu);

  // Need to send setup response, so the transaction can be completed.
  unsigned   transaction_id    = get_transaction_id(e2_client->last_tx_e2_pdu.pdu).value();
  e2_message e2_setup_response = generate_e2_setup_response(transaction_id);
  e2_setup_response.pdu.successful_outcome()
      .value.e2setup_resp()
      ->ran_functions_accepted[0]
      ->ran_function_id_item()
      .ran_function_id = e2sm_kpm_asn1_packer::ran_func_id;
  test_logger.info("Injecting E2SetupResponse");
  e2agent->get_e2_interface().handle_message(e2_setup_response);
  e2agent->stop();
}

TEST_P(e2sm_kpm_indication, e2sm_kpm_generates_ric_indication_style1)
{
  // Measurement values in 5 time slot.
  std::vector<float>    meas_real_values = {0.15625, 1.0, 4.0, 4.00001, 1234.1234};
  std::vector<uint32_t> meas_int_values  = {1, 2, 3, 4, 5};
  uint32_t              nof_meas_data    = meas_real_values.size();
  uint32_t              nof_metrics      = 2;
  uint32_t              nof_records      = nof_metrics;

  // Define E2SM_KPM action format 1.
  e2sm_kpm_action_definition_s action_def;
  action_def.ric_style_type = 1;
  e2sm_kpm_action_definition_format1_s& action_def_f1 =
      action_def.action_definition_formats.set_action_definition_format1();
  action_def_f1.cell_global_id_present = false;
  action_def_f1.granul_period          = 100;

  meas_info_item_s meas_info_item;
  meas_info_item.meas_type.set_meas_name().from_string("test"); // Dummy metric not supported.
  label_info_item_s label_info_item{};
  label_info_item.meas_label.no_label_present = true;
  label_info_item.meas_label.no_label         = meas_label_s::no_label_opts::true_value;
  meas_info_item.label_info_list.push_back(label_info_item);
  action_def_f1.meas_info_list.push_back(meas_info_item);

  asn1::e2ap::ric_action_to_be_setup_item_s ric_action = generate_e2sm_kpm_ric_action(action_def);
  ASSERT_FALSE(e2sm_kpm_iface->action_supported(ric_action));

  action_def_f1.meas_info_list[0].meas_type.set_meas_name().from_string("DRB.UEThpUl"); // Change to a valid metric.

  action_def_f1.meas_info_list.push_back(meas_info_item); // Add a second metric.
  action_def_f1.meas_info_list[1].meas_type.set_meas_name().from_string("DRB.RlcSduDelayDl");
  ric_action = generate_e2sm_kpm_ric_action(action_def);

  if (g_enable_pcap) {
    // Save E2 Subscription Request.
    e2_message e2_subscript_req = generate_e2sm_kpm_subscription_request(ric_action);
    packer->handle_message(e2_subscript_req);
  }

  ASSERT_TRUE(e2sm_kpm_iface->action_supported(ric_action));
  auto report_service = e2sm_kpm_iface->get_e2sm_report_service(ric_action.ric_action_definition);

  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());
  // As E2 is always present and provides valid metrics, there is no need to check if the indication is ready when
  // filled with only no_values.

  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements.
    du_meas_provider->push_measurements_int({1}, {1}, {meas_int_values[i]});
    du_meas_provider->push_measurements_float({1}, {1}, {meas_real_values[i]});
    // Trigger measurement collection.
    report_service->collect_measurements();
  }

  TESTASSERT_EQ(true, report_service->is_ind_msg_ready());
  // Get RIC indication msg content.
  byte_buffer ind_hdr_bytes = report_service->get_indication_header();
  byte_buffer ind_msg_bytes = report_service->get_indication_message();

  // Decode RIC Indication and check the content.
  e2sm_kpm_ind_msg_s ric_ind_msg;
  asn1::cbit_ref     ric_ind_bref(ind_msg_bytes);
  if (ric_ind_msg.unpack(ric_ind_bref) != asn1::SRSASN_SUCCESS) {
    test_logger.debug("e2sm_kpm: RIC indication msg could not be unpacked");
    return;
  }

  TESTASSERT_EQ(nof_meas_data, ric_ind_msg.ind_msg_formats.ind_msg_format1().meas_data.size());
  for (unsigned i = 0; i < nof_meas_data; ++i) {
    TESTASSERT_EQ(nof_records, ric_ind_msg.ind_msg_formats.ind_msg_format1().meas_data[i].meas_record.size());
    TESTASSERT_EQ(meas_int_values[i],
                  ric_ind_msg.ind_msg_formats.ind_msg_format1().meas_data[i].meas_record[0].integer());
    TESTASSERT_EQ(meas_real_values[i],
                  ric_ind_msg.ind_msg_formats.ind_msg_format1().meas_data[i].meas_record[1].real().value);
  }

  if (g_enable_pcap) {
    e2_message e2_msg = generate_e2_ind_msg(ind_hdr_bytes, ind_msg_bytes);
    packer->handle_message(e2_msg);
  }
}

TEST_P(e2sm_kpm_indication, e2sm_kpm_generates_ric_indication_style2)
{
  std::vector<uint32_t> ue_ids = {31};
  du_meas_provider->set_ue_ids(ue_ids);

  // Presence and measurement values in 10 time slots.
  std::vector<uint32_t> presence      = {1, 1, 0, 1, 1, 1, 0, 0, 1, 0};
  std::vector<uint32_t> meas_values   = {1, 2, 0, 4, 5, 6, 0, 0, 9, 0};
  uint32_t              nof_meas_data = presence.size();
  uint32_t              nof_ues       = ue_ids.size();
  uint32_t              nof_metrics   = 1;
  uint32_t              nof_records   = nof_metrics * nof_ues;

  // Measurement records are no_value before UE is present and satisfies conditions.
  std::vector<uint32_t> cond_presence;
  get_presence_starting_with_cond_satisfied(presence, {}, cond_presence);

  // Define E2SM_KPM action format 2.
  e2sm_kpm_action_definition_s action_def;
  action_def.ric_style_type = 2;
  e2sm_kpm_action_definition_format2_s& action_def_f2 =
      action_def.action_definition_formats.set_action_definition_format2();

  ue_id_c& ueid           = action_def_f2.ue_id;
  ueid.set_gnb_du_ue_id() = generate_ueid_gnb_du(ue_ids[0]);

  action_def_f2.subscript_info.cell_global_id_present = false;
  action_def_f2.subscript_info.granul_period          = 100;

  meas_info_item_s meas_info_item;
  meas_info_item.meas_type.set_meas_name().from_string("test"); // Dummy metric not supported
  label_info_item_s label_info_item{};
  label_info_item.meas_label.no_label_present = true;
  label_info_item.meas_label.no_label         = meas_label_s::no_label_opts::true_value;
  meas_info_item.label_info_list.push_back(label_info_item);
  action_def_f2.subscript_info.meas_info_list.push_back(meas_info_item);

  asn1::e2ap::ric_action_to_be_setup_item_s ric_action = generate_e2sm_kpm_ric_action(action_def);
  ASSERT_FALSE(e2sm_kpm_iface->action_supported(ric_action));

  action_def_f2.subscript_info.meas_info_list[0].meas_type.set_meas_name().from_string("DRB.UEThpDl");
  ric_action = generate_e2sm_kpm_ric_action(action_def);

  if (g_enable_pcap) {
    // Save E2 Subscription Request.
    e2_message e2_subscript_req = generate_e2sm_kpm_subscription_request(ric_action);
    packer->handle_message(e2_subscript_req);
  }

  ASSERT_TRUE(e2sm_kpm_iface->action_supported(ric_action));
  auto report_service = e2sm_kpm_iface->get_e2sm_report_service(ric_action.ric_action_definition);

  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());
  // Fill only with no_values and check if indication is ready.
  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements.
    du_meas_provider->push_measurements_int({0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0});
    // Trigger measurement collection.
    report_service->collect_measurements();
  }
  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());

  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements.
    du_meas_provider->push_measurements_int({presence[i]}, {1}, {meas_values[i]});
    // Trigger measurement collection.
    report_service->collect_measurements();
  }

  TESTASSERT_EQ(true, report_service->is_ind_msg_ready());
  // Get RIC indication msg content.
  byte_buffer ind_hdr_bytes = report_service->get_indication_header();
  byte_buffer ind_msg_bytes = report_service->get_indication_message();

  // Decode RIC Indication and check the content.
  e2sm_kpm_ind_msg_s ric_ind_msg;
  asn1::cbit_ref     ric_ind_bref(ind_msg_bytes);
  if (ric_ind_msg.unpack(ric_ind_bref) != asn1::SRSASN_SUCCESS) {
    test_logger.debug("e2sm_kpm: RIC indication msg could not be unpacked");
    return;
  }

  TESTASSERT_EQ(nof_meas_data, ric_ind_msg.ind_msg_formats.ind_msg_format1().meas_data.size());
  for (unsigned i = 0; i < nof_meas_data; ++i) {
    TESTASSERT_EQ(nof_records, ric_ind_msg.ind_msg_formats.ind_msg_format1().meas_data[i].meas_record.size());
    if (presence[i]) {
      TESTASSERT_EQ(meas_values[i],
                    ric_ind_msg.ind_msg_formats.ind_msg_format1().meas_data[i].meas_record[0].integer());
    } else {
      TESTASSERT_EQ(
          fmt::underlying(meas_record_item_c::types_opts::no_value),
          fmt::underlying(ric_ind_msg.ind_msg_formats.ind_msg_format1().meas_data[i].meas_record[0].type().value));
    }
  }

  if (g_enable_pcap) {
    e2_message e2_msg = generate_e2_ind_msg(ind_hdr_bytes, ind_msg_bytes);
    packer->handle_message(e2_msg);
  }
}

TEST_P(e2sm_kpm_indication, e2sm_kpm_generates_ric_indication_style3)
{
  std::vector<uint32_t> ue_ids = {32, 129, 2, 15, 8};
  du_meas_provider->set_ue_ids(ue_ids);
  // Presence, cond_satisfied and measurement values in 5 time slots for 5 UEs.
  std::vector<std::vector<uint32_t>> presence = {
      {1, 1, 1, 0, 1},
      {1, 1, 1, 0, 0},
      {1, 1, 1, 0, 0},
      {0, 1, 1, 0, 1},
      {1, 1, 1, 0, 1},
  };
  std::vector<std::vector<uint32_t>> cond_satisfied = {
      {0, 1, 0, 0, 1},
      {0, 1, 0, 0, 0},
      {1, 0, 0, 0, 0},
      {0, 0, 0, 0, 1},
      {1, 1, 0, 0, 1},
  };
  std::vector<std::vector<uint32_t>> meas_values = {
      {1, 2, 3, 0, 5},
      {11, 12, 13, 0, 15},
      {21, 22, 23, 0, 25},
      {31, 32, 33, 0, 35},
      {41, 42, 43, 0, 45},
  };
  uint32_t nof_metrics   = 1;
  uint32_t nof_meas_data = presence.size();
  // Measurement records are no_value before UE is present and satisfies conditions.
  std::vector<std::vector<uint32_t>> cond_presence;
  get_presence_starting_with_cond_satisfied(presence, cond_satisfied, cond_presence);
  std::vector<uint32_t> reported_ues     = get_reported_ues(cond_presence);
  uint32_t              nof_reported_ues = reported_ues.size();
  uint32_t              nof_records      = nof_metrics * nof_reported_ues;

  // Define E2SM_KPM action format 3.
  e2sm_kpm_action_definition_s action_def;
  action_def.ric_style_type = 3;
  e2sm_kpm_action_definition_format3_s& action_def_f3 =
      action_def.action_definition_formats.set_action_definition_format3();

  meas_cond_item_s meas_cond_item;
  meas_cond_item.meas_type.set_meas_name().from_string("test"); // Dummy metric not supported.

  // Report UEThpDl for all UEs with RSRP > -110 and RSRP < -50.
  // Add conditions, order is important. Labels has to be first.
  matching_cond_item_s matching_cond_item1;
  meas_label_s         meas_label1{};
  meas_label1.five_qi_present                               = true;
  meas_label1.five_qi                                       = 1;
  matching_cond_item1.lc_or_present                         = true; // If false use OR, if true then use AND.
  matching_cond_item1.lc_or                                 = lc_or_opts::true_value;
  matching_cond_item1.matching_cond_choice.set_meas_label() = meas_label1;
  meas_cond_item.matching_cond.push_back(matching_cond_item1);

  matching_cond_item_s matching_cond_item2;
  test_cond_info_s     test_cond_info1;
  test_cond_info1.test_type.set_ul_r_srp().value = test_cond_type_c::ul_r_srp_opts::true_value;
  test_cond_info1.test_expr_present              = true;
  test_cond_info1.test_expr                      = test_cond_expression_opts::greaterthan;
  // TODO: seems that asn1 does not suppport negative numbers.
  test_cond_info1.test_value_present                            = true;
  test_cond_info1.test_value.set_value_int()                    = 50;
  matching_cond_item2.lc_or_present                             = true; // If false use OR, if true then use AND.
  matching_cond_item2.lc_or                                     = lc_or_opts::true_value;
  matching_cond_item2.matching_cond_choice.set_test_cond_info() = test_cond_info1;
  meas_cond_item.matching_cond.push_back(matching_cond_item2);

  matching_cond_item_s matching_cond_item3;
  test_cond_info_s     test_cond_info2;
  test_cond_info2.test_type.set_ul_r_srp().value = test_cond_type_c::ul_r_srp_opts::true_value;
  test_cond_info2.test_expr_present              = true;
  test_cond_info2.test_value_present             = true;
  test_cond_info2.test_expr                      = test_cond_expression_opts::lessthan;
  // TODO: seems that asn1 does not suppport negative numbers.
  test_cond_info2.test_value.set_value_int()                    = 110;
  matching_cond_item3.lc_or_present                             = false; // If false use OR, if true then use AND.
  matching_cond_item3.matching_cond_choice.set_test_cond_info() = test_cond_info2;
  meas_cond_item.matching_cond.push_back(matching_cond_item3);

  // Put all conditions together for a single metric.
  action_def_f3.meas_cond_list.push_back(meas_cond_item);
  action_def_f3.cell_global_id_present = false;
  action_def_f3.granul_period          = 100;

  asn1::e2ap::ric_action_to_be_setup_item_s ric_action = generate_e2sm_kpm_ric_action(action_def);
  ASSERT_FALSE(e2sm_kpm_iface->action_supported(ric_action));

  action_def_f3.meas_cond_list[0].meas_type.set_meas_name().from_string("DRB.UEThpDl"); // Change to a valid metric.
  ric_action = generate_e2sm_kpm_ric_action(action_def);

  if (g_enable_pcap) {
    // Save E2 Subscription Request.
    e2_message e2_subscript_req = generate_e2sm_kpm_subscription_request(ric_action);
    packer->handle_message(e2_subscript_req);
  }

  ASSERT_TRUE(e2sm_kpm_iface->action_supported(ric_action));
  auto report_service = e2sm_kpm_iface->get_e2sm_report_service(ric_action.ric_action_definition);

  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());
  // Fill only with no_values and check if indication is ready.
  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements.
    du_meas_provider->push_measurements_int({0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0});
    // Trigger measurement collection.
    report_service->collect_measurements();
  }
  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());

  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements
    du_meas_provider->push_measurements_int(presence[i], cond_satisfied[i], meas_values[i]);
    // Trigger measurement collection.
    report_service->collect_measurements();
  }

  TESTASSERT_EQ(true, report_service->is_ind_msg_ready());
  // Get RIC indication msg content.
  byte_buffer ind_hdr_bytes = report_service->get_indication_header();
  byte_buffer ind_msg_bytes = report_service->get_indication_message();

  // Decode RIC Indication and check the content.
  e2sm_kpm_ind_msg_s ric_ind_msg;
  asn1::cbit_ref     ric_ind_bref(ind_msg_bytes);
  if (ric_ind_msg.unpack(ric_ind_bref) != asn1::SRSASN_SUCCESS) {
    test_logger.debug("e2sm_kpm: RIC indication msg could not be unpacked");
    return;
  }

  TESTASSERT_EQ(nof_meas_data, ric_ind_msg.ind_msg_formats.ind_msg_format2().meas_data.size());
  for (unsigned i = 0; i < nof_meas_data; ++i) {
    TESTASSERT_EQ(nof_records, ric_ind_msg.ind_msg_formats.ind_msg_format2().meas_data[i].meas_record.size());
    for (unsigned j = 0; j < nof_reported_ues; ++j) {
      uint32_t ue_id = ric_ind_msg.ind_msg_formats.ind_msg_format2()
                           .meas_cond_ue_id_list[0]
                           .matching_ue_id_list[j]
                           .ue_id.gnb_du_ue_id()
                           .gnb_cu_ue_f1ap_id;
      std::vector<uint32_t>::iterator it     = std::find(ue_ids.begin(), ue_ids.end(), ue_id);
      uint32_t                        ue_idx = std::distance(ue_ids.begin(), it);
      if (cond_presence[i][ue_idx]) {
        TESTASSERT_EQ(meas_values[i][ue_idx],
                      ric_ind_msg.ind_msg_formats.ind_msg_format2().meas_data[i].meas_record[j].integer());
      } else {
        TESTASSERT_EQ(
            fmt::underlying(meas_record_item_c::types_opts::no_value),
            fmt::underlying(ric_ind_msg.ind_msg_formats.ind_msg_format2().meas_data[i].meas_record[j].type().value));
      }
    }
  }

  if (g_enable_pcap) {
    e2_message e2_msg = generate_e2_ind_msg(ind_hdr_bytes, ind_msg_bytes);
    packer->handle_message(e2_msg);
  }
}

TEST_P(e2sm_kpm_indication, e2sm_kpm_generates_ric_indication_style4)
{
  std::vector<uint32_t> ue_ids = {23, 3, 14, 2, 9};
  du_meas_provider->set_ue_ids(ue_ids);
  // Presence, cond_satisfied and measurement values in 5 time slots for 5 UEs.
  std::vector<std::vector<uint32_t>> presence = {
      {1, 1, 1, 0, 1},
      {1, 1, 1, 0, 0},
      {1, 1, 1, 0, 0},
      {0, 1, 1, 0, 1},
      {1, 1, 1, 0, 1},
  };
  std::vector<std::vector<uint32_t>> cond_satisfied = {
      {0, 1, 0, 0, 1},
      {0, 1, 0, 0, 0},
      {1, 0, 0, 0, 0},
      {0, 0, 0, 0, 1},
      {1, 1, 0, 0, 1},
  };
  std::vector<std::vector<uint32_t>> meas_values = {
      {1, 2, 3, 0, 5},
      {11, 12, 13, 0, 15},
      {21, 22, 23, 0, 25},
      {31, 32, 33, 0, 35},
      {41, 42, 43, 0, 45},
  };
  uint32_t nof_meas_data = presence.size();
  uint32_t nof_metrics   = 1;
  // Measurement records are no_value before UE is present and satisfies conditions.
  std::vector<std::vector<uint32_t>> cond_presence;
  get_presence_starting_with_cond_satisfied(presence, cond_satisfied, cond_presence);
  std::vector<uint32_t> reported_ues     = get_reported_ues(cond_presence);
  uint32_t              nof_reported_ues = reported_ues.size();
  uint32_t              nof_records      = nof_metrics;

  // Define E2SM_KPM action format 4.
  e2sm_kpm_action_definition_s action_def;
  action_def.ric_style_type = 4;
  e2sm_kpm_action_definition_format4_s& action_def_f4 =
      action_def.action_definition_formats.set_action_definition_format4();

  // Report UEThpDl for all UEs with rsrp > -110 and rsrp < -50.
  matching_ue_cond_per_sub_item_s matching_ue_cond_item1;
  matching_ue_cond_item1.test_cond_info.test_type.set_ul_r_srp().value = test_cond_type_c::ul_r_srp_opts::true_value;
  matching_ue_cond_item1.test_cond_info.test_expr_present              = true;
  matching_ue_cond_item1.test_cond_info.test_value_present             = true;
  matching_ue_cond_item1.test_cond_info.test_expr                      = test_cond_expression_opts::greaterthan;
  // TODO: seems that asn1 does not suppport negative numbers.
  matching_ue_cond_item1.test_cond_info.test_value.set_value_int() = 50;
  matching_ue_cond_item1.lc_or_present                             = false; // if false use OR, if true then use AND
  action_def_f4.matching_ue_cond_list.push_back(matching_ue_cond_item1);

  matching_ue_cond_per_sub_item_s matching_ue_cond_item2;
  matching_ue_cond_item2.test_cond_info.test_type.set_ul_r_srp().value = test_cond_type_c::ul_r_srp_opts::true_value;
  matching_ue_cond_item2.test_cond_info.test_expr_present              = true;
  matching_ue_cond_item2.test_cond_info.test_value_present             = true;
  matching_ue_cond_item2.test_cond_info.test_expr                      = test_cond_expression_opts::lessthan;
  // TODO: seems that asn1 does not suppport negative numbers.
  matching_ue_cond_item2.test_cond_info.test_value.set_value_int() = 110;
  action_def_f4.matching_ue_cond_list.push_back(matching_ue_cond_item2);

  e2sm_kpm_action_definition_format1_s& subscription_info = action_def_f4.sub_info;
  subscription_info.cell_global_id_present                = false;
  subscription_info.granul_period                         = 100;

  meas_info_item_s meas_info_item;
  meas_info_item.meas_type.set_meas_name().from_string("test"); // dummy metric not supported
  label_info_item_s label_info_item{};
  label_info_item.meas_label.no_label_present = true;
  label_info_item.meas_label.no_label         = meas_label_s::no_label_opts::true_value;
  meas_info_item.label_info_list.push_back(label_info_item);
  subscription_info.meas_info_list.push_back(meas_info_item);

  asn1::e2ap::ric_action_to_be_setup_item_s ric_action = generate_e2sm_kpm_ric_action(action_def);
  ASSERT_FALSE(e2sm_kpm_iface->action_supported(ric_action));

  subscription_info.meas_info_list[0].meas_type.set_meas_name().from_string("DRB.UEThpDl"); // change to a valid metric
  ric_action = generate_e2sm_kpm_ric_action(action_def);

  if (g_enable_pcap) {
    // Save E2 Subscription Request.
    e2_message e2_subscript_req = generate_e2sm_kpm_subscription_request(ric_action);
    packer->handle_message(e2_subscript_req);
  }

  ASSERT_TRUE(e2sm_kpm_iface->action_supported(ric_action));
  auto report_service = e2sm_kpm_iface->get_e2sm_report_service(ric_action.ric_action_definition);

  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());
  // Fill only with no_values and check if indication is ready.
  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements.
    du_meas_provider->push_measurements_int({0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0});
    // Trigger measurement collection.
    report_service->collect_measurements();
  }
  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());

  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements.
    du_meas_provider->push_measurements_int(presence[i], cond_satisfied[i], meas_values[i]);
    // Trigger measurement collection.
    report_service->collect_measurements();
  }

  TESTASSERT_EQ(true, report_service->is_ind_msg_ready());
  // Get RIC indication msg content.
  byte_buffer ind_hdr_bytes = report_service->get_indication_header();
  byte_buffer ind_msg_bytes = report_service->get_indication_message();

  // Decode RIC Indication and check the content.
  e2sm_kpm_ind_msg_s ric_ind_msg;
  asn1::cbit_ref     ric_ind_bref(ind_msg_bytes);
  if (ric_ind_msg.unpack(ric_ind_bref) != asn1::SRSASN_SUCCESS) {
    test_logger.debug("e2sm_kpm: RIC indication msg could not be unpacked");
    return;
  }

  TESTASSERT_EQ(nof_reported_ues, ric_ind_msg.ind_msg_formats.ind_msg_format3().ue_meas_report_list.size());
  for (unsigned j = 0; j < nof_reported_ues; ++j) {
    uint32_t ue_id =
        ric_ind_msg.ind_msg_formats.ind_msg_format3().ue_meas_report_list[j].ue_id.gnb_du_ue_id().gnb_cu_ue_f1ap_id;
    std::vector<uint32_t>::iterator it     = std::find(ue_ids.begin(), ue_ids.end(), ue_id);
    uint32_t                        ue_idx = std::distance(ue_ids.begin(), it);
    TESTASSERT_EQ(nof_meas_data,
                  ric_ind_msg.ind_msg_formats.ind_msg_format3().ue_meas_report_list[j].meas_report.meas_data.size());
    for (unsigned i = 0; i < nof_meas_data; ++i) {
      auto& meas_record =
          ric_ind_msg.ind_msg_formats.ind_msg_format3().ue_meas_report_list[j].meas_report.meas_data[i].meas_record;
      TESTASSERT_EQ(nof_records, meas_record.size());
      if (cond_presence[i][ue_idx]) {
        TESTASSERT_EQ(meas_values[i][ue_idx], meas_record[0].integer());
      } else {
        TESTASSERT_EQ(fmt::underlying(meas_record_item_c::types_opts::no_value),
                      fmt::underlying(meas_record[0].type().value));
      }
    }
  }

  if (g_enable_pcap) {
    e2_message e2_msg = generate_e2_ind_msg(ind_hdr_bytes, ind_msg_bytes);
    packer->handle_message(e2_msg);
  }
}

TEST_P(e2sm_kpm_indication, e2sm_kpm_generates_ric_indication_style5)
{
  std::vector<uint32_t> ue_ids = {2, 81, 22, 5, 18};
  du_meas_provider->set_ue_ids(ue_ids);
  // presence, cond_satisfied and meas value in 5 time slots for 5 UEs.
  std::vector<std::vector<uint32_t>> presence = {
      {0, 1, 1, 0, 1},
      {1, 0, 1, 0, 0},
      {1, 0, 1, 0, 0},
      {0, 1, 0, 0, 1},
      {1, 1, 0, 0, 1},
  };
  std::vector<uint32_t>              cond_satisfied = {1, 1, 1, 1, 1};
  std::vector<std::vector<uint32_t>> meas_values    = {
      {1, 2, 3, 0, 5},
      {11, 12, 13, 0, 15},
      {21, 22, 23, 0, 25},
      {31, 32, 33, 0, 35},
      {41, 42, 43, 0, 45},
  };
  uint32_t nof_meas_data = presence.size();
  uint32_t nof_ues       = ue_ids.size();
  uint32_t nof_metrics   = 1;
  uint32_t nof_records   = nof_metrics;
  // Measurement records are no_value before UE is present and satisfies conditions.
  std::vector<std::vector<uint32_t>> cond_presence;
  get_presence_starting_with_cond_satisfied(presence, {}, cond_presence);
  std::vector<uint32_t> reported_ues     = get_reported_ues(cond_presence);
  uint32_t              nof_reported_ues = reported_ues.size();

  // Define E2SM_KPM action format 5.
  e2sm_kpm_action_definition_s action_def;
  action_def.ric_style_type = 5;
  e2sm_kpm_action_definition_format5_s& action_def_f5 =
      action_def.action_definition_formats.set_action_definition_format5();

  action_def_f5.matching_ue_id_list.resize(nof_ues);
  for (unsigned i = 0; i < nof_ues; i++) {
    action_def_f5.matching_ue_id_list[i].ue_id.set_gnb_du_ue_id() = generate_ueid_gnb_du(ue_ids[i]);
  }

  e2sm_kpm_action_definition_format1_s& subscript_info = action_def_f5.sub_info;
  subscript_info.cell_global_id_present                = false;
  subscript_info.granul_period                         = 100;

  meas_info_item_s meas_info_item;
  meas_info_item.meas_type.set_meas_name().from_string("test"); // Dummy metric not supported.
  label_info_item_s label_info_item{};
  label_info_item.meas_label.no_label_present = true;
  label_info_item.meas_label.no_label         = meas_label_s::no_label_opts::true_value;
  meas_info_item.label_info_list.push_back(label_info_item);

  subscript_info.meas_info_list.push_back(meas_info_item);

  asn1::e2ap::ric_action_to_be_setup_item_s ric_action = generate_e2sm_kpm_ric_action(action_def);
  ASSERT_FALSE(e2sm_kpm_iface->action_supported(ric_action));

  subscript_info.meas_info_list[0].meas_type.set_meas_name().from_string("DRB.UEThpDl"); // Change to a valid metric.
  ric_action = generate_e2sm_kpm_ric_action(action_def);

  if (g_enable_pcap) {
    // Save E2 Subscription Request.
    e2_message e2_subscript_req = generate_e2sm_kpm_subscription_request(ric_action);
    packer->handle_message(e2_subscript_req);
  }

  ASSERT_TRUE(e2sm_kpm_iface->action_supported(ric_action));
  auto report_service = e2sm_kpm_iface->get_e2sm_report_service(ric_action.ric_action_definition);

  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());
  // Fill only with no_values and check if indication is ready.
  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements.
    du_meas_provider->push_measurements_int({0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0});
    // Trigger measurement collection.
    report_service->collect_measurements();
  }
  TESTASSERT_EQ(false, report_service->is_ind_msg_ready());

  for (unsigned i = 0; i < nof_meas_data; ++i) {
    // Push dummy metric measurements.
    du_meas_provider->push_measurements_int(presence[i], cond_satisfied, meas_values[i]);
    // Trigger measurement collection.
    report_service->collect_measurements();
  }

  TESTASSERT_EQ(true, report_service->is_ind_msg_ready());
  // Get RIC indication msg content.
  byte_buffer ind_hdr_bytes = report_service->get_indication_header();
  byte_buffer ind_msg_bytes = report_service->get_indication_message();

  // Decode RIC Indication and check the content.
  e2sm_kpm_ind_msg_s ric_ind_msg;
  asn1::cbit_ref     ric_ind_bref(ind_msg_bytes);
  if (ric_ind_msg.unpack(ric_ind_bref) != asn1::SRSASN_SUCCESS) {
    test_logger.debug("e2sm_kpm: RIC indication msg could not be unpacked");
    return;
  }

  TESTASSERT_EQ(nof_reported_ues, ric_ind_msg.ind_msg_formats.ind_msg_format3().ue_meas_report_list.size());
  for (unsigned j = 0; j < nof_reported_ues; ++j) {
    uint32_t ue_id =
        ric_ind_msg.ind_msg_formats.ind_msg_format3().ue_meas_report_list[j].ue_id.gnb_du_ue_id().gnb_cu_ue_f1ap_id;
    std::vector<uint32_t>::iterator it     = std::find(ue_ids.begin(), ue_ids.end(), ue_id);
    uint32_t                        ue_idx = std::distance(ue_ids.begin(), it);
    TESTASSERT_EQ(nof_meas_data,
                  ric_ind_msg.ind_msg_formats.ind_msg_format3().ue_meas_report_list[j].meas_report.meas_data.size());
    for (unsigned i = 0; i < nof_meas_data; ++i) {
      auto& meas_record =
          ric_ind_msg.ind_msg_formats.ind_msg_format3().ue_meas_report_list[j].meas_report.meas_data[i].meas_record;
      TESTASSERT_EQ(nof_records, meas_record.size());
      if (cond_presence[i][ue_idx]) {
        TESTASSERT_EQ(meas_values[i][ue_idx], meas_record[0].integer());
      } else {
        TESTASSERT_EQ(fmt::underlying(meas_record_item_c::types_opts::no_value),
                      fmt::underlying(meas_record[0].type().value));
      }
    }
  }

  if (g_enable_pcap) {
    e2_message e2_msg = generate_e2_ind_msg(ind_hdr_bytes, ind_msg_bytes);
    packer->handle_message(e2_msg);
  }
}

INSTANTIATE_TEST_SUITE_P(e2sm_kpm_tests, e2_entity_test_with_pcap, testing::Values(g_pcap));
INSTANTIATE_TEST_SUITE_P(e2sm_kpm_tests, e2sm_kpm_indication, testing::Values(g_pcap));

int main(int argc, char** argv)
{
  // Check for '--enable_pcap' cmd line argument, do not use getopt as it interferes with gtest.
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--enable_pcap") {
      g_enable_pcap = true;
    }
  }

  srslog::init();

  std::unique_ptr<task_worker_executor> pcap_exec;
  std::unique_ptr<task_worker>          pcap_worker;
  std::unique_ptr<dlt_pcap>             common_pcap_writer;

  if (g_enable_pcap) {
    pcap_worker        = std::make_unique<task_worker>("pcap_worker", 128);
    pcap_exec          = std::make_unique<task_worker_executor>(*pcap_worker);
    common_pcap_writer = create_e2ap_pcap("/tmp/e2sm_kpm_test.pcap", *pcap_exec);
    g_pcap             = common_pcap_writer.get();
  }

  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
