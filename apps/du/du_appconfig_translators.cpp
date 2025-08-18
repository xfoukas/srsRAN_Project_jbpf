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

#include "du_appconfig_translators.h"
#include "apps/services/worker_manager/worker_manager_config.h"
#include "du_appconfig.h"

using namespace srsran;
using namespace std::chrono_literals;

void srsran::fill_du_worker_manager_config(worker_manager_config& config, const du_appconfig& unit_cfg)
{
  srsran_assert(config.du_hi_cfg, "DU high worker config does not exist");

  config.nof_low_prio_threads     = unit_cfg.expert_execution_cfg.threads.non_rt_threads.nof_non_rt_threads;
  config.low_prio_task_queue_size = unit_cfg.expert_execution_cfg.threads.non_rt_threads.non_rt_task_queue_size;
  config.low_prio_sched_config    = unit_cfg.expert_execution_cfg.affinities.low_priority_cpu_cfg;
}
