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

#include "lib/cu_cp/cu_cp_controller/node_connection_notifier.h"
#include "lib/cu_cp/cu_cp_impl_interface.h"
#include "lib/cu_cp/cu_up_processor/cu_up_processor_impl_interface.h"
#include "lib/cu_cp/du_processor/du_processor.h"
#include "lib/cu_cp/ue_manager/ue_manager_impl.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/support/async/async_task.h"
#include "srsran/support/async/fifo_async_task_scheduler.h"
#include <cstdint>
#include <list>
#include <memory>
#include <variant>

namespace srsran {
namespace srs_cu_cp {

/// \brief Generate DU-to-CU RRC Container with CellGroupConfig.
byte_buffer generate_container_with_cell_group_config();

/// \brief Generate RRC Container with RRC Setup Complete message.
byte_buffer generate_rrc_setup_complete();

// Generate RRC Reconfiguration Complete PDU.
byte_buffer generate_rrc_reconfiguration_complete_pdu(unsigned transaction_id, uint8_t count);

struct dummy_du_processor_cu_cp_notifier : public du_processor_cu_cp_notifier {
public:
  explicit dummy_du_processor_cu_cp_notifier(ue_manager* ue_mng_ = nullptr) : ue_mng(ue_mng_) {}

  void attach_handler(cu_cp_du_event_handler* cu_cp_handler_, cu_cp_ue_removal_handler* ue_removal_handler_)
  {
    cu_cp_handler      = cu_cp_handler_;
    ue_removal_handler = ue_removal_handler_;
  }

  void on_rrc_ue_created(ue_index_t ue_index, rrc_ue_interface& rrc_ue) override
  {
    logger.info("ue={}: Received a RRC UE creation notification", ue_index);

    if (cu_cp_handler != nullptr) {
      cu_cp_handler->handle_rrc_ue_creation(ue_index, rrc_ue);
    }
  }

  byte_buffer on_target_cell_sib1_required(du_index_t du_index, nr_cell_global_id_t cgi) override
  {
    return make_byte_buffer("deadbeef").value();
  }

  async_task<void> on_ue_removal_required(ue_index_t ue_index) override
  {
    logger.info("ue={}: Received a UE removal request", ue_index);

    return launch_async([this, ue_index](coro_context<async_task<void>>& ctx) mutable {
      CORO_BEGIN(ctx);
      if (ue_removal_handler != nullptr) {
        ue_removal_handler->handle_ue_removal_request(ue_index);
      } else {
        if (ue_mng != nullptr) {
          ue_mng->remove_ue(ue_index);
        }

        if (rrc_removal_handler != nullptr) {
          rrc_removal_handler->remove_ue(ue_index);
        }
      }
      CORO_RETURN();
    });
  }

  async_task<void> on_ue_release_required(const cu_cp_ue_context_release_request& request) override
  {
    logger.info("ue={}: Received UE release request", request.ue_index);

    return launch_async([](coro_context<async_task<void>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN();
    });
  }

  async_task<void> on_transaction_info_loss(const f1_ue_transaction_info_loss_event& ev) override
  {
    return launch_async([](coro_context<async_task<void>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN();
    });
  }

private:
  srslog::basic_logger&     logger              = srslog::fetch_basic_logger("TEST");
  ue_manager*               ue_mng              = nullptr;
  cu_cp_du_event_handler*   cu_cp_handler       = nullptr;
  cu_cp_ue_removal_handler* ue_removal_handler  = nullptr;
  rrc_ue_handler*           rrc_removal_handler = nullptr;
};

struct dummy_cu_cp_ue_context_manipulation_handler : public cu_cp_ue_context_manipulation_handler {
public:
  async_task<void> handle_ue_context_release(const cu_cp_ue_context_release_request& request) override
  {
    logger.info("ue={}: Received UE release request", request.ue_index);

    return launch_async([](coro_context<async_task<void>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN();
    });
  }

  async_task<bool> handle_ue_context_transfer(ue_index_t ue_index, ue_index_t old_ue_index) override
  {
    logger.info("ue={} old_ue={}: Received UE transfer required", ue_index, old_ue_index);

    return launch_async([this](coro_context<async_task<bool>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN(ue_transfer_outcome);
    });
  }

  void handle_handover_reconfiguration_sent(const cu_cp_intra_cu_handover_target_request& request) override
  {
    logger.info("ue={}: Awaiting a RRC Reconfiguration Complete (transaction_id={})",
                request.target_ue_index,
                request.transaction_id);
    last_transaction_id = request.transaction_id;
  }

  void handle_handover_ue_context_push(ue_index_t source_ue_index, ue_index_t target_ue_index) override
  {
    logger.info("source_ue={} target_ue={}: Received handover ue context push", source_ue_index, target_ue_index);
  }

  void initialize_handover_ue_release_timer(ue_index_t                              ue_index,
                                            std::chrono::milliseconds               handover_ue_release_timeout,
                                            const cu_cp_ue_context_release_request& ue_context_release_request) override
  {
    logger.info("ue={}: Initializing UE release timer", ue_index);
  }

  unsigned last_transaction_id = 99999;

private:
  srslog::basic_logger& logger              = srslog::fetch_basic_logger("TEST");
  bool                  ue_transfer_outcome = true;
};

class dummy_cu_cp_ue_removal_handler : public cu_cp_ue_removal_handler
{
public:
  explicit dummy_cu_cp_ue_removal_handler(ue_manager* ue_mng_ = nullptr) : ue_mng(ue_mng_) {}

  async_task<void> handle_ue_removal_request(ue_index_t ue_index) override
  {
    if (ue_mng != nullptr) {
      ue_mng->remove_ue(ue_index);
    }

    return launch_async([](coro_context<async_task<void>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN();
    });
  }

  void handle_pending_ue_task_cancellation(ue_index_t ue_index) override {}

private:
  ue_manager* ue_mng = nullptr;
};

class dummy_du_connection_notifier : public du_connection_notifier
{
public:
  bool on_du_setup_request(du_index_t du_index, const du_setup_request& req) override { return true; }
};

struct dummy_ngap_ue_context_removal_handler : public ngap_ue_context_removal_handler {
public:
  void remove_ue_context(ue_index_t ue_index) override { logger.info("ue={}: Removing UE", ue_index); }

private:
  srslog::basic_logger& logger = srslog::fetch_basic_logger("TEST");
};

// Configuration struct to parameterize the modification outcome
struct pdu_session_modified_outcome_t {
  pdu_session_id_t      psi;
  std::vector<drb_id_t> drb_added;
  std::vector<drb_id_t> drb_removed;
};

// Stuct to configure Bearer Context Setup/Modification result content.
struct bearer_context_outcome_t {
  bool                outcome = false;
  std::list<unsigned> pdu_sessions_setup_list;  // List of PDU session IDs that were successful to setup.
  std::list<unsigned> pdu_sessions_failed_list; // List of PDU sessions IDs that failed to be setup.
  std::list<pdu_session_modified_outcome_t>
      pdu_sessions_modified_list; // List of PDU session IDs that were successfully modified.
};

struct dummy_e1ap_bearer_context_manager : public e1ap_bearer_context_manager {
public:
  dummy_e1ap_bearer_context_manager() = default;

  void set_first_message_outcome(const bearer_context_outcome_t& outcome) { first_e1ap_response = outcome; }

  void set_second_message_outcome(const bearer_context_outcome_t& outcome) { second_e1ap_response = outcome; }

  void fill_pdu_session_setup_list(
      slotted_id_vector<pdu_session_id_t, e1ap_pdu_session_resource_setup_modification_item>& e1ap_setup_list,
      const std::list<unsigned>&                                                              outcome_setup_list)
  {
    for (const auto& psi : outcome_setup_list) {
      // add only the most relevant items
      e1ap_pdu_session_resource_setup_modification_item res_setup_item;
      res_setup_item.pdu_session_id = uint_to_pdu_session_id(psi);

      // add a single DRB with the same ID like the PDU session it belongs to
      drb_id_t                   drb_id = uint_to_drb_id(psi); // Note: we use the PDU session ID here also as DRB ID
      e1ap_drb_setup_item_ng_ran drb_item;
      drb_item.drb_id = drb_id;

      // add a QoS flow
      e1ap_qos_flow_item qos_item;
      qos_item.qos_flow_id = uint_to_qos_flow_id(psi); // Note: use the PSI again as QoS flow ID
      drb_item.flow_setup_list.emplace(qos_item.qos_flow_id, qos_item);

      // add one UP transport item
      e1ap_up_params_item up_item;
      up_item.cell_group_id = 0;
      up_item.up_tnl_info   = {transport_layer_address::create_from_string("127.0.0.1"), int_to_gtpu_teid(0x1)};
      drb_item.ul_up_transport_params.push_back(up_item);
      res_setup_item.drb_setup_list_ng_ran.emplace(drb_id, drb_item);

      e1ap_setup_list.emplace(res_setup_item.pdu_session_id, res_setup_item);
    }
  }

  void fill_pdu_session_failed_list(
      slotted_id_vector<pdu_session_id_t, e1ap_pdu_session_resource_failed_item>& e1ap_failed_list,
      const std::list<unsigned>&                                                  outcome_failed_list)
  {
    for (const auto& id : outcome_failed_list) {
      e1ap_pdu_session_resource_failed_item res_failed_item;
      res_failed_item.pdu_session_id = uint_to_pdu_session_id(id);
      e1ap_failed_list.emplace(res_failed_item.pdu_session_id, res_failed_item);
    }
  }

  void fill_bearer_context_response(e1ap_bearer_context_setup_response& result, const bearer_context_outcome_t& outcome)
  {
    result.success = outcome.outcome;
    fill_pdu_session_setup_list(result.pdu_session_resource_setup_list, outcome.pdu_sessions_setup_list);
    fill_pdu_session_failed_list(result.pdu_session_resource_failed_list, outcome.pdu_sessions_failed_list);
  }

  void fill_pdu_session_modified_list(
      slotted_id_vector<pdu_session_id_t, e1ap_pdu_session_resource_modified_item>& e1ap_modified_list,
      const std::list<pdu_session_modified_outcome_t>&                              outcome_modified_list)
  {
    for (const auto& modified_item : outcome_modified_list) {
      // add only the most relevant items
      e1ap_pdu_session_resource_modified_item res_mod_item;
      res_mod_item.pdu_session_id = modified_item.psi;

      for (const auto& drb_id : modified_item.drb_added) {
        // add a single DRB with the same ID like the PDU session it belongs to
        e1ap_drb_setup_item_ng_ran drb_item;
        drb_item.drb_id = drb_id;

        // add a QoS flow
        e1ap_qos_flow_item qos_item;
        qos_item.qos_flow_id = uint_to_qos_flow_id(drb_id_to_uint(drb_id)); // QoS flow has same ID like DRB
        drb_item.flow_setup_list.emplace(qos_item.qos_flow_id, qos_item);

        // add one UP transport item
        e1ap_up_params_item up_item;
        drb_item.ul_up_transport_params.push_back(up_item);
        res_mod_item.drb_setup_list_ng_ran.emplace(drb_id, drb_item);
      }

      e1ap_modified_list.emplace(res_mod_item.pdu_session_id, res_mod_item);
    }
  }

  void fill_bearer_context_response(e1ap_bearer_context_modification_response& result,
                                    const bearer_context_outcome_t&            outcome)
  {
    result.success = outcome.outcome;
    fill_pdu_session_setup_list(result.pdu_session_resource_setup_list, outcome.pdu_sessions_setup_list);
    fill_pdu_session_failed_list(result.pdu_session_resource_failed_list, outcome.pdu_sessions_failed_list);
    fill_pdu_session_modified_list(result.pdu_session_resource_modified_list, outcome.pdu_sessions_modified_list);
  }

  async_task<e1ap_bearer_context_setup_response>
  handle_bearer_context_setup_request(const e1ap_bearer_context_setup_request& msg) override
  {
    logger.info("Received a new bearer context setup request");

    first_e1ap_request = msg; // store msg to verify content in TC

    return launch_async([res = e1ap_bearer_context_setup_response{}, msg, this](
                            coro_context<async_task<e1ap_bearer_context_setup_response>>& ctx) mutable {
      CORO_BEGIN(ctx);

      if (first_e1ap_response.has_value()) {
        const auto& response = first_e1ap_response.value();
        fill_bearer_context_response(res, response);

        // Invalidate E1 response so it's not consumed again.
        first_e1ap_response.reset();
      }

      CORO_RETURN(res);
    });
  }

  async_task<e1ap_bearer_context_modification_response>
  handle_bearer_context_modification_request(const e1ap_bearer_context_modification_request& msg) override
  {
    logger.info("Received a new bearer context modification request");

    // store msg to be verify content in TC
    if (first_e1ap_request.has_value()) {
      second_e1ap_request = msg;
    } else {
      first_e1ap_request = msg;
    }

    return launch_async([res = e1ap_bearer_context_modification_response{},
                         this](coro_context<async_task<e1ap_bearer_context_modification_response>>& ctx) mutable {
      CORO_BEGIN(ctx);

      if (first_e1ap_response.has_value()) {
        // first E1AP message is already a bearer modification
        const auto& response = first_e1ap_response.value();
        fill_bearer_context_response(res, response);
        first_e1ap_response.reset(); // Make sure it's not consumed again.
      } else if (second_e1ap_response.has_value()) {
        // second E1AP message is a bearer modification
        const auto& response = second_e1ap_response.value();
        fill_bearer_context_response(res, response);
        second_e1ap_response.reset(); // Make sure it's not consumed again.
      }
      CORO_RETURN(res);
    });
  }

  async_task<void> handle_bearer_context_release_command(const e1ap_bearer_context_release_command& cmd) override
  {
    logger.info("Received a new bearer context release command");

    last_release_command = cmd;

    return launch_async([](coro_context<async_task<void>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN();
    });
  }

  void reset()
  {
    first_e1ap_request.reset();
    second_e1ap_request.reset();
  }

  e1ap_bearer_context_release_command last_release_command;

  std::optional<std::variant<e1ap_bearer_context_setup_request, e1ap_bearer_context_modification_request>>
                                                          first_e1ap_request;
  std::optional<e1ap_bearer_context_modification_request> second_e1ap_request;

private:
  srslog::basic_logger&                   logger = srslog::fetch_basic_logger("TEST");
  std::optional<bearer_context_outcome_t> first_e1ap_response;
  std::optional<bearer_context_outcome_t> second_e1ap_response;
};

struct dummy_ngap_control_message_handler : public ngap_control_message_handler {
public:
  dummy_ngap_control_message_handler() = default;

  async_task<bool> handle_ue_context_release_request(const cu_cp_ue_context_release_request& msg) override
  {
    logger.info("Received a UE Context Release Request");
    return launch_async([this](coro_context<async_task<bool>>& ctx) {
      CORO_BEGIN(ctx);
      CORO_RETURN(release_request_outcome);
    });
  }

  async_task<ngap_handover_preparation_response>
  handle_handover_preparation_request(const ngap_handover_preparation_request& request) override
  {
    return launch_async([](coro_context<async_task<ngap_handover_preparation_response>>& ctx) {
      CORO_BEGIN(ctx);
      CORO_RETURN(ngap_handover_preparation_response{false});
    });
  }

  void handle_inter_cu_ho_rrc_recfg_complete(const ue_index_t           ue_index,
                                             const nr_cell_global_id_t& cgi,
                                             const tac_t                tac) override
  {
    logger.info("Received a RRC Reconfiguration Complete for Inter-CU Handover");
  }

  void set_ue_context_release_request_outcome(bool outcome_) { release_request_outcome = outcome_; }

private:
  bool                  release_request_outcome = true;
  srslog::basic_logger& logger                  = srslog::fetch_basic_logger("TEST");
};

struct ue_context_outcome_t {
  bool                outcome = false;
  std::list<unsigned> drb_success_list; // List of DRB IDs that were successful to setup.
  std::list<unsigned> drb_failed_list;  // List of DRB IDs that failed to be setup.
  std::list<unsigned> drb_removed_list; // List of DRB IDs that were removed.
  byte_buffer         cell_group_cfg = make_byte_buffer("5800b24223c853a0120c7c080408c008").value();
};

struct dummy_f1ap_ue_context_manager : public f1ap_ue_context_manager {
public:
  dummy_f1ap_ue_context_manager() = default;

  void set_ue_context_setup_outcome(bool outcome) { ue_context_setup_outcome = outcome; }

  void set_ue_context_modification_outcome(ue_context_outcome_t outcome)
  {
    ue_context_modification_outcome = std::move(outcome);
  }

  async_task<f1ap_ue_context_setup_response>
  handle_ue_context_setup_request(const f1ap_ue_context_setup_request&   request,
                                  std::optional<rrc_ue_transfer_context> rrc_context) override
  {
    logger.info("Received a new UE context setup request");

    return launch_async([res = f1ap_ue_context_setup_response{},
                         this](coro_context<async_task<f1ap_ue_context_setup_response>>& ctx) mutable {
      CORO_BEGIN(ctx);

      res.success                          = ue_context_setup_outcome;
      res.du_to_cu_rrc_info.cell_group_cfg = make_byte_buffer("5800b24223c853a0120c7c080408c008").value();

      CORO_RETURN(res);
    });
  }

  async_task<f1ap_ue_context_modification_response>
  handle_ue_context_modification_request(const f1ap_ue_context_modification_request& request) override
  {
    logger.info("Received a new UE context modification request");

    // store request so it can be verified in the test code
    make_partial_copy(ue_context_modification_request, request);

    return launch_async([res = f1ap_ue_context_modification_response{},
                         this](coro_context<async_task<f1ap_ue_context_modification_response>>& ctx) mutable {
      CORO_BEGIN(ctx);

      res.success = ue_context_modification_outcome.outcome;
      for (const auto& drb_id : ue_context_modification_outcome.drb_success_list) {
        // add only the most relevant items
        f1ap_drb_setupmod drb_item;
        drb_item.drb_id = uint_to_drb_id(drb_id); // set ID
        res.drbs_setup_list.push_back(drb_item);
      }
      res.du_to_cu_rrc_info.cell_group_cfg = ue_context_modification_outcome.cell_group_cfg.copy();
      // TODO: add failed list and other fields here ..

      CORO_RETURN(res);
    });
  }

  async_task<ue_index_t> handle_ue_context_release_command(const f1ap_ue_context_release_command& msg) override
  {
    logger.info("Received a new UE context release command");

    last_release_command.ue_index        = msg.ue_index;
    last_release_command.cause           = msg.cause;
    last_release_command.rrc_release_pdu = msg.rrc_release_pdu.copy();
    last_release_command.srb_id          = msg.srb_id;

    return launch_async([msg](coro_context<async_task<ue_index_t>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN(msg.ue_index);
    });
  }

  bool handle_ue_id_update(ue_index_t ue_index, ue_index_t old_ue_index) override { return true; }

  const f1ap_ue_context_modification_request& get_ctxt_mod_request() { return ue_context_modification_request; }

  f1ap_ue_context_release_command last_release_command;

private:
  void make_partial_copy(f1ap_ue_context_modification_request&       target,
                         const f1ap_ue_context_modification_request& source)
  {
    // only copy fields that are actually checked in unit tests
    target.drbs_to_be_setup_mod_list = source.drbs_to_be_setup_mod_list;
    target.drbs_to_be_released_list  = source.drbs_to_be_released_list;
  }

  srslog::basic_logger& logger                   = srslog::fetch_basic_logger("TEST");
  bool                  ue_context_setup_outcome = false;
  ue_context_outcome_t  ue_context_modification_outcome;

  f1ap_ue_context_modification_request ue_context_modification_request;
};

struct dummy_cu_up_processor_cu_up_management_notifier : public cu_up_processor_cu_up_management_notifier {
public:
  dummy_cu_up_processor_cu_up_management_notifier() = default;

  void on_new_cu_up_connection() override { logger.info("Received a new CU-UP connection notification"); }

  void on_cu_up_remove_request_received(const cu_up_index_t cu_up_index) override
  {
    logger.info("Received a CU-UP remove request for cu_up_index={}", cu_up_index);
    last_cu_up_index_to_remove = cu_up_index;
  }

  cu_up_index_t last_cu_up_index_to_remove;

private:
  srslog::basic_logger& logger = srslog::fetch_basic_logger("TEST");
};

struct dummy_ue_task_scheduler : public ue_task_scheduler {
public:
  dummy_ue_task_scheduler(timer_manager& timers_, task_executor& exec_) : timer_db(timers_), exec(exec_) {}
  bool           schedule_async_task(async_task<void> task) override { return ctrl_loop.schedule(std::move(task)); }
  unique_timer   create_timer() override { return timer_db.create_unique_timer(exec); }
  timer_factory  get_timer_factory() override { return timer_factory{timer_db, exec}; }
  task_executor& get_executor() override { return exec; }

  void tick_timer() { timer_db.tick(); }

private:
  fifo_async_task_scheduler ctrl_loop{16};
  timer_manager&            timer_db;
  task_executor&            exec;
};

class dummy_cu_cp_rrc_ue_interface : public cu_cp_rrc_ue_interface
{
public:
  void add_ue_context(rrc_ue_reestablishment_context_response context) { reest_context = context; }

  bool next_ue_setup_response = true;

  rrc_ue_reestablishment_context_response
  handle_rrc_reestablishment_request(pci_t old_pci, rnti_t old_c_rnti, ue_index_t ue_index) override
  {
    logger.info("ue={} old_pci={} old_c-rnti={}: Received RRC Reestablishment Request", ue_index, old_pci, old_c_rnti);

    return reest_context;
  }

  async_task<bool> handle_rrc_reestablishment_context_modification_required(ue_index_t ue_index) override
  {
    logger.info("ue={}: Received Reestablishment Context Modification Required");

    return launch_async([](coro_context<async_task<bool>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN(true);
    });
  }

  void handle_rrc_reestablishment_failure(const cu_cp_ue_context_release_request& request) override
  {
    logger.info("ue={}: Received RRC Reestablishment failure notification", request.ue_index);
  }

  void handle_rrc_reestablishment_complete(ue_index_t old_ue_index) override
  {
    logger.info("ue={}: Received RRC Reestablishment complete notification", old_ue_index);
  }

  async_task<bool> handle_ue_context_transfer(ue_index_t ue_index, ue_index_t old_ue_index) override
  {
    logger.info("ue={}: Requested a UE context transfer from old_ue={}", ue_index, old_ue_index);
    return launch_async([](coro_context<async_task<bool>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN(true);
    });
  }

  async_task<void> handle_ue_context_release(const cu_cp_ue_context_release_request& request) override
  {
    logger.info("ue={}: Requested a UE release", request.ue_index);
    last_cu_cp_ue_context_release_request = request;

    return launch_async([](coro_context<async_task<void>>& ctx) mutable {
      CORO_BEGIN(ctx);
      CORO_RETURN();
    });
  }

  cu_cp_ue_context_release_request last_cu_cp_ue_context_release_request;

private:
  rrc_ue_reestablishment_context_response reest_context = {};
  srslog::basic_logger&                   logger        = srslog::fetch_basic_logger("TEST");
};

} // namespace srs_cu_cp
} // namespace srsran
