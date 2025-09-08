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

#include "reestablishment_context_modification_routine.h"
#include "pdu_session_routine_helpers.h"
#include "srsran/e1ap/cu_cp/e1ap_cu_cp_bearer_context_update.h"
#include "srsran/f1ap/ue_context_management_configs.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::rrc_nr;

reestablishment_context_modification_routine::reestablishment_context_modification_routine(
    ue_index_t                             ue_index_,
    const srsran::security::sec_as_config& security_cfg_,
    e1ap_bearer_context_manager&           e1ap_bearer_ctxt_mng_,
    f1ap_ue_context_manager&               f1ap_ue_ctxt_mng_,
    rrc_ue_interface*                      rrc_ue_,
    cu_cp_rrc_ue_interface&                cu_cp_notifier_,
    ue_task_scheduler&                     ue_task_sched_,
    up_resource_manager&                   up_resource_mng_,
    srslog::basic_logger&                  logger_) :
  ue_index(ue_index_),
  security_cfg(security_cfg_),
  e1ap_bearer_ctxt_mng(e1ap_bearer_ctxt_mng_),
  f1ap_ue_ctxt_mng(f1ap_ue_ctxt_mng_),
  rrc_ue(rrc_ue_),
  cu_cp_notifier(cu_cp_notifier_),
  ue_task_sched(ue_task_sched_),
  up_resource_mng(up_resource_mng_),
  logger(logger_)
{
}

void reestablishment_context_modification_routine::operator()(coro_context<async_task<bool>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("ue={}: \"{}\" initialized", ue_index, name());

  {
    // prepare first BearerContextModificationRequest
    generate_bearer_context_modification_request_for_new_ul_tnl();

    // call E1AP procedure and wait for BearerContextModificationResponse
    CORO_AWAIT_VALUE(
        bearer_context_modification_response,
        e1ap_bearer_ctxt_mng.handle_bearer_context_modification_request(bearer_context_modification_request));

    // Handle BearerContextModificationResponse and fill subsequent UE context modification
    if (!generate_ue_context_modification_request(
            ue_context_mod_request, bearer_context_modification_response.pdu_session_resource_modified_list)) {
      logger.warning("ue={}: \"{}\" failed to modify bearer at CU-UP", ue_index, name());
      CORO_EARLY_RETURN(false);
    }

    bearer_context_modification_request = {};
  }

  {
    // prepare UE Context Modification Request and call F1 notifier
    ue_context_mod_request.ue_index = ue_index;

    CORO_AWAIT_VALUE(ue_context_modification_response,
                     f1ap_ue_ctxt_mng.handle_ue_context_modification_request(ue_context_mod_request));

    // Handle UE Context Modification Response
    if (!generate_bearer_context_modification(bearer_context_modification_request,
                                              bearer_context_modification_response,
                                              ue_context_modification_response,
                                              up_resource_mng,
                                              true)) {
      logger.warning("ue={}: \"{}\" failed to modify UE context at DU", ue_index, name());
      CORO_EARLY_RETURN(false);
    }
  }

  // Inform CU-UP about the new TEID for UL F1u traffic
  {
    // add remaining fields to BearerContextModificationRequest
    bearer_context_modification_request.ue_index = ue_index;

    // call E1AP procedure and wait for BearerContextModificationResponse
    CORO_AWAIT_VALUE(
        bearer_context_modification_response,
        e1ap_bearer_ctxt_mng.handle_bearer_context_modification_request(bearer_context_modification_request));

    // Handle BearerContextModificationResponse
    if (!generate_ue_context_modification_request(
            ue_context_mod_request, bearer_context_modification_response.pdu_session_resource_modified_list)) {
      logger.warning("ue={}: \"{}\" failed to modify bearer at CU-UP", ue_index, name());
      CORO_EARLY_RETURN(false);
    }
  }

  {
    // prepare RRC Reconfiguration and call RRC UE notifier
    {
      // add SRB2 again
      std::vector<f1ap_srb_to_setup> srbs_to_setup_list;
      f1ap_srb_to_setup              srb_to_setup = {};
      srb_to_setup.srb_id                         = srb_id_t::srb2;
      srbs_to_setup_list.push_back(srb_to_setup);

      // convert pdu session context
      std::map<pdu_session_id_t, up_pdu_session_context_update> pdu_sessions_to_setup_list;
      for (const auto& pdu_session_id : up_resource_mng.get_pdu_sessions()) {
        up_pdu_session_context_update context_update{pdu_session_id};
        context_update.drb_to_add = up_resource_mng.get_pdu_session_context(pdu_session_id).drbs;

        pdu_sessions_to_setup_list.emplace(pdu_session_id, context_update);
      }

      if (!fill_rrc_reconfig_args(rrc_reconfig_args,
                                  srbs_to_setup_list,
                                  pdu_sessions_to_setup_list,
                                  {} /* No DRB to be removed */,
                                  ue_context_modification_response.du_to_cu_rrc_info,
                                  {},
                                  {} /* TODO: include meas config in context*/,
                                  true /* Reestablish SRBs */,
                                  true /* Reestablish DRBs */,
                                  std::nullopt /* don't update keys */,
                                  {},
                                  logger)) {
        logger.warning("ue={}: \"{}\" Failed to fill RrcReconfiguration", ue_index, name());
        CORO_EARLY_RETURN(false);
      }
    }

    CORO_AWAIT_VALUE(rrc_reconfig_result, rrc_ue->handle_rrc_reconfiguration_request(rrc_reconfig_args));

    // Handle RRC Reconfiguration result.
    if (not rrc_reconfig_result) {
      logger.warning("ue={}: \"{}\" RRC reconfiguration failed", ue_index, name());
      // Notify NGAP to request UE context release from AMF
      ue_task_sched.schedule_async_task(cu_cp_notifier.handle_ue_context_release(
          {ue_index, {}, ngap_cause_radio_network_t::release_due_to_ngran_generated_reason}));
      CORO_EARLY_RETURN(false);
    }
  }

  // we are done
  CORO_RETURN(true);
}

bool reestablishment_context_modification_routine::generate_bearer_context_modification_request_for_new_ul_tnl()
{
  bearer_context_modification_request.ue_index                 = ue_index;
  bearer_context_modification_request.new_ul_tnl_info_required = true;

  // Request new UL TNL info for all DRBs
  std::vector<pdu_session_id_t>          pdu_session_ids              = up_resource_mng.get_pdu_sessions();
  e1ap_ng_ran_bearer_context_mod_request ngran_bearer_context_mod_req = {};
  for (const pdu_session_id_t& psi : pdu_session_ids) {
    e1ap_pdu_session_res_to_modify_item pdu_sess_mod_item;
    pdu_sess_mod_item.pdu_session_id = psi;

    const std::map<drb_id_t, up_drb_context>& drbs = up_resource_mng.get_pdu_session_context(psi).drbs;
    for (const std::pair<const drb_id_t, up_drb_context>& drb : drbs) {
      logger.debug("{}, {}: Requesting new UL TNL for DRB", psi, drb.first);
      e1ap_drb_to_modify_item_ng_ran drb_to_mod = {};
      drb_to_mod.drb_id                         = drb.first;
      pdu_sess_mod_item.drb_to_modify_list_ng_ran.emplace(drb_to_mod.drb_id, drb_to_mod);
    }

    ngran_bearer_context_mod_req.pdu_session_res_to_modify_list.emplace(psi, pdu_sess_mod_item);
  }

  bearer_context_modification_request.ng_ran_bearer_context_mod_request = ngran_bearer_context_mod_req;
  return true;
}

bool reestablishment_context_modification_routine::generate_ue_context_modification_request(
    f1ap_ue_context_modification_request& ue_context_mod_req,
    const slotted_id_vector<pdu_session_id_t, e1ap_pdu_session_resource_modified_item>&
        e1ap_pdu_session_resource_modify_list)
{
  for (const auto& e1ap_item : e1ap_pdu_session_resource_modify_list) {
    cu_cp_pdu_session_resource_modify_response_item item;
    item.pdu_session_id = e1ap_item.pdu_session_id;

    for (const auto& e1ap_drb_item : e1ap_item.drb_modified_list_ng_ran) {
      // Catch implementation limitations.
      if (!e1ap_drb_item.flow_failed_list.empty()) {
        logger.warning("Non-empty QoS flow failed list not supported");
        return false;
      }

      // verify only a single UL transport info item is present.
      if (e1ap_drb_item.ul_up_transport_params.size() != 1) {
        logger.warning("Multiple UL UP transport items not supported");
        return false;
      }

      item.transfer.qos_flow_add_or_modify_response_list.emplace();

      // re-establish old flows
      const up_drb_context& drb_up_context = up_resource_mng.get_drb_context(e1ap_drb_item.drb_id);

      for (const auto& flow : drb_up_context.qos_flows) {
        qos_flow_add_or_mod_response_item qos_flow;
        qos_flow.qos_flow_id = flow.first;
        item.transfer.qos_flow_add_or_modify_response_list.value().emplace(qos_flow.qos_flow_id, qos_flow);
      }

      // Fill UE context modification for DU
      {
        f1ap_drb_to_modify drb_modified_item;
        drb_modified_item.drb_id = e1ap_drb_item.drb_id;

        // Add up tnl info
        for (const auto& ul_up_transport_param : e1ap_drb_item.ul_up_transport_params) {
          drb_modified_item.uluptnl_info_list.push_back(ul_up_transport_param.up_tnl_info);
        }

        ue_context_mod_req.drbs_to_be_modified_list.push_back(drb_modified_item);
      }
    }

    // Fail on any DRB that fails to be setup
    if (!e1ap_item.drb_failed_list_ng_ran.empty()) {
      logger.warning("Non-empty DRB failed list not supported");
      return false;
    }
  }

  return true;
}

bool reestablishment_context_modification_routine::generate_bearer_context_modification(
    e1ap_bearer_context_modification_request&        bearer_ctxt_mod_req,
    const e1ap_bearer_context_modification_response& bearer_ctxt_mod_resp,
    const f1ap_ue_context_modification_response&     ue_context_modification_resp,
    up_resource_manager&                             up_resource_manager,
    bool                                             reestablish_pdcp)
{
  // Fail procedure if (single) DRB couldn't be setup
  if (!ue_context_modification_resp.drbs_failed_to_be_setup_list.empty()) {
    logger.warning("Couldn't setup {} DRBs at DU", ue_context_modification_resp.drbs_failed_to_be_setup_list.size());
    return false;
  }

  // Fail procedure if (single) DRB couldn't be modified.
  if (!ue_context_modification_resp.drbs_failed_to_be_modified_list.empty()) {
    logger.warning("Couldn't modify {} DRBs at DU",
                   ue_context_modification_resp.drbs_failed_to_be_modified_list.size());
    return false;
  }

  // Fill security info
  bearer_ctxt_mod_req.security_info.emplace();
  bearer_ctxt_mod_req.security_info->security_algorithm.ciphering_algo                 = security_cfg.cipher_algo;
  bearer_ctxt_mod_req.security_info->security_algorithm.integrity_protection_algorithm = security_cfg.integ_algo;
  auto k_enc_buffer = byte_buffer::create(security_cfg.k_enc);
  if (not k_enc_buffer.has_value()) {
    logger.warning("Unable to allocate byte_buffer");
    return false;
  }
  bearer_ctxt_mod_req.security_info->up_security_key.encryption_key = std::move(k_enc_buffer.value());
  if (security_cfg.k_int.has_value()) {
    auto k_int_buffer = byte_buffer::create(security_cfg.k_int.value());
    if (not k_int_buffer.has_value()) {
      logger.warning("Unable to allocate byte_buffer");
      return false;
    }
    bearer_ctxt_mod_req.security_info->up_security_key.integrity_protection_key = std::move(k_int_buffer.value());
  }

  // Fill NG-RAN specific info.
  e1ap_ng_ran_bearer_context_mod_request& e1ap_bearer_context_mod =
      bearer_ctxt_mod_req.ng_ran_bearer_context_mod_request.emplace();

  /// Iterate over all PDU sessions to be updated and match the containing DRBs.
  for (const auto& pdu_session : bearer_ctxt_mod_resp.pdu_session_resource_modified_list) {
    // The modifications are only for this PDU session.
    e1ap_pdu_session_res_to_modify_item e1ap_mod_item;
    e1ap_mod_item.pdu_session_id = pdu_session.pdu_session_id;

    for (const auto& drb_item : ue_context_modification_resp.drbs_modified_list) {
      // Only include the DRB if it belongs to the this session.
      if (pdu_session.drb_modified_list_ng_ran.contains(drb_item.drb_id)) {
        // DRB belongs to this PDU session
        e1ap_drb_to_modify_item_ng_ran e1ap_drb_item;
        e1ap_drb_item.drb_id = drb_item.drb_id;

        for (const auto& dl_up_tnl_info : drb_item.dluptnl_info_list) {
          e1ap_up_params_item e1ap_dl_up_param;
          e1ap_dl_up_param.up_tnl_info   = dl_up_tnl_info;
          e1ap_dl_up_param.cell_group_id = 0; // TODO: Remove hardcoded value

          e1ap_drb_item.dl_up_params.push_back(e1ap_dl_up_param);
        }

        if (reestablish_pdcp) {
          const up_drb_context& drb_ctxt = up_resource_manager.get_drb_context(drb_item.drb_id);
          e1ap_drb_item.pdcp_cfg.emplace();
          fill_e1ap_drb_pdcp_config(e1ap_drb_item.pdcp_cfg.value(), drb_ctxt.pdcp_cfg);
          e1ap_drb_item.pdcp_cfg->pdcp_reest = true;
        }
        e1ap_mod_item.drb_to_modify_list_ng_ran.emplace(drb_item.drb_id, e1ap_drb_item);
      }
    }
    e1ap_bearer_context_mod.pdu_session_res_to_modify_list.emplace(e1ap_mod_item.pdu_session_id, e1ap_mod_item);
  }

  return true;
}
