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

#include "e2sm_kpm_asn1_packer.h"

using namespace asn1::e2ap;
using namespace asn1::e2sm;
using namespace srsran;

const std::string e2sm_kpm_asn1_packer::short_name       = "ORAN-E2SM-KPM";
const std::string e2sm_kpm_asn1_packer::oid              = "1.3.6.1.4.1.53148.1.2.2.2";
const std::string e2sm_kpm_asn1_packer::func_description = "KPM Monitor";
const uint32_t    e2sm_kpm_asn1_packer::ran_func_id      = 2;
const uint32_t    e2sm_kpm_asn1_packer::revision         = 0;

e2sm_kpm_asn1_packer::e2sm_kpm_asn1_packer(e2sm_kpm_meas_provider& meas_provider_) : meas_provider(meas_provider_) {}

e2sm_action_definition
e2sm_kpm_asn1_packer::handle_packed_e2sm_action_definition(const srsran::byte_buffer& action_definition)
{
  e2sm_action_definition action_def;
  asn1::cbit_ref         bref(action_definition);
  action_def.service_model = e2sm_service_model_t::KPM;
  if (std::get<e2sm_kpm_action_definition_s>(action_def.action_definition).unpack(bref) != asn1::SRSASN_SUCCESS) {
    printf("Failed to unpack E2SM KPM Action Definition\n");
    action_def.service_model = e2sm_service_model_t::UNKNOWN_SM;
  }
  return action_def;
}

e2sm_ric_control_request
e2sm_kpm_asn1_packer::handle_packed_ric_control_request(const asn1::e2ap::ric_ctrl_request_s& req)
{
  printf("Failure - RIC control not available in e2sm_kpm.\n");
  e2sm_ric_control_request control_request = {};
  return control_request;
}

e2_ric_control_response e2sm_kpm_asn1_packer::pack_ric_control_response(const e2sm_ric_control_response& e2sm_response)
{
  printf("Failure - RIC control not available in e2sm_kpm.\n");
  e2_ric_control_response control_response = {};
  return control_response;
}

e2sm_event_trigger_definition
e2sm_kpm_asn1_packer::handle_packed_event_trigger_definition(const srsran::byte_buffer& event_trigger_definition)
{
  e2sm_event_trigger_definition       e2sm_event_trigger_def;
  e2sm_kpm_event_trigger_definition_s e2sm_kpm_event_trigger_def;
  asn1::cbit_ref                      bref(event_trigger_definition);
  if (e2sm_kpm_event_trigger_def.unpack(bref) != asn1::SRSASN_SUCCESS) {
    printf("Failed to unpack E2SM KPM Event Trigger Definition\n");
  }

  e2sm_event_trigger_def.ric_service_type = e2sm_event_trigger_definition::e2sm_ric_service_type_t::REPORT;
  e2sm_event_trigger_def.report_period =
      e2sm_kpm_event_trigger_def.event_definition_formats.event_definition_format1().report_period;

  return e2sm_event_trigger_def;
}

asn1::unbounded_octstring<true> e2sm_kpm_asn1_packer::pack_ran_function_description()
{
  e2sm_kpm_ran_function_description_s ran_function_desc;
  // Add ran_function_name item.
  ran_function_desc.ran_function_name.ran_function_short_name.resize(short_name.size());
  ran_function_desc.ran_function_name.ran_function_e2sm_o_id.resize(oid.size());
  ran_function_desc.ran_function_name.ran_function_description.resize(func_description.size());

  ran_function_desc.ran_function_name.ran_function_short_name.from_string(short_name);
  ran_function_desc.ran_function_name.ran_function_e2sm_o_id.from_string(oid);
  ran_function_desc.ran_function_name.ran_function_description.from_string(func_description);
  ran_function_desc.ext = false;
  // Add ric_event_trigger_style item.
  ric_event_trigger_style_item_s ric_event_trigger_style_item;
  ric_event_trigger_style_item.ric_event_trigger_style_type = 1;
  ric_event_trigger_style_item.ric_event_trigger_style_name.from_string("Periodic Report");
  ric_event_trigger_style_item.ric_event_trigger_format_type = 1;
  ran_function_desc.ric_event_trigger_style_list.push_back(ric_event_trigger_style_item);

  // Addd Report Service Style Types, see O-RAN.WG3.E2SM-KPM-R003-v03.00.
  ric_report_style_item_s ric_report_style_item;

  std::vector<std::string> supported_e2_level_metrics = meas_provider.get_supported_metric_names(E2_NODE_LEVEL);
  std::vector<std::string> supported_ue_level_metrics = meas_provider.get_supported_metric_names(UE_LEVEL);

  if (!supported_e2_level_metrics.empty()) {
    // RIC Report Style Type 1.
    ric_report_style_item.ric_report_style_type = 1;
    ric_report_style_item.ric_report_style_name.from_string("E2 Node Measurement");
    ric_report_style_item.ric_action_format_type  = 1;
    ric_report_style_item.ric_ind_hdr_format_type = 1;
    ric_report_style_item.ric_ind_msg_format_type = 1;
    ric_report_style_item.meas_info_action_list.clear();
    for (const auto& metric : supported_e2_level_metrics) {
      meas_info_action_item_s meas_info_action_item;
      meas_info_action_item.meas_name.from_string(metric);
      meas_info_action_item.meas_id_present = false;
      ric_report_style_item.meas_info_action_list.push_back(meas_info_action_item);
    }
    ran_function_desc.ric_report_style_list.push_back(ric_report_style_item);
  }

  if (!supported_ue_level_metrics.empty()) {
    // RIC Report Style Type 2.
    ric_report_style_item.ric_report_style_type = 2;
    ric_report_style_item.ric_report_style_name.from_string("E2 Node Measurement for a single UE");
    ric_report_style_item.ric_action_format_type  = 2;
    ric_report_style_item.ric_ind_hdr_format_type = 1;
    ric_report_style_item.ric_ind_msg_format_type = 1;
    ric_report_style_item.meas_info_action_list.clear();
    for (const auto& metric : supported_ue_level_metrics) {
      meas_info_action_item_s meas_info_action_item;
      meas_info_action_item.meas_name.from_string(metric);
      meas_info_action_item.meas_id_present = false;
      ric_report_style_item.meas_info_action_list.push_back(meas_info_action_item);
    }
    ran_function_desc.ric_report_style_list.push_back(ric_report_style_item);

    // RIC Report Style Type 3.
    ric_report_style_item.ric_report_style_type = 3;
    ric_report_style_item.ric_report_style_name.from_string("Condition-based, UE-level E2 Node Measurement");
    ric_report_style_item.ric_action_format_type  = 3;
    ric_report_style_item.ric_ind_hdr_format_type = 1;
    ric_report_style_item.ric_ind_msg_format_type = 2;
    ric_report_style_item.meas_info_action_list.clear();
    for (const auto& metric : supported_ue_level_metrics) {
      meas_info_action_item_s meas_info_action_item;
      meas_info_action_item.meas_name.from_string(metric);
      meas_info_action_item.meas_id_present = false;
      ric_report_style_item.meas_info_action_list.push_back(meas_info_action_item);
    }
    ran_function_desc.ric_report_style_list.push_back(ric_report_style_item);

    // RIC Report Style Type 4.
    ric_report_style_item.ric_report_style_type = 4;
    ric_report_style_item.ric_report_style_name.from_string("Common Condition-based, UE-level Measurement");
    ric_report_style_item.ric_action_format_type  = 4;
    ric_report_style_item.ric_ind_hdr_format_type = 1;
    ric_report_style_item.ric_ind_msg_format_type = 3;
    ric_report_style_item.meas_info_action_list.clear();
    for (const auto& metric : supported_ue_level_metrics) {
      meas_info_action_item_s meas_info_action_item;
      meas_info_action_item.meas_name.from_string(metric);
      meas_info_action_item.meas_id_present = false;
      ric_report_style_item.meas_info_action_list.push_back(meas_info_action_item);
    }
    ran_function_desc.ric_report_style_list.push_back(ric_report_style_item);

    // RIC Report Style Type 5.
    ric_report_style_item.ric_report_style_type = 5;
    ric_report_style_item.ric_report_style_name.from_string("E2 Node Measurement for multiple UEs");
    ric_report_style_item.ric_action_format_type  = 5;
    ric_report_style_item.ric_ind_hdr_format_type = 1;
    ric_report_style_item.ric_ind_msg_format_type = 3;
    ric_report_style_item.meas_info_action_list.clear();
    for (const auto& metric : supported_ue_level_metrics) {
      meas_info_action_item_s meas_info_action_item;
      meas_info_action_item.meas_name.from_string(metric);
      meas_info_action_item.meas_id_present = false;
      ric_report_style_item.meas_info_action_list.push_back(meas_info_action_item);
    }
    ran_function_desc.ric_report_style_list.push_back(ric_report_style_item);
  }

  srsran::byte_buffer buf;
  asn1::bit_ref       bref(buf);
  if (ran_function_desc.pack(bref) != asn1::SRSASN_SUCCESS) {
    printf("Failed to pack E2SM KPM RAN Function Description\n");
    asn1::unbounded_octstring<true> err_buf;
    return err_buf;
  }

  asn1::unbounded_octstring<true> ran_function_description(std::move(buf));
  return ran_function_description;
}
