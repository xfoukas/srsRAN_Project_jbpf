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

#pragma once

#include "srsran/asn1/e2ap/e2ap.h"
#include "srsran/asn1/e2sm/e2sm_kpm_ies.h"
#include "srsran/asn1/e2sm/e2sm_rc_ies.h"
#include "srsran/e2/e2_messages.h"
#include "srsran/support/async/async_task.h"
#include <map>
#include <variant>

namespace srsran {

enum e2sm_service_model_t { KPM, RC, UNKNOWN_SM };

struct e2sm_event_trigger_definition {
  enum e2sm_ric_service_type_t { REPORT, INSERT, POLICY, UNKNOWN };
  e2sm_ric_service_type_t ric_service_type;
  uint64_t                report_period;
};

struct e2sm_action_definition {
  e2sm_service_model_t                                                                            service_model;
  std::variant<asn1::e2sm::e2sm_kpm_action_definition_s, asn1::e2sm::e2sm_rc_action_definition_s> action_definition;
};

struct e2sm_ric_control_request {
  e2sm_service_model_t                         service_model                = e2sm_service_model_t::RC;
  bool                                         ric_call_process_id_present  = false;
  bool                                         ric_ctrl_ack_request_present = false;
  uint64_t                                     ric_call_process_id;
  std::variant<asn1::e2sm::e2sm_rc_ctrl_hdr_s> request_ctrl_hdr;
  std::variant<asn1::e2sm::e2sm_rc_ctrl_msg_s> request_ctrl_msg;
  bool                                         ric_ctrl_ack_request;
};

struct e2sm_ric_control_response {
  e2sm_service_model_t                             service_model = e2sm_service_model_t::RC;
  bool                                             success;
  bool                                             ric_call_process_id_present = false;
  bool                                             ric_ctrl_outcome_present    = false;
  uint64_t                                         ric_call_process_id;
  std::variant<asn1::e2sm::e2sm_rc_ctrl_outcome_s> ric_ctrl_outcome;
  asn1::e2ap::cause_c                              cause;
};

/// RIC control action executor maps an control action request to the proper stack functions.
class e2sm_control_action_executor
{
public:
  virtual ~e2sm_control_action_executor() = default;
  /// \brief get action id associated with the control action executor.
  /// \return Returns action id.
  virtual uint32_t get_action_id() = 0;
  /// \brief get control action definition to be used in RAN Function Description.
  /// \return Returns ran function definition ctrl action item.
  virtual asn1::e2sm::ran_function_definition_ctrl_action_item_s get_control_action_definition() = 0;
  /// \brief check if the requested RIC control action is supported.
  /// \param[in] req is a RIC control action request (with control header and message).
  /// \return Returns true if action supported by control action executor.
  virtual bool ric_control_action_supported(const e2sm_ric_control_request& req) = 0;
  /// \brief trigger execution of the RIC control action.
  /// \param[in] req is a RIC control action request (with control header and message).
  /// \return Returns RIC control response.
  virtual async_task<e2sm_ric_control_response> execute_ric_control_action(const e2sm_ric_control_request& req) = 0;

  template <typename T, typename Func>
  void parse_ran_parameter_value(const asn1::e2sm::ran_param_value_type_c& ran_param,
                                 uint64_t                                  ran_param_id,
                                 uint64_t                                  ue_id,
                                 T&                                        ctrl_cfg,
                                 Func                                      parse_action_ran_parameter_value)
  {
    parse_action_ran_parameter_value(ran_param, ran_param_id, ue_id, ctrl_cfg);

    if (ran_param.type() == asn1::e2sm::ran_param_value_type_c::types_opts::ran_p_choice_list) {
      for (auto& ran_p_list : ran_param.ran_p_choice_list().ran_param_list.list_of_ran_param) {
        for (auto& ran_p : ran_p_list.seq_of_ran_params) {
          if (action_params.find(ran_p.ran_param_id) != action_params.end()) {
            parse_ran_parameter_value(
                ran_p.ran_param_value_type, ran_p.ran_param_id, ue_id, ctrl_cfg, parse_action_ran_parameter_value);
          }
        }
      }
    } else if (ran_param.type() == asn1::e2sm::ran_param_value_type_c::types_opts::ran_p_choice_structure) {
      for (auto& ran_seq : ran_param.ran_p_choice_structure().ran_param_structure.seq_of_ran_params) {
        if (action_params.find(ran_seq.ran_param_id) != action_params.end()) {
          parse_ran_parameter_value(
              ran_seq.ran_param_value_type, ran_seq.ran_param_id, ue_id, ctrl_cfg, parse_action_ran_parameter_value);
        }
      }
    }
  }

protected:
  std::string                     action_name;
  std::map<uint32_t, std::string> action_params;
};

class e2sm_report_service
{
public:
  virtual ~e2sm_report_service() = default;
  /// \brief Trigger collection of metric measurements.
  /// \return Returns True if collection was successful.
  virtual bool collect_measurements() = 0;
  /// \brief check if a valid indication message was created (i.e. if it does not contain only no_values).
  /// \return Returns True if the indication message is ready to be sent.
  virtual bool is_ind_msg_ready() = 0;
  /// \brief get the indication message with data collected by the report service.
  /// \return Returns the packed resultant Indication Message.
  virtual srsran::byte_buffer get_indication_message() = 0;
  /// \brief get the indication header with data generated by the report service.
  /// \return Returns the indication header.
  virtual srsran::byte_buffer get_indication_header() = 0;
};

class e2sm_control_service
{
public:
  virtual ~e2sm_control_service() = default;
  /// \brief get style type id associated with the control service.
  /// \return Returns style type id.
  virtual uint32_t get_style_type() = 0;
  /// \brief get control service definition to be used in RAN Function Description.
  /// \return Returns ran function definition ctrl item.
  virtual asn1::e2sm::ran_function_definition_ctrl_item_s get_control_style_definition() = 0;
  /// \brief add an RIC control action executor associated with a specific action id.
  /// \param[in] act_executor is a RIC control action executor that maps RIC action to the proper stack functions.
  /// \return Returns true if action executor successfully added.
  virtual bool add_e2sm_rc_control_action_executor(std::unique_ptr<e2sm_control_action_executor> act_executor) = 0;
  /// \brief check if the requested RIC control action is supported.
  /// \param[in] req is a RIC control action request (with control header and message).
  /// \return Returns true if the action supported by control service.
  virtual bool control_request_supported(const e2sm_ric_control_request& req) = 0;
  /// \brief trigger execution of the RIC control action.
  /// \param[in] req is a RIC control action request (with control header and message).
  /// \return Returns RIC control response.
  virtual async_task<e2sm_ric_control_response> execute_control_request(const e2sm_ric_control_request& req) = 0;
};

class e2sm_handler
{
public:
  virtual ~e2sm_handler() = default;
  /// \brief Handle the packed E2SM Action Definition.
  /// \param[in] buf
  /// \return Returns the unpacked E2SM Action Definition.
  virtual e2sm_action_definition handle_packed_e2sm_action_definition(const srsran::byte_buffer& buf) = 0;
  /// \brief Handle the packed E2SM Event Trigger Definition.
  /// \param[in] buf
  /// \return Returns the E2SM Event Trigger Definition.
  virtual e2sm_event_trigger_definition handle_packed_event_trigger_definition(const srsran::byte_buffer& buf) = 0;
  /// \brief Handle the packed E2 RIC Control Request.
  /// \param[in] req E2 RIC Control Request.
  /// \return Returns the unpacked E2SM RIC Control Request.
  virtual e2sm_ric_control_request handle_packed_ric_control_request(const asn1::e2ap::ric_ctrl_request_s& req) = 0;
  /// \brief Pack the E2SM RIC Control Response.
  /// \param[in] response E2SM RIC Control Response.
  /// \return Returns the packed E2 RIC Control Response.
  virtual e2_ric_control_response pack_ric_control_response(const e2sm_ric_control_response& e2sm_response) = 0;
  /// @brief Pack the RAN function description.
  virtual asn1::unbounded_octstring<true> pack_ran_function_description() = 0;
};

class e2sm_interface
{
public:
  virtual ~e2sm_interface() = default;
  /// \brief gets a reference to the packer for this service model.
  /// \return Returns a reference to the packer.
  virtual e2sm_handler& get_e2sm_packer() = 0;
  /// \brief Check if the requested RIC action is supported.
  /// \param[in] ric_action is a RIC action to be setup.
  /// \return Returns true if action supported by E2SM.
  virtual bool action_supported(const asn1::e2ap::ric_action_to_be_setup_item_s& ric_action) = 0;
  /// \brief gets a unique_ptr to the e2sm report service for an action.
  /// \param[in] action is a RIC action to be setup (in binary form).
  /// \return Returns a unique_ptr to the e2sm report service.
  virtual std::unique_ptr<e2sm_report_service>
  get_e2sm_report_service(const srsran::byte_buffer& action_definition) = 0;
  /// \brief add an RIC control service associated with a specific style id.
  /// \param[in] control_service is a RIC control service.
  /// \return Returns true if the control service successfully added.
  virtual bool add_e2sm_control_service(std::unique_ptr<e2sm_control_service> control_service) = 0;
  /// \brief gets a unique_ptr to the e2sm control service for a control msg.
  /// \param[in] req is a RIC control action request (with control header and message).
  /// \return Returns a unique_ptr to the e2sm report service.
  virtual e2sm_control_service* get_e2sm_control_service(const e2sm_ric_control_request& req) = 0;
};
} // namespace srsran
