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

#include "../mac_ctrl/mac_scheduler_configurator.h"
#include "../rnti_manager.h"
#include "mac_rach_handler.h"
#include "mac_scheduler_adapter.h"
#include "positioning_handler.h"
#include "rlf_detector.h"
#include "uci_cell_decoder.h"
#include "srsran/scheduler/mac_scheduler.h"
#include "srsran/support/async/manual_event.h"

namespace srsran {

/// \brief This class adapts srsRAN scheduler interface to operate with srsRAN MAC.
/// The configuration completion notification handling (e.g. ue creation complete) is deferred for later processing
/// rather than being processed inline. We defer the processing because we do not want it to take place while
/// the scheduler is still processing the slot_indication, given that latter has higher priority.
class srsran_scheduler_adapter final : public mac_scheduler_adapter
{
public:
  explicit srsran_scheduler_adapter(const mac_config& params, rnti_manager& rnti_mng_);

  void add_cell(const mac_scheduler_cell_creation_request& msg) override;

  void remove_cell(du_cell_index_t cell_index) override;

  void start_cell(du_cell_index_t cell_index) override { sched_impl->handle_cell_activation_request(cell_index); }

  void stop_cell(du_cell_index_t cell_index) override { sched_impl->handle_cell_deactivation_request(cell_index); }

  async_task<bool> handle_ue_creation_request(const mac_ue_create_request& msg) override;

  async_task<bool> handle_ue_reconfiguration_request(const mac_ue_reconfiguration_request& msg) override;

  async_task<bool> handle_ue_removal_request(const mac_ue_delete_request& msg) override;

  void handle_ue_config_applied(du_ue_index_t ue_index) override;

  void handle_ul_bsr_indication(const mac_bsr_ce_info& bsr) override;

  void handle_ul_sched_command(const mac_ul_scheduling_command& cmd) override;

  void handle_dl_mac_ce_indication(const mac_ce_scheduling_command& mac_ce) override;

  /// \brief Forward to scheduler an RLC DL buffer state update.
  void handle_dl_buffer_state_update(const mac_dl_buffer_state_indication_message& dl_bs_ind) override;

  void handle_ul_phr_indication(const mac_phr_ce_info& phr) override;

  void handle_crnti_ce_indication(du_ue_index_t old_ue_index, du_cell_index_t cell_index) override;

  void handle_paging_information(const paging_information& msg) override;

  const sched_result& slot_indication(slot_point slot_tx, du_cell_index_t cell_idx) override;

  void handle_error_indication(slot_point                         slot_tx,
                               du_cell_index_t                    cell_idx,
                               mac_cell_slot_handler::error_event event) override;

  void handle_si_change_indication(const si_scheduling_update_request& request) override;

  async_task<mac_cell_positioning_measurement_response>
  handle_positioning_measurement_request(du_cell_index_t                                 cell_index,
                                         const mac_cell_positioning_measurement_request& req) override;

  mac_cell_rach_handler& get_cell_rach_handler(du_cell_index_t cell_index) override
  {
    return cell_handlers[cell_index].get_rach_handler();
  }

  mac_cell_control_information_handler& get_cell_control_info_handler(du_cell_index_t cell_index) override
  {
    return cell_handlers[cell_index];
  }

private:
  class cell_handler final : public mac_cell_control_information_handler
  {
  public:
    cell_handler(srsran_scheduler_adapter& parent_, const sched_cell_configuration_request_message& sched_cfg);

    void handle_crc(const mac_crc_indication_message& msg) override;

    void handle_uci(const mac_uci_indication_message& msg) override;

    void handle_srs(const mac_srs_indication_message& msg) override;

    mac_cell_rach_handler_impl& get_rach_handler() { return rach_handler; }

    uci_cell_decoder uci_decoder;

    std::unique_ptr<positioning_handler> pos_handler;

  private:
    const du_cell_index_t     cell_idx = INVALID_DU_CELL_INDEX;
    srsran_scheduler_adapter& parent;

    // Handler of RACH indications for this cell.
    mac_cell_rach_handler_impl& rach_handler;
  };

  class sched_config_notif_adapter final : public sched_configuration_notifier
  {
  public:
    explicit sched_config_notif_adapter(srsran_scheduler_adapter& parent_) : parent(parent_) {}
    void on_ue_config_complete(du_ue_index_t ue_index, bool ue_creation_result) override;
    void on_ue_deletion_completed(du_ue_index_t ue_index) override;

  private:
    srsran_scheduler_adapter& parent;
  };

  // Allocator for TC-RNTIs.
  rnti_manager& rnti_mng;

  /// Detector of UE RLFs.
  rlf_detector          rlf_handler;
  task_executor&        ctrl_exec;
  srslog::basic_logger& logger;

  /// Notifier that is used by MAC to start and await configurations of the scheduler.
  sched_config_notif_adapter notifier;

  /// srsGNB scheduler.
  std::unique_ptr<mac_scheduler> sched_impl;

  /// Handler of RACH indications.
  mac_rach_handler rach_handler;

  std::atomic<slot_point>                                     last_slot_point;
  std::atomic<std::chrono::high_resolution_clock::time_point> last_slot_tp;

  /// List of event flags used by scheduler to notify that the configuration is complete.
  struct ue_notification_context {
    manual_event<bool> ue_config_ready;
  };
  std::array<ue_notification_context, MAX_NOF_DU_UES> sched_cfg_notif_map;

  /// Handler for each DU cell.
  slotted_id_table<du_cell_index_t, cell_handler, MAX_NOF_DU_CELLS> cell_handlers;
};

} // namespace srsran
