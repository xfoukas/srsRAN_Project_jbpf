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

#include "lib/mac/mac_config_interfaces.h"
#include "lib/mac/mac_ctrl/mac_scheduler_configurator.h"
#include "srsran/du/du_high/du_high_executor_mapper.h"
#include "srsran/mac/mac_cell_result.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/scheduler/scheduler_metrics.h"
#include "srsran/support/async/async_no_op_task.h"
#include "srsran/support/async/manual_event.h"
#include "srsran/support/executors/task_executor.h"

namespace srsran {

class mac_ctrl_dummy_configurer final : public mac_ctrl_configurator
{
public:
  rnti_t add_ue(du_ue_index_t ue_index, du_cell_index_t pcell_index, rnti_t rnti) override { return rnti; }
  void   remove_ue(du_ue_index_t ue_index) override {}
};

/// Dummy class to emulate MAC UL procedures to add/remove/configure UEs
class mac_ul_dummy_configurer final : public mac_ul_configurator
{
public:
  bool                                                                             expected_result   = true;
  bool                                                                             ul_ccch_forwarded = false;
  manual_event_flag                                                                ue_created_ev;
  std::optional<mac_ue_create_request>                                             last_ue_create_request;
  std::optional<mac_ue_delete_request>                                             last_ue_delete_request;
  std::optional<std::pair<du_ue_index_t, std::vector<mac_logical_channel_config>>> last_ue_bearers_added;
  std::optional<std::pair<du_ue_index_t, std::vector<lcid_t>>>                     last_ue_bearers_rem;

  async_task<bool> add_ue(const mac_ue_create_request& msg) override;
  async_task<bool> addmod_bearers(du_ue_index_t                                  ue_index,
                                  const std::vector<mac_logical_channel_config>& ul_logical_channels) override;
  async_task<bool> remove_bearers(du_ue_index_t ue_index, span<const lcid_t> lcids_to_rem) override;
  async_task<void> remove_ue(const mac_ue_delete_request& msg) override;
  bool             flush_ul_ccch_msg(du_ue_index_t ue_index, byte_buffer pdu) override
  {
    ul_ccch_forwarded = true;
    return true;
  }
  void handle_ue_config_applied(du_ue_index_t ue_index) override {}
};

class mac_cell_dummy_controller final : public mac_cell_controller
{
public:
  async_task<void>                       start() override;
  async_task<void>                       stop() override { return start(); }
  async_task<mac_cell_reconfig_response> reconfigure(const mac_cell_reconfig_request& request) override
  {
    return launch_no_op_task(mac_cell_reconfig_response{true});
  }
};

class mac_cell_dummy_time_mapper final : public mac_cell_time_mapper
{
public:
  std::optional<mac_cell_slot_time_info> get_last_mapping() const override { return std::nullopt; }
  std::optional<time_point>              get_time_point(slot_point slot) const override { return std::nullopt; }
  std::optional<slot_point>              get_slot_point(time_point time) const override { return std::nullopt; }
};

class mac_dl_dummy_configurer final : public mac_dl_configurator
{
public:
  bool                                                                             expected_result = true;
  manual_event_flag                                                                ue_created_ev;
  std::optional<mac_ue_create_request>                                             last_ue_create_request;
  std::optional<mac_ue_delete_request>                                             last_ue_delete_request;
  std::optional<std::pair<du_ue_index_t, std::vector<mac_logical_channel_config>>> last_ue_bearers_added;
  std::optional<std::pair<du_ue_index_t, std::vector<lcid_t>>>                     last_ue_bearers_rem;
  mac_cell_dummy_controller                                                        cell_ctrl;
  mac_cell_dummy_time_mapper                                                       time_mapper;

  async_task<bool> add_ue(const mac_ue_create_request& msg) override;
  async_task<void> remove_ue(const mac_ue_delete_request& msg) override;
  async_task<bool> addmod_bearers(du_ue_index_t                          ue_index,
                                  du_cell_index_t                        pcell_index,
                                  span<const mac_logical_channel_config> logical_channels) override;
  async_task<bool>
  remove_bearers(du_ue_index_t ue_index, du_cell_index_t pcell_index, span<const lcid_t> lcids_to_rem) override;

  mac_cell_controller& add_cell(const mac_cell_creation_request& cell_cfg, mac_cell_config_dependencies deps) override
  {
    return cell_ctrl;
  }
  void                  remove_cell(du_cell_index_t cell_index) override {}
  mac_cell_controller&  get_cell_controller(du_cell_index_t cell_index) override { return cell_ctrl; }
  mac_cell_time_mapper& get_time_mapper(du_cell_index_t cell_index) override { return time_mapper; }
};

class dummy_ue_executor_mapper : public srs_du::du_high_ue_executor_mapper
{
public:
  dummy_ue_executor_mapper(task_executor& exec_) : exec(exec_) {}

  void           rebind_executors(du_ue_index_t ue_index, du_cell_index_t pcell_index) override {}
  task_executor& ctrl_executor(du_ue_index_t ue_index) override { return exec; }
  task_executor& f1u_dl_pdu_executor(du_ue_index_t ue_index) override { return exec; }
  task_executor& mac_ul_pdu_executor(du_ue_index_t ue_index) override { return exec; }

  task_executor& exec;
};

class dummy_dl_executor_mapper : public srs_du::du_high_cell_executor_mapper
{
public:
  dummy_dl_executor_mapper(const std::initializer_list<task_executor*>& execs_) : execs(execs_.begin(), execs_.end()) {}

  task_executor& executor(du_cell_index_t cell_index) override { return *execs[cell_index % execs.size()]; }
  task_executor& slot_ind_executor(du_cell_index_t cell_index) override { return *execs[cell_index % execs.size()]; }

  std::vector<task_executor*> execs;
};

class dummy_mac_event_indicator : public mac_ul_ccch_notifier
{
public:
  std::optional<ul_ccch_indication_message> last_ccch_ind;

  void on_ul_ccch_msg_received(const ul_ccch_indication_message& msg) override { last_ccch_ind = msg; }

  bool verify_ul_ccch_msg(const ul_ccch_indication_message& test_msg)
  {
    if (not last_ccch_ind.has_value()) {
      return false;
    }
    bool test = last_ccch_ind.value().tc_rnti == test_msg.tc_rnti &&
                last_ccch_ind.value().cell_index == test_msg.cell_index &&
                last_ccch_ind.value().slot_rx == test_msg.slot_rx && last_ccch_ind.value().subpdu == test_msg.subpdu;
    return test;
  }

  bool verify_no_ul_ccch_msg() const { return not last_ccch_ind.has_value(); }
};

class dummy_mac_cell_result_notifier : public mac_cell_result_notifier
{
public:
  void on_new_downlink_scheduler_results(const mac_dl_sched_result& dl_res) override {}
  void on_new_downlink_data(const mac_dl_data_result& dl_data) override {}
  void on_new_uplink_scheduler_results(const mac_ul_sched_result& ul_res) override {}
  void on_cell_results_completion(slot_point slot) override {}
};

class dummy_mac_result_notifier : public mac_result_notifier
{
  dummy_mac_cell_result_notifier cell;

public:
  mac_cell_result_notifier& get_cell(du_cell_index_t cell_index) override { return cell; }
};

class mac_scheduler_dummy_adapter : public mac_scheduler_configurator
{
public:
  manual_event<bool>                   ue_created_ev;
  std::optional<mac_ue_create_request> last_ue_create_request;

  void add_cell(const mac_scheduler_cell_creation_request& msg) override {}

  void remove_cell(du_cell_index_t cell_index) override {}

  async_task<bool> handle_ue_creation_request(const mac_ue_create_request& msg) override;
  async_task<bool> handle_ue_reconfiguration_request(const mac_ue_reconfiguration_request& msg) override;
  async_task<bool> handle_ue_removal_request(const mac_ue_delete_request& msg) override;
  void             handle_ue_config_applied(du_ue_index_t ue_idx) override;

  class dummy_notifier : public sched_configuration_notifier
  {
    void on_ue_config_complete(du_ue_index_t ue_index, bool success) override {}
    void on_ue_deletion_completed(du_ue_index_t ue_index) override {}
  } notifier;
};

} // namespace srsran
