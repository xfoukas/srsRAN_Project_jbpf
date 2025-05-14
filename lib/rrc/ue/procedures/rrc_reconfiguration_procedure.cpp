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

#include "rrc_reconfiguration_procedure.h"
#include "../rrc_asn1_helpers.h"
#include "srsran/ran/cause/ngap_cause.h"

#ifdef JBPF_ENABLED
#include "jbpf_srsran_hooks.h"
#endif

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::rrc_nr;

rrc_reconfiguration_procedure::rrc_reconfiguration_procedure(rrc_ue_context_t&                            context_,
                                                             const rrc_reconfiguration_procedure_request& args_,
                                                             rrc_ue_reconfiguration_proc_notifier& rrc_ue_notifier_,
                                                             rrc_ue_event_manager&                 event_mng_,
                                                             rrc_ue_control_message_handler&       srb_notifier_,
                                                             rrc_ue_logger&                        logger_) :
  context(context_),
  args(args_),
  rrc_ue(rrc_ue_notifier_),
  event_mng(event_mng_),
  srb_notifier(srb_notifier_),
  logger(logger_)
{
}

void rrc_reconfiguration_procedure::operator()(coro_context<async_task<bool>>& ctx)
{
  CORO_BEGIN(ctx);

#ifdef JBPF_ENABLED 
  {
    struct jbpf_rrc_ctx_info ctx_info = {0, (uint64_t)context.ue_index};
    printf("MJB hook_rrc_ue_procedure_started: ue_index %ld, RRC_RECONFIGURATION \n", (uint64_t)context.ue_index);
    hook_rrc_ue_procedure_started(&ctx_info, RRC_RECONFIGURATION, 0);
  }
#endif

  if (context.state != rrc_state::connected) {

#ifdef JBPF_ENABLED 
    {
      struct jbpf_rrc_ctx_info ctx_info = {0, (uint64_t)context.ue_index};
      printf("MJB hook_rrc_ue_procedure_completed: ue_index %ld, RRC_RECONFIGURATION \n", (uint64_t)context.ue_index);
      hook_rrc_ue_procedure_completed(&ctx_info, RRC_RECONFIGURATION, false, 0);
    }
#endif

    logger.log_error("\"{}\" failed. UE is not RRC connected", name());
    CORO_EARLY_RETURN(false);
  }

  logger.log_debug("\"{}\" initialized", name());
  // create new transaction for RRC Reconfiguration procedure
  transaction =
      event_mng.transactions.create_transaction(std::chrono::milliseconds(context.cfg.rrc_procedure_timeout_ms));

  if (args.radio_bearer_cfg.has_value()) {
    for (const rrc_srb_to_add_mod& srb_to_add_mod : args.radio_bearer_cfg->srb_to_add_mod_list) {
      srb_creation_message srb = {};
      srb.ue_index             = context.ue_index;
      srb.srb_id               = srb_to_add_mod.srb_id;
      srb_notifier.create_srb(srb);
    }
  }

  send_rrc_reconfiguration();
  CORO_AWAIT(transaction);

  if (transaction.has_response()) {
    logger.log_debug("\"{}\" finished successfully", name());
    procedure_result = true;
  } else {
    logger.log_warning("\"{}\" timed out after {}ms", name(), context.cfg.rrc_procedure_timeout_ms.count());
  }

#ifdef JBPF_ENABLED 
  {
    struct jbpf_rrc_ctx_info ctx_info = {0, (uint64_t)context.ue_index};
    printf("MJB hook_rrc_ue_procedure_completed: ue_index %ld, RRC_RECONFIGURATION \n", (uint64_t)context.ue_index);
    hook_rrc_ue_procedure_completed(&ctx_info, RRC_RECONFIGURATION, procedure_result, 0);
  }
#endif

  CORO_RETURN(procedure_result);
}

void rrc_reconfiguration_procedure::send_rrc_reconfiguration()
{
  dl_dcch_msg_s dl_dcch_msg;
  dl_dcch_msg.msg.set_c1().set_rrc_recfg();
  rrc_recfg_s& rrc_reconfig = dl_dcch_msg.msg.c1().rrc_recfg();
  fill_asn1_rrc_reconfiguration_msg(rrc_reconfig, transaction.id(), args);
  rrc_ue.on_new_dl_dcch(srb_id_t::srb1, dl_dcch_msg);
}
