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

#include "ue_delete_procedure.h"
#include "proc_logger.h"

using namespace srsran;

void mac_ue_delete_procedure::operator()(coro_context<async_task<mac_ue_delete_response>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("{}: started...", mac_log_prefix(req.ue_index, req.rnti, name()));

  // > Remove UE from scheduler.
  // Note: Removing the UE from the scheduler before the MAC avoids potential race conditions (assuming the scheduler
  // doesn't allocate UEs after being removed).
  CORO_AWAIT(sched_configurator.handle_ue_removal_request(req));

  // > Remove UE and associated DL channels from the MAC DL.
  CORO_AWAIT(dl_mac.remove_ue(req));

  // > Remove UE associated UL channels from the MAC UL.
  CORO_AWAIT(ul_mac.remove_ue(req));

  // > Enqueue UE deletion
  ctrl_mac.remove_ue(req.ue_index);

  logger.info("{}: finished successfully", mac_log_prefix(req.ue_index, req.rnti, name()));

  // 4. Signal end of procedure and pass response
  CORO_RETURN(mac_ue_delete_response{true});
}
