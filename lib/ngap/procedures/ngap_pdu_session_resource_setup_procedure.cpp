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

#include "ngap_pdu_session_resource_setup_procedure.h"
#include "../ngap_asn1_helpers.h"
#include "srsran/asn1/ngap/common.h"
#include "srsran/ngap/ngap.h"
#include "srsran/ngap/ngap_message.h"

#ifdef JBPF_ENABLED
#include "jbpf_srsran_hooks.h"
#endif

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::ngap;

ngap_pdu_session_resource_setup_procedure::ngap_pdu_session_resource_setup_procedure(
    const cu_cp_pdu_session_resource_setup_request&    request_,
    const asn1::ngap::pdu_session_res_setup_request_s& asn1_request_,
    const ngap_ue_ids&                                 ue_ids_,
    ngap_cu_cp_notifier&                               cu_cp_notifier_,
    ngap_metrics_aggregator&                           metrics_handler_,
    ngap_message_notifier&                             amf_notif_,
    ngap_ue_logger&                                    logger_) :
  request(request_),
  asn1_request(asn1_request_),
  ue_ids(ue_ids_),
  cu_cp_notifier(cu_cp_notifier_),
  metrics_handler(metrics_handler_),
  amf_notifier(amf_notif_),
  logger(logger_)
{
  // Map PDU session ID to S-NSSAI for metrics.
  for (const auto& pdu_session : request.pdu_session_res_setup_items) {
    pdu_session_id_to_snssai[pdu_session.pdu_session_id] = pdu_session.s_nssai;
  }
}

void ngap_pdu_session_resource_setup_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.log_debug("\"{}\" started...", name());

  
#ifdef JBPF_ENABLED 
  {
    struct jbpf_ngap_ctx_info ctx_info = {0, (uint64_t)request.ue_index,
      (ue_ids.ran_ue_id != ran_ue_id_t::invalid), ran_ue_id_to_uint(ue_ids.ran_ue_id),
      (ue_ids.amf_ue_id != amf_ue_id_t::invalid), amf_ue_id_to_uint(ue_ids.amf_ue_id)};
    hook_ngap_procedure_started(&ctx_info, NGAP_PROCEDURE_PDU_SESSION_SETUP, 0);
  }
#endif

  // Verify PDU Session Resource Setup Request.
  verification_outcome = verify_pdu_session_resource_setup_request(request, asn1_request, logger);

  if (verification_outcome.request.pdu_session_res_setup_items.empty()) {
    logger.log_info("Validation of PDUSessionResourceSetupRequest failed");
    response = verification_outcome.response;
  } else {
    // Add NAS PDU to PDU Session Resource Setup Request.
    if (asn1_request->nas_pdu_present) {
      verification_outcome.request.nas_pdu = asn1_request->nas_pdu.copy();
    }

    // Handle mandatory IEs.
    CORO_AWAIT_VALUE(response, cu_cp_notifier.on_new_pdu_session_resource_setup_request(verification_outcome.request));

    // Combine validation response with DU processor response.
    combine_pdu_session_resource_setup_response();
  }

  if (!response.pdu_session_res_failed_to_setup_items.empty()) {
    logger.log_warning("Some or all PDUSessionResourceSetupItems failed to setup");
  }

  if (send_pdu_session_resource_setup_response()) {
    logger.log_debug("\"{}\" finished successfully", name());
#ifdef JBPF_ENABLED 
    {
      struct jbpf_ngap_ctx_info ctx_info = {0, (uint64_t)request.ue_index,
        (ue_ids.ran_ue_id != ran_ue_id_t::invalid), ran_ue_id_to_uint(ue_ids.ran_ue_id),
        (ue_ids.amf_ue_id != amf_ue_id_t::invalid), amf_ue_id_to_uint(ue_ids.amf_ue_id)};
      hook_ngap_procedure_completed(&ctx_info, NGAP_PROCEDURE_PDU_SESSION_SETUP, true, 0);
    }
#endif
  } else {
    logger.log_debug("\"{}\" failed", name());
#ifdef JBPF_ENABLED 
    {
      struct jbpf_ngap_ctx_info ctx_info = {0, (uint64_t)request.ue_index,
        (ue_ids.ran_ue_id != ran_ue_id_t::invalid), ran_ue_id_to_uint(ue_ids.ran_ue_id),
        (ue_ids.amf_ue_id != amf_ue_id_t::invalid), amf_ue_id_to_uint(ue_ids.amf_ue_id)};
      hook_ngap_procedure_completed(&ctx_info, NGAP_PROCEDURE_PDU_SESSION_SETUP, false, 0);
    }
#endif
  }


  CORO_RETURN();
}

void ngap_pdu_session_resource_setup_procedure::combine_pdu_session_resource_setup_response()
{
  for (const auto& setup_item : verification_outcome.response.pdu_session_res_setup_response_items) {
    response.pdu_session_res_setup_response_items.emplace(setup_item.pdu_session_id, setup_item);
  }
  for (const auto& failed_item : verification_outcome.response.pdu_session_res_failed_to_setup_items) {
    response.pdu_session_res_failed_to_setup_items.emplace(failed_item.pdu_session_id, failed_item);
  }
  if (!response.crit_diagnostics.has_value()) {
    response.crit_diagnostics = verification_outcome.response.crit_diagnostics;
  }
}

bool ngap_pdu_session_resource_setup_procedure::send_pdu_session_resource_setup_response()
{
  ngap_message ngap_msg = {};

  ngap_msg.pdu.set_successful_outcome();
  ngap_msg.pdu.successful_outcome().load_info_obj(ASN1_NGAP_ID_PDU_SESSION_RES_SETUP);

  fill_asn1_pdu_session_res_setup_response(ngap_msg.pdu.successful_outcome().value.pdu_session_res_setup_resp(),
                                           response);

  auto& pdu_session_res_setup_resp           = ngap_msg.pdu.successful_outcome().value.pdu_session_res_setup_resp();
  pdu_session_res_setup_resp->amf_ue_ngap_id = amf_ue_id_to_uint(ue_ids.amf_ue_id);
  pdu_session_res_setup_resp->ran_ue_ngap_id = ran_ue_id_to_uint(ue_ids.ran_ue_id);

  // Notify metrics handler about successful PDU sessions.
  for (const auto& pdu_session : response.pdu_session_res_setup_response_items) {
    metrics_handler.aggregate_successful_pdu_session_setup(pdu_session_id_to_snssai.at(pdu_session.pdu_session_id));
  }
  // Notify metrics handler about failed PDU sessions.
  for (const auto& pdu_session : response.pdu_session_res_failed_to_setup_items) {
    metrics_handler.aggregate_failed_pdu_session_setup(pdu_session_id_to_snssai.at(pdu_session.pdu_session_id),
                                                       pdu_session.unsuccessful_transfer.cause);
  }

  // Forward message to AMF.
  if (!amf_notifier.on_new_message(ngap_msg)) {
    logger.log_warning("AMF notifier is not set. Cannot send PDUSessionResourceSetupResponse");
    return false;
  }

  return true;
}
