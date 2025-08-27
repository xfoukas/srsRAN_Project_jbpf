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

#include "../ngap_validators/ngap_validators.h"
#include "../ue_context/ngap_ue_context.h"
#include "srsran/ngap/ngap.h"
#include "srsran/support/async/async_task.h"

namespace srsran {
namespace srs_cu_cp {

class ngap_pdu_session_resource_modify_procedure
{
public:
  ngap_pdu_session_resource_modify_procedure(const cu_cp_pdu_session_resource_modify_request&    request_,
                                             const asn1::ngap::pdu_session_res_modify_request_s& asn1_request_,
                                             const ngap_ue_ids&                                  ue_ids_,
                                             ngap_cu_cp_notifier&                                cu_cp_notifier_,
                                             ngap_message_notifier&                              amf_notifier_,
                                             ngap_control_message_handler&                       ngap_ctrl_handler_,
                                             ngap_ue_logger&                                     logger_);

  void operator()(coro_context<async_task<void>>& ctx);

  static const char* name() { return "PDU Session Resource Modify Procedure"; }

private:
  void combine_pdu_session_resource_modify_response();

  // Result senders.
  bool send_pdu_session_resource_modify_response();

  const cu_cp_pdu_session_resource_modify_request    request;
  const asn1::ngap::pdu_session_res_modify_request_s asn1_request;
  const ngap_ue_ids                                  ue_ids;
  cu_cp_pdu_session_resource_modify_response         response;
  ngap_cu_cp_notifier&                               cu_cp_notifier;
  ngap_message_notifier&                             amf_notifier;
  ngap_control_message_handler&                      ngap_ctrl_handler;
  ngap_ue_logger&                                    logger;

  cu_cp_ue_context_release_request ue_context_release_request;

  // Procedure outcomes.
  pdu_session_resource_modify_validation_outcome verification_outcome;
};

} // namespace srs_cu_cp
} // namespace srsran
