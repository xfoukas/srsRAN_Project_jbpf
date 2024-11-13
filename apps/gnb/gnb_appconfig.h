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

#pragma once

#include "apps/services/buffer_pool/buffer_pool_appconfig.h"
#include "apps/services/e2/e2_appconfig.h"
#include "apps/services/logger/logger_appconfig.h"
#include "apps/services/os_sched_affinity_manager.h"
#include "srsran/ran/gnb_id.h"
#include "srsran/support/executors/unique_thread.h"
#include <string>

#ifdef JBPF_ENABLED
#include "jbpf.h"
//#include "jbpf_lcm_ipc.h"
#endif

namespace srsran {

struct cu_up_appconfig {
  unsigned gtpu_queue_size          = 2048;
  unsigned gtpu_reordering_timer_ms = 0;
  bool     warn_on_drop             = false;
};

/// Metrics report configuration.
struct metrics_appconfig {
  std::string addr = "127.0.0.1";
  uint16_t    port = 55555;
};

/// CPU affinities configuration for the gNB app.
struct cpu_affinities_appconfig {
  /// CPUs isolation.
  std::optional<os_sched_affinity_bitmask> isolated_cpus;
  /// Low priority workers CPU affinity mask.
  os_sched_affinity_config low_priority_cpu_cfg = {sched_affinity_mask_types::low_priority,
                                                   {},
                                                   sched_affinity_mask_policy::mask};
};

/// Non real time thread configuration for the gNB.
struct non_rt_threads_appconfig {
  /// Number of non real time threads for processing of CP and UP data in the upper layers
  unsigned nof_non_rt_threads = 4;
};

/// Expert threads configuration of the gNB app.
struct expert_threads_appconfig {
  /// Non real time thread configuration of the gNB app.
  non_rt_threads_appconfig non_rt_threads;
};

/// Expert configuration of the gNB app.
struct expert_execution_appconfig {
  /// gNB CPU affinities.
  cpu_affinities_appconfig affinities;
  /// Expert thread configuration of the gNB app.
  expert_threads_appconfig threads;
};

/// HAL configuration of the gNB app.
struct hal_appconfig {
  /// EAL configuration arguments.
  std::string eal_args;
};

#ifdef JBPF_ENABLED
/// Janus configuration
struct jbpf_appconfig {
  /// jbpf run path
  std::string jbpf_run_path = JBPF_DEFAULT_RUN_PATH;
  /// jbpf namespace
  std::string jbpf_namespace = JBPF_DEFAULT_NAMESPACE;
  /// jbpf mode (0 standalone, 1 ipc)
  unsigned jbpf_ipc_enabled = 0;
  /// LCM IPC thread for codelet lifecycle management
  bool jbpf_has_lcm_ipc_thread = true;
  /// LCM IPC socket name
  std::string jbpf_lcm_ipc_name = JBPF_DEFAULT_LCM_SOCKET;
  /// jbpf ipc memory size
  size_t jbpf_ipc_mem_size = JBPF_HUGEPAGE_SIZE_1GB;
  /// jbpf ipc memory_name
  std::string jbpf_ipc_mem_name = "srsran_ipc_app";
  /// jbpf standalone output IP
  std::string jbpf_standalone_io_out_ip = "127.0.0.1";
  /// jbpf standalone destination UDP port to stream the data
  unsigned jbpf_standalone_io_out_port = 20788;
  /// jbpf standalone UDP Port to receive control inputs
  unsigned jbpf_standalone_io_in_port = 1924;
  /// Standalone IO thread CPU id
  unsigned jbpf_standalone_io_cpu = 0;
  /// Standalone IO thread CPU policy
  unsigned jbpf_standalone_io_policy = 0;
  /// Standalone IO thread CPU priority
  unsigned jbpf_standalone_io_priority = 0;
  /// Requested IO memory size
  unsigned jbpf_io_mem_size_mb = 1024;
  /// Agent thread CPU id
  unsigned jbpf_agent_cpu = 0;
  /// Agent thread CPU policy
  unsigned jbpf_agent_policy = 0;
  /// Agent thread CPU priority
  unsigned jbpf_agent_priority = 0;
  /// Maintenance thread CPU id
  unsigned jbpf_maint_cpu = 0;
  /// Maintenance thread CPU policy
  unsigned jbpf_maint_policy = 0;
  /// Maintenance thread CPU priority
  unsigned jbpf_maint_priority = 0;
};
#endif

/// Monolithic gnb application configuration.
struct gnb_appconfig {
  /// Default constructor to update the log filename.
  gnb_appconfig() { log_cfg.filename = "/tmp/gnb.log"; }
  /// Loggers configuration.
  logger_appconfig log_cfg;
  /// Metrics configuration.
  metrics_appconfig metrics_cfg;
  /// gNodeB identifier.
  gnb_id_t gnb_id = {411, 22};
  /// Node name.
  std::string ran_node_name = "srsgnb01";
  /// E2 configuration.
  e2_appconfig e2_cfg;
  /// Buffer pool configuration.
  buffer_pool_appconfig buffer_pool_config;
  /// Expert configuration.
  expert_execution_appconfig expert_execution_cfg;
#ifdef JBPF_ENABLED
  /// jbpf configuration.
  jbpf_appconfig jbpf_cfg;
#endif
  /// HAL configuration.
  std::optional<hal_appconfig> hal_config;
};

} // namespace srsran
