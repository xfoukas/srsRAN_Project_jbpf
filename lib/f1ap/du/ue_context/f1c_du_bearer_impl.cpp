/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "f1c_du_bearer_impl.h"
#include "du/procedures/f1ap_du_event_manager.h"
#include "srsran/asn1/f1ap/common.h"
#include "srsran/asn1/f1ap/f1ap_pdu_contents.h"
#include "srsran/f1ap/common/f1ap_message.h"
#include "srsran/pdcp/pdcp_sn_util.h"
#include "srsran/support/async/async_no_op_task.h"
#include "srsran/support/async/execute_on.h"

using namespace srsran;
using namespace srsran::srs_du;

f1c_srb0_du_bearer::f1c_srb0_du_bearer(f1ap_ue_context&           ue_ctxt_,
                                       const nr_cell_global_id_t& pcell_cgi,
                                       const byte_buffer&         du_cu_rrc_container_,
                                       f1ap_message_notifier&     f1ap_notifier_,
                                       f1c_rx_sdu_notifier&       f1c_rx_sdu_notifier_,
                                       f1ap_event_manager&        ev_manager_,
                                       task_executor&             ctrl_exec_,
                                       task_executor&             ue_exec_) :
  ue_ctxt(ue_ctxt_),
  nr_cgi(pcell_cgi),
  du_cu_rrc_container(du_cu_rrc_container_.copy()),
  f1ap_notifier(f1ap_notifier_),
  sdu_notifier(f1c_rx_sdu_notifier_),
  ev_manager(ev_manager_),
  ctrl_exec(ctrl_exec_),
  ue_exec(ue_exec_),
  logger(srslog::fetch_basic_logger("DU-F1"))
{
}

void f1c_srb0_du_bearer::handle_sdu(byte_buffer_chain sdu)
{
  // Ensure SRB tasks are handled within the control executor.
  if (not ctrl_exec.execute([this, sdu = std::move(sdu)]() {
        const protocol_transaction<f1ap_transaction_response> transaction =
            ev_manager.transactions.create_transaction();

        // Pack Initial UL RRC Message Transfer as per TS38.473, Section 8.4.1.
        f1ap_message msg;
        msg.pdu.set_init_msg().load_info_obj(ASN1_F1AP_ID_INIT_UL_RRC_MSG_TRANSFER);
        asn1::f1ap::init_ul_rrc_msg_transfer_s& init_msg = msg.pdu.init_msg().value.init_ul_rrc_msg_transfer();
        init_msg->gnb_du_ue_f1ap_id                      = gnb_du_ue_f1ap_id_to_uint(ue_ctxt.gnb_du_ue_f1ap_id);
        init_msg->nr_cgi.plmn_id                         = nr_cgi.plmn_id.to_bytes();
        init_msg->nr_cgi.nr_cell_id.from_number(nr_cgi.nci.value());
        init_msg->c_rnti = to_value(ue_ctxt.rnti);
        if (not init_msg->rrc_container.append(sdu.begin(), sdu.end())) {
          logger.error("UL {} SRB0 Tx PDU: Discarding Tx PDU. Cause: Failed to append SDU to RRC container.", ue_ctxt);
          return;
        }
        init_msg->du_to_cu_rrc_container_present           = true;
        init_msg->du_to_cu_rrc_container                   = std::move(du_cu_rrc_container);
        init_msg->sul_access_ind_present                   = false;
        init_msg->transaction_id                           = transaction.id();
        init_msg->ran_ue_id_present                        = false;
        init_msg->rrc_container_rrc_setup_complete_present = false;

        // Notify upper layers of the initial UL RRC Message Transfer.
        f1ap_notifier.on_new_message(msg);
      })) {
    logger.error("Tx PDU {}: Discarding SRB0 Tx PDU. Cause: The task executor queue is full.", ue_ctxt);
  }
}

void f1c_srb0_du_bearer::handle_transmit_notification(uint32_t highest_pdcp_sn, uint32_t queue_free_bytes)
{
  report_fatal_error("Transmission notifications do not exist for SRB0");
}

void f1c_srb0_du_bearer::handle_delivery_notification(uint32_t highest_pdcp_sn)
{
  report_fatal_error("Delivery notifications do not exist for SRB0");
}

void f1c_srb0_du_bearer::handle_pdu(byte_buffer pdu)
{
  // Change to UE execution context before forwarding the SDU to lower layers.
  if (not ue_exec.execute([this, sdu = std::move(pdu)]() mutable { sdu_notifier.on_new_sdu(std::move(sdu)); })) {
    logger.error("Rx {} PDU: Discarding SRB0 Rx PDU. Cause: The task executor queue is full.", ue_ctxt);
  }
}

async_task<bool> f1c_srb0_du_bearer::handle_pdu_and_await_delivery(byte_buffer               sdu,
                                                                   std::chrono::milliseconds time_to_wait)
{
  // Forward task to lower layers.
  handle_pdu(std::move(sdu));

  // For SRB0, there is no delivery notification.
  return launch_no_op_task(true);
}

async_task<bool> f1c_srb0_du_bearer::handle_pdu_and_await_transmission(byte_buffer               pdu,
                                                                       std::chrono::milliseconds time_to_wait)
{
  // Forward task to lower layers.
  handle_pdu(std::move(pdu));

  // For SRB0, there is no transmission notification.
  return launch_no_op_task(true);
}

// Max number of concurrent PDCP SN deliveries/transmissions being awaited on.
static constexpr size_t MAX_CONCURRENT_PDU_EVENTS = 8;

f1c_other_srb_du_bearer::f1c_other_srb_du_bearer(f1ap_ue_context&       ue_ctxt_,
                                                 srb_id_t               srb_id_,
                                                 f1ap_message_notifier& f1ap_notifier_,
                                                 f1c_rx_sdu_notifier&   f1c_sdu_notifier_,
                                                 f1ap_du_configurator&  du_configurator_,
                                                 task_executor&         ctrl_exec_,
                                                 task_executor&         ue_exec_) :
  ue_ctxt(ue_ctxt_),
  srb_id(srb_id_),
  f1ap_notifier(f1ap_notifier_),
  sdu_notifier(f1c_sdu_notifier_),
  du_configurator(du_configurator_),
  ctrl_exec(ctrl_exec_),
  ue_exec(ue_exec_),
  logger(srslog::fetch_basic_logger("DU-F1")),
  event_pool(MAX_CONCURRENT_PDU_EVENTS)
{
  // set up the always set event
  event_source_type source{du_configurator.get_timer_factory()};
  always_set_event.subscribe_to(source, std::chrono::milliseconds{0});
  source.set(true);
}

void f1c_other_srb_du_bearer::handle_sdu(byte_buffer_chain sdu)
{
  // Ensure SRB tasks are handled within the control executor as they involve access to the UE context.
  if (not ctrl_exec.execute([this, sdu = std::move(sdu)]() {
        gnb_cu_ue_f1ap_id_t cu_ue_id = ue_ctxt.gnb_cu_ue_f1ap_id;
        if (cu_ue_id >= gnb_cu_ue_f1ap_id_t::max) {
          logger.warning("ue={} rnti={} du_ue={} SRB={}: Discarding F1AP RX SDU. Cause: GNB-CU-UE-F1AP-ID is invalid.",
                         ue_ctxt.ue_index,
                         ue_ctxt.rnti,
                         ue_ctxt.gnb_du_ue_f1ap_id,
                         srb_id_to_uint(srb_id));
          return;
        }

        f1ap_message msg;

        // Fill F1AP UL RRC Message Transfer.
        msg.pdu.set_init_msg().load_info_obj(ASN1_F1AP_ID_UL_RRC_MSG_TRANSFER);
        asn1::f1ap::ul_rrc_msg_transfer_s& ul_msg = msg.pdu.init_msg().value.ul_rrc_msg_transfer();
        ul_msg->gnb_du_ue_f1ap_id                 = gnb_du_ue_f1ap_id_to_uint(ue_ctxt.gnb_du_ue_f1ap_id);
        ul_msg->gnb_cu_ue_f1ap_id                 = gnb_cu_ue_f1ap_id_to_uint(ue_ctxt.gnb_cu_ue_f1ap_id);
        ul_msg->srb_id                            = srb_id_to_uint(srb_id);
        if (not ul_msg->rrc_container.append(sdu.begin(), sdu.end())) {
          logger.error("Tx PDU {}: Discarding SRB{} Tx PDU. Cause: Failed to append SDU to RRC container.",
                       ue_ctxt,
                       srb_id_to_uint(srb_id));
          return;
        }
        ul_msg->sel_plmn_id_present           = false;
        ul_msg->new_gnb_du_ue_f1ap_id_present = false;

        f1ap_notifier.on_new_message(msg);

        // If a UE RRC config is pending, we consider the reception of a UL RRC message as the confirmation that the
        // RRC config has been applied by the UE.
        if (ue_ctxt.rrc_state == f1ap_ue_context::ue_rrc_state::config_pending) {
          ue_ctxt.rrc_state = f1ap_ue_context::ue_rrc_state::config_applied;
          du_configurator.on_ue_config_applied(ue_ctxt.ue_index);
        }
      })) {
    logger.error(
        "Tx PDU {}: Discarding SRB{} Tx PDU. Cause: The task executor queue is full.", ue_ctxt, srb_id_to_uint(srb_id));
  }
}

void f1c_other_srb_du_bearer::handle_pdu(srsran::byte_buffer pdu)
{
  if (pdu.length() < 3) {
    logger.warning(
        "DL {} Rx SRB{} PDU: Dropping PDU. Cause: Invalid length={}.", ue_ctxt, srb_id_to_uint(srb_id), pdu.length());
    return;
  }

  // Change to UE execution context before forwarding the SDU to lower layers.
  if (not ue_exec.execute([this, sdu = std::move(pdu)]() mutable { sdu_notifier.on_new_sdu(std::move(sdu)); })) {
    logger.error("Rx PDU {}: Discarding  SRB{} Rx PDU. Cause: The task executor queue is full.",
                 ue_ctxt,
                 srb_id_to_uint(srb_id));
  }
}

async_task<bool> f1c_other_srb_du_bearer::handle_pdu_and_await_transmission(byte_buffer               pdu,
                                                                            std::chrono::milliseconds time_to_wait)
{
  return handle_pdu_and_await(std::move(pdu), true, time_to_wait);
}

async_task<bool> f1c_other_srb_du_bearer::handle_pdu_and_await_delivery(byte_buffer               pdu,
                                                                        std::chrono::milliseconds time_to_wait)
{
  return handle_pdu_and_await(std::move(pdu), false, time_to_wait);
}

void f1c_other_srb_du_bearer::handle_transmit_notification(uint32_t highest_pdcp_sn, uint32_t queue_free_bytes)
{
  if (not ue_exec.defer([this, highest_pdcp_sn]() { handle_notification(highest_pdcp_sn, true); })) {
    logger.warning("Discarded transmit notification for SRB{} because the task executor queue is full.",
                   srb_id_to_uint(srb_id));
  }
}

void f1c_other_srb_du_bearer::handle_delivery_notification(uint32_t highest_pdcp_sn)
{
  if (not ue_exec.defer([this, highest_pdcp_sn]() { handle_notification(highest_pdcp_sn, false); })) {
    logger.warning("Discarded delivery notification for SRB{} because the task executor queue is full.",
                   srb_id_to_uint(srb_id));
  }
}

async_task<bool> f1c_other_srb_du_bearer::handle_pdu_and_await(byte_buffer               pdu,
                                                               bool                      tx_or_delivery,
                                                               std::chrono::milliseconds time_to_wait)
{
  // Extract PDCP SN.
  std::optional<uint32_t> pdcp_sn = get_pdcp_sn(pdu, pdcp_sn_size::size12bits, true, logger);

  // Check if PDCP SN extraction was successful.
  if (not pdcp_sn) {
    // If not, forward PDU to lower layers anyway. An RLF may be triggered.
    handle_pdu(std::move(pdu));

    // Return right-away, instead of waiting for notification.
    logger.warning("Rx PDU {}: Ignoring SRB{} Rx PDU {} notification. Cause: Failed to extract PDCP SN.",
                   ue_ctxt,
                   srb_id_to_uint(srb_id),
                   tx_or_delivery ? "Tx" : "delivery");
    return launch_no_op_task(false);
  }

  return launch_async([this,
                       pdcp_sn = pdcp_sn.value(),
                       tx_or_delivery,
                       time_to_wait,
                       result = event_result_type{},
                       sdu    = std::move(pdu)](coro_context<async_task<bool>>& ctx) mutable {
    CORO_BEGIN(ctx);

    CORO_AWAIT(execute_on_blocking(ue_exec));

    // Forward SDU to lower layers.
    sdu_notifier.on_new_sdu(std::move(sdu));

    // Await SDU delivery.
    CORO_AWAIT_VALUE(result, wait_for_notification(pdcp_sn, tx_or_delivery, time_to_wait));

    // Return back to control executor.
    CORO_AWAIT(execute_on_blocking(ctrl_exec));

    // True for success, false for timeout/cancel.
    CORO_RETURN(result.index() == 0);
  });
}

void f1c_other_srb_du_bearer::handle_notification(uint32_t highest_pdcp_sn, bool tx_or_delivery)
{
  std::optional<uint32_t>& last_sn        = tx_or_delivery ? last_pdcp_sn_transmitted : last_pdcp_sn_delivered;
  auto&                    pending_events = tx_or_delivery ? pending_transmissions : pending_deliveries;

  // Register the highest PDCP SN.
  last_sn = highest_pdcp_sn;

  // Trigger awaiters of delivery notifications.
  pending_events.erase(std::remove_if(pending_events.begin(),
                                      pending_events.end(),
                                      [highest_pdcp_sn](const auto& event) {
                                        if (event->pdcp_sn <= highest_pdcp_sn) {
                                          // Trigger awaiter and remove it from the list of pending events.
                                          event->source.set(true);
                                          return true;
                                        }
                                        return false;
                                      }),
                       pending_events.end());
}

f1c_other_srb_du_bearer::event_observer_type&
f1c_other_srb_du_bearer::wait_for_notification(uint32_t                  pdcp_sn,
                                               bool                      tx_or_delivery,
                                               std::chrono::milliseconds time_to_wait)
{
  std::optional<uint32_t>& last_sn        = tx_or_delivery ? last_pdcp_sn_transmitted : last_pdcp_sn_delivered;
  auto&                    pending_events = tx_or_delivery ? pending_transmissions : pending_deliveries;

  // Check if the PDU has been already delivered.
  if (last_sn.has_value() and last_sn.value() >= pdcp_sn) {
    // SN already notified.
    return always_set_event;
  }

  // Allocate a notification event awaitable.
  auto event_slot = event_pool.create(pdcp_sn, du_configurator.get_timer_factory(), time_to_wait);
  if (event_slot == nullptr) {
    logger.warning("Rx PDU {}: Ignoring {} Rx PDU {} notification. Cause: Failed to allocate notification handler.",
                   ue_ctxt,
                   srb_id,
                   tx_or_delivery ? "Tx" : "delivery");
    return always_set_event;
  }

  // Allocation successful. Return awaitable event.
  pending_events.emplace_back(std::move(event_slot));
  return pending_events.back()->observer;
}
