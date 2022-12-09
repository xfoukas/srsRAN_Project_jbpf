/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "ng_initial_context_setup_procedure.h"
#include "ng_procedure_helpers.h"

using namespace srsgnb;
using namespace srsgnb::srs_cu_cp;
using namespace asn1::ngap;

ng_initial_context_setup_procedure::ng_initial_context_setup_procedure(
    ngc_ue&                                         ue_,
    const asn1::ngap::init_context_setup_request_s& request_,
    ngc_message_notifier&                           amf_notif_,
    srslog::basic_logger&                           logger_) :
  ue(ue_), request(request_), amf_notifier(amf_notif_), logger(logger_)
{
}

void ng_initial_context_setup_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("Initial Context Setup Procedure started");

  // Handle mendatorty IEs
  CORO_AWAIT_VALUE(
      success,
      ue.get_rrc_ue_control_notifier().on_new_security_context(*request->ue_security_cap, *request->security_key));

  if (not success) {
    initial_context_failure_message fail_msg = {};
    fail_msg.cause.set_protocol();
    send_initial_context_setup_failure(fail_msg);

    logger.debug("Initial Context Setup Procedure finished");
    CORO_EARLY_RETURN();
  }

  // Handle optional IEs
  if (request->nas_pdu_present) {
    handle_nas_pdu(logger, request->nas_pdu.value, ue);
  }

  initial_context_response_message resp_msg = {};
  send_initial_context_setup_response(resp_msg);

  logger.debug("Initial Context Setup Procedure finished");
  CORO_RETURN();
}

void ng_initial_context_setup_procedure::send_initial_context_setup_response(
    const initial_context_response_message& msg)
{
  ngc_message ngc_msg = {};

  ngc_msg.pdu.set_successful_outcome();
  ngc_msg.pdu.successful_outcome().load_info_obj(ASN1_NGAP_ID_INIT_CONTEXT_SETUP);
  auto& init_ctxt_setup_resp                 = ngc_msg.pdu.successful_outcome().value.init_context_setup_resp();
  init_ctxt_setup_resp->amf_ue_ngap_id.value = amf_ue_id_to_uint(ue.get_amf_ue_id());
  init_ctxt_setup_resp->ran_ue_ngap_id.value = ran_ue_id_to_uint(ue.get_ran_ue_id());

  // Fill PDU Session Resource Setup Response List
  if (!msg.succeed_to_setup.empty()) {
    init_ctxt_setup_resp->pdu_session_res_setup_list_cxt_res_present = true;
    init_ctxt_setup_resp->pdu_session_res_setup_list_cxt_res->resize(msg.succeed_to_setup.size());
    for (auto& it : msg.succeed_to_setup) {
      asn1::ngap::pdu_session_res_setup_item_cxt_res_s res_item;
      res_item.pdu_session_id = it.pdu_session_id;
      res_item.pdu_session_res_setup_resp_transfer.resize(it.pdu_session_res.length());
      std::copy(
          it.pdu_session_res.begin(), it.pdu_session_res.end(), res_item.pdu_session_res_setup_resp_transfer.begin());

      init_ctxt_setup_resp->pdu_session_res_setup_list_cxt_res->push_back(res_item);
    }
  }

  // Fill PDU Session Resource Failed to Setup List
  if (!msg.failed_to_setup.empty()) {
    init_ctxt_setup_resp->pdu_session_res_failed_to_setup_list_cxt_res_present = true;
    init_ctxt_setup_resp->pdu_session_res_failed_to_setup_list_cxt_res->resize(msg.failed_to_setup.size());
    for (auto& it : msg.failed_to_setup) {
      asn1::ngap::pdu_session_res_failed_to_setup_item_cxt_res_s res_item;
      res_item.pdu_session_id = it.pdu_session_id;
      res_item.pdu_session_res_setup_unsuccessful_transfer.resize(it.pdu_session_res.length());
      std::copy(it.pdu_session_res.begin(),
                it.pdu_session_res.end(),
                res_item.pdu_session_res_setup_unsuccessful_transfer.begin());

      init_ctxt_setup_resp->pdu_session_res_failed_to_setup_list_cxt_res->push_back(res_item);
    }
  }

  // Fill Criticality Diagnostics
  if (msg.crit_diagnostics.has_value()) {
    init_ctxt_setup_resp->crit_diagnostics_present = true;
    init_ctxt_setup_resp->crit_diagnostics.value   = msg.crit_diagnostics.value();
  }

  logger.info("Sending Initial Context Setup Response to AMF");
  amf_notifier.on_new_message(ngc_msg);
}

void ng_initial_context_setup_procedure::send_initial_context_setup_failure(const initial_context_failure_message& msg)
{
  ngc_message ngc_msg = {};

  ngc_msg.pdu.set_unsuccessful_outcome();
  ngc_msg.pdu.unsuccessful_outcome().load_info_obj(ASN1_NGAP_ID_INIT_CONTEXT_SETUP);
  auto& init_ctxt_setup_fail                 = ngc_msg.pdu.unsuccessful_outcome().value.init_context_setup_fail();
  init_ctxt_setup_fail->amf_ue_ngap_id.value = amf_ue_id_to_uint(ue.get_amf_ue_id());
  init_ctxt_setup_fail->ran_ue_ngap_id.value = ran_ue_id_to_uint(ue.get_ran_ue_id());

  init_ctxt_setup_fail->cause.value = msg.cause;

  // Fill PDU Session Resource Failed to Setup List
  if (!msg.failed_to_setup.empty()) {
    init_ctxt_setup_fail->pdu_session_res_failed_to_setup_list_cxt_fail_present = true;
    init_ctxt_setup_fail->pdu_session_res_failed_to_setup_list_cxt_fail->resize(msg.failed_to_setup.size());
    for (auto& it : msg.failed_to_setup) {
      asn1::ngap::pdu_session_res_failed_to_setup_item_cxt_fail_s fail_item;
      fail_item.pdu_session_id = it.pdu_session_id;
      fail_item.pdu_session_res_setup_unsuccessful_transfer.resize(it.pdu_session_res.length());
      std::copy(it.pdu_session_res.begin(),
                it.pdu_session_res.end(),
                fail_item.pdu_session_res_setup_unsuccessful_transfer.begin());

      init_ctxt_setup_fail->pdu_session_res_failed_to_setup_list_cxt_fail->push_back(fail_item);
    }
  }

  // Fill Criticality Diagnostics
  if (msg.crit_diagnostics.has_value()) {
    init_ctxt_setup_fail->crit_diagnostics_present = true;
    init_ctxt_setup_fail->crit_diagnostics.value   = msg.crit_diagnostics.value();
  }

  logger.info("Sending Initial Context Setup Failure to AMF");
  amf_notifier.on_new_message(ngc_msg);
}
