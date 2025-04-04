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

#include "rrc_ue_capability_transfer_procedure.h"
#include "../rrc_asn1_helpers.h"


#ifdef JBPF_ENABLED
#include "jbpf_srsran_hooks.h"
#endif

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::rrc_nr;

rrc_ue_capability_transfer_procedure::rrc_ue_capability_transfer_procedure(
    rrc_ue_context_t&                           context_,
    rrc_ue_security_mode_command_proc_notifier& rrc_ue_notifier_,
    rrc_ue_event_manager&                       event_mng_,
    rrc_ue_logger&                              logger_) :
  context(context_), rrc_ue(rrc_ue_notifier_), event_mng(event_mng_), logger(logger_)
{
}

void rrc_ue_capability_transfer_procedure::operator()(coro_context<async_task<bool>>& ctx)
{
  CORO_BEGIN(ctx);

#ifdef JBPF_ENABLED 
  {
    struct jbpf_rrc_ctx_info ctx_info = {0, (uint64_t)context.ue_index};
    hook_rrc_ue_procedure_started(&ctx_info, RRC_UE_CAPABILITY, 0);
  }
#endif

  if (context.capabilities_list.has_value()) {
    logger.log_debug("\"{}\" skipped. Capabilities already present", name());

#ifdef JBPF_ENABLED 
    {
      struct jbpf_rrc_ctx_info ctx_info = {0, (uint64_t)context.ue_index};
      hook_rrc_ue_procedure_completed(&ctx_info, RRC_UE_CAPABILITY, false, 0);
    }
#endif
  
    CORO_EARLY_RETURN(true);
  }

  logger.log_debug("\"{}\" initialized", name());
  // create new transaction for RRCUeCapabilityEnquiry
  transaction =
      event_mng.transactions.create_transaction(std::chrono::milliseconds(context.cfg.rrc_procedure_timeout_ms));

  // send RRC UE Capability Enquiry to UE
  send_rrc_ue_capability_enquiry();

  // Await UE response
  CORO_AWAIT(transaction);

  if (transaction.has_response()) {
    if (transaction.response().msg.c1().ue_cap_info().crit_exts.ue_cap_info().ue_cap_rat_container_list_present) {
      context.capabilities_list.emplace(
          transaction.response().msg.c1().ue_cap_info().crit_exts.ue_cap_info().ue_cap_rat_container_list);
      for (const auto& ue_cap_rat_container :
           transaction.response().msg.c1().ue_cap_info().crit_exts.ue_cap_info().ue_cap_rat_container_list) {
        if (ue_cap_rat_container.rat_type.value == asn1::rrc_nr::rat_type_e::nr) {
          asn1::cbit_ref            bref{ue_cap_rat_container.ue_cap_rat_container.copy()};
          asn1::rrc_nr::ue_nr_cap_s ue_nr_cap;
          if (ue_nr_cap.unpack(bref) != asn1::SRSASN_SUCCESS) {
            logger.log_error("Error unpacking UE Capabilities");
            continue;
          }

          if (logger.get_basic_logger().debug.enabled()) {
            asn1::json_writer json_writer;
            ue_nr_cap.to_json(json_writer);
            logger.log_debug("UE Capabilities:\n{}", json_writer.to_string().c_str());
          }

        } else {
          logger.log_warning("Unsupported RAT type {}", ue_cap_rat_container.rat_type);
        }
      }
    }
    procedure_result = context.capabilities_list.has_value();
  } else {
    logger.log_warning("\"{}\" timed out after {}ms", name(), context.cfg.rrc_procedure_timeout_ms.count());
  }

#ifdef JBPF_ENABLED 
  {
    struct jbpf_rrc_ctx_info ctx_info = {0, (uint64_t)context.ue_index};
    hook_rrc_ue_procedure_completed(&ctx_info, RRC_UE_CAPABILITY, procedure_result, 0);
  }
#endif

  logger.log_debug("\"{}\" finalized", name());
  CORO_RETURN(procedure_result);
}

void rrc_ue_capability_transfer_procedure::send_rrc_ue_capability_enquiry()
{
  dl_dcch_msg_s dl_dcch_msg;
  dl_dcch_msg.msg.set_c1().set_ue_cap_enquiry();
  ue_cap_enquiry_s& rrc_ue_cap_enquiry = dl_dcch_msg.msg.c1().set_ue_cap_enquiry();
  fill_asn1_rrc_ue_capability_enquiry(rrc_ue_cap_enquiry, transaction.id(), context.cell.bands);
  rrc_ue.on_new_dl_dcch(srb_id_t::srb1, dl_dcch_msg);
}
