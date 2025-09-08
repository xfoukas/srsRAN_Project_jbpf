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

#include "../ngap_error_indication_helper.h"
#include "../ue_context/ngap_ue_context.h"
#include "srsran/ngap/ngap.h"
#include "srsran/support/async/async_task.h"

namespace srsran {
namespace srs_cu_cp {

class ngap_ue_context_release_procedure
{
public:
  ngap_ue_context_release_procedure(
      const cu_cp_ue_context_release_command&                     command_,
      const ngap_ue_ids&                                          ue_ids_,
      std::unordered_map<ue_index_t, error_indication_request_t>& stored_error_indications_,
      ngap_cu_cp_notifier&                                        cu_cp_notifier_,
      ngap_message_notifier&                                      amf_notifier_,
      ngap_ue_logger&                                             logger_);

  void operator()(coro_context<async_task<void>>& ctx);

  static const char* name() { return "UE Context Release Procedure"; }

private:
  // Result senders.
  bool send_ue_context_release_complete();

  cu_cp_ue_context_release_command                            command;
  const ngap_ue_ids                                           ue_ids;
  std::unordered_map<ue_index_t, error_indication_request_t>& stored_error_indications;
  cu_cp_ue_context_release_complete                           ue_context_release_complete;
  ngap_cu_cp_notifier&                                        cu_cp_notifier;
  ngap_message_notifier&                                      amf_notifier;
  ngap_ue_logger                                              logger;
};

} // namespace srs_cu_cp
} // namespace srsran
