/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
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

#include "ngap_handover_resource_allocation_procedure.h"
#include "../ngap_asn1_helpers.h"
#include "srsran/asn1/ngap/common.h"
#include "srsran/ngap/ngap_message.h"

#ifdef JBPF_ENABLED
#include "jbpf_srsran_hooks.h"
#endif

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::ngap;

ngap_handover_resource_allocation_procedure::ngap_handover_resource_allocation_procedure(
    const ngap_handover_request& request_,
    const amf_ue_id_t            amf_ue_id_,
    ngap_ue_context_list&        ue_ctxt_list_,
    ngap_cu_cp_notifier&         cu_cp_notifier_,
    ngap_message_notifier&       amf_notif_,
    timer_manager&               timers_,
    task_executor&               task_exec_,
    srslog::basic_logger&        logger_) :
  request(request_),
  amf_ue_id(amf_ue_id_),
  ue_ctxt_list(ue_ctxt_list_),
  cu_cp_notifier(cu_cp_notifier_),
  amf_notifier(amf_notif_),
  timers(timers_),
  task_exec(task_exec_),
  logger(logger_)
{
}

void ngap_handover_resource_allocation_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("ue={}: \"{}\" initialized", request.ue_index, name());


#ifdef JBPF_ENABLED 
  {
    struct jbpf_ngap_ctx_info ctx_info = {0, (uint64_t)request.ue_index,
      false, 0,
      (amf_ue_id != amf_ue_id_t::invalid), amf_ue_id_to_uint(amf_ue_id)};
    hook_ngap_procedure_started(&ctx_info, NGAP_PROCEDURE_RESOURCE_ALLOCATION, 0);
  }
#endif

  // Notify DU repository about handover request and await requst ack
  CORO_AWAIT_VALUE(response, cu_cp_notifier.on_ngap_handover_request(request));

  #ifdef JBPF_ENABLED 
  {
    struct jbpf_ngap_ctx_info ctx_info = {0, (uint64_t)request.ue_index,
      false, 0,
      (amf_ue_id != amf_ue_id_t::invalid), amf_ue_id_to_uint(amf_ue_id)};
    hook_ngap_procedure_completed(&ctx_info, NGAP_PROCEDURE_RESOURCE_ALLOCATION, response.success, 0);
  }
#endif

  if (response.success) {
    // Create NGAP UE
    if (create_ngap_ue(response.ue_index)) {
      // Update UE with AMF UE ID
      ngap_ue_context& ue_ctxt = ue_ctxt_list[response.ue_index];
      ue_ctxt_list.update_amf_ue_id(ue_ctxt.ue_ids.ran_ue_id, amf_ue_id);

      send_handover_request_ack(ue_ctxt.ue_ids.ue_index, ue_ctxt.ue_ids.ran_ue_id);
      logger.debug("ue={}: \"{}\" finalized", response.ue_index, name());
    } else {
      send_handover_failure();
      logger.debug("ue={}: \"{}\" failed", response.ue_index, name());
      CORO_EARLY_RETURN();
    }
  } else {
    send_handover_failure();
    logger.debug("ue={}: \"{}\" failed", response.ue_index, name());
    CORO_EARLY_RETURN();
  }

  CORO_RETURN();
}

bool ngap_handover_resource_allocation_procedure::create_ngap_ue(ue_index_t ue_index)
{
  // Create NGAP UE
  // Allocate RAN-UE-ID
  ran_ue_id_t ran_ue_id = ue_ctxt_list.allocate_ran_ue_id();
  if (ran_ue_id == ran_ue_id_t::invalid) {
    logger.error("ue={}: No RAN-UE-ID available", ue_index);
    return false;
  }

  // Notify CU-CP about creation of NGAP UE
  ngap_cu_cp_ue_notifier* ue_notifier = cu_cp_notifier.on_new_ngap_ue(ue_index);
  if (ue_notifier == nullptr) {
    logger.error("ue={}: Failed to create UE", ue_index);
    // Remove created UE context
    ue_ctxt_list.remove_ue_context(ue_index);
    return false;
  }

  // Create UE context and store it
  ue_ctxt_list.add_ue(ue_index, ran_ue_id, *ue_notifier, timers, task_exec);

  ue_ctxt_list[ue_index].logger.log_debug("Created UE");

  return true;
}

void ngap_handover_resource_allocation_procedure::send_handover_request_ack(ue_index_t ue_index, ran_ue_id_t ran_ue_id)
{
  ngap_message ngap_msg;

  ngap_msg.pdu.set_successful_outcome();
  ngap_msg.pdu.successful_outcome().load_info_obj(ASN1_NGAP_ID_HO_RES_ALLOC);

  fill_asn1_handover_resource_allocation_response(ngap_msg.pdu.successful_outcome().value.ho_request_ack(), response);

  auto& ho_request_ack           = ngap_msg.pdu.successful_outcome().value.ho_request_ack();
  ho_request_ack->amf_ue_ngap_id = amf_ue_id_to_uint(amf_ue_id);
  ho_request_ack->ran_ue_ngap_id = ran_ue_id_to_uint(ran_ue_id);

  amf_notifier.on_new_message(ngap_msg);
}

void ngap_handover_resource_allocation_procedure::send_handover_failure()
{
  ngap_message ngap_msg;
  ngap_msg.pdu.set_unsuccessful_outcome();
  ngap_msg.pdu.unsuccessful_outcome().load_info_obj(ASN1_NGAP_ID_HO_RES_ALLOC);

  fill_asn1_handover_resource_allocation_response(ngap_msg.pdu.unsuccessful_outcome().value.ho_fail(), response);

  auto& ho_fail           = ngap_msg.pdu.unsuccessful_outcome().value.ho_fail();
  ho_fail->amf_ue_ngap_id = amf_ue_id_to_uint(amf_ue_id);

  logger.info("ue={} amf_ue={}: Sending HoFailure", request.ue_index, amf_ue_id);
  amf_notifier.on_new_message(ngap_msg);
}
