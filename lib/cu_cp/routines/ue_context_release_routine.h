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

#include "../ue_manager/ue_manager_impl.h"
#include "srsran/support/async/async_task.h"
#include "srsran/support/async/eager_async_task.h"

namespace srsran {
namespace srs_cu_cp {

/// \brief Handles the setup of PDU session resources from the RRC viewpoint.
class ue_context_release_routine
{
public:
  ue_context_release_routine(const cu_cp_ue_context_release_command& command_,
                             e1ap_bearer_context_manager*            e1ap_bearer_ctxt_mng_,
                             f1ap_ue_context_manager&                f1ap_ue_ctxt_mng_,
                             cu_cp_ue_removal_handler&               ue_removal_handler_,
                             ue_manager&                             ue_mng_,
                             srslog::basic_logger&                   logger_);

  void operator()(coro_context<async_task<cu_cp_ue_context_release_complete>>& ctx);

  static const char* name() { return "UE Context Release Routine"; }

private:
  const cu_cp_ue_context_release_command command;

  e1ap_bearer_context_manager* e1ap_bearer_ctxt_mng = nullptr; // to trigger bearer context setup at CU-UP
  f1ap_ue_context_manager&     f1ap_ue_ctxt_mng;               // to trigger UE context modification at DU
  cu_cp_ue_removal_handler&    ue_removal_handler;             // to remove UE
  ue_manager&                  ue_mng;
  srslog::basic_logger&        logger;

  // (sub-)routine requests
  rrc_ue_release_context              release_context;
  f1ap_ue_context_release_command     f1ap_ue_context_release_cmd;
  e1ap_bearer_context_release_command bearer_context_release_command;

  // (sub-)routine results
  ue_index_t                        f1ap_ue_context_release_result;
  cu_cp_ue_context_release_complete release_complete;
};

} // namespace srs_cu_cp
} // namespace srsran
