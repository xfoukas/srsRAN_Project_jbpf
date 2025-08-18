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

#include "bearer_context_modification_procedure.h"
#include "../e1ap_cu_cp_asn1_helpers.h"
#include "cu_cp/ue_context/e1ap_bearer_transaction_manager.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::e1ap;

bearer_context_modification_procedure::bearer_context_modification_procedure(const e1ap_configuration&        e1ap_cfg_,
                                                                             const e1ap_message&              request_,
                                                                             e1ap_bearer_transaction_manager& ev_mng_,
                                                                             e1ap_message_notifier& e1ap_notif_,
                                                                             e1ap_ue_logger&        logger_) :
  e1ap_cfg(e1ap_cfg_), request(request_), ev_mng(ev_mng_), e1ap_notifier(e1ap_notif_), logger(logger_)
{
}

void bearer_context_modification_procedure::operator()(
    coro_context<async_task<e1ap_bearer_context_modification_response>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.log_debug("\"{}\" initialized", name());

  // Subscribe to respective publisher to receive BEARER CONTEXT MODIFICATION RESPONSE/FAILURE message.
  transaction_sink.subscribe_to(ev_mng.context_modification_outcome, e1ap_cfg.proc_timeout);

  // Send command to CU-UP.
  send_bearer_context_modification_request();

  // Await response.
  CORO_AWAIT(transaction_sink);

  // Handle response from CU-UP and return UE index.
  CORO_RETURN(create_bearer_context_modification_result());
}

void bearer_context_modification_procedure::send_bearer_context_modification_request()
{
  // Send UE context modification request message.
  e1ap_notifier.on_new_message(request);
}

e1ap_bearer_context_modification_response
bearer_context_modification_procedure::create_bearer_context_modification_result()
{
  e1ap_bearer_context_modification_response res{};

  if (transaction_sink.successful()) {
    const asn1::e1ap::bearer_context_mod_resp_s& resp = transaction_sink.response();
    fill_e1ap_bearer_context_modification_response(res, resp);

    logger.log_debug("\"{}\" finalized", name());
  } else if (transaction_sink.failed()) {
    const asn1::e1ap::bearer_context_mod_fail_s& fail = transaction_sink.failure();
    fill_e1ap_bearer_context_modification_response(res, fail);

    logger.log_warning("\"{}\" failed", name());
  } else {
    res.success = false;

    if (transaction_sink.timeout_expired()) {
      logger.log_warning("BearerContextModificationResponse timeout");
    } else {
      logger.log_warning("BearerContextModificationResponse cancelled");
    }
  }
  return res;
}
