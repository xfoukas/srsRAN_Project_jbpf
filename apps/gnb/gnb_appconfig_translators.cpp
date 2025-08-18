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

#include "gnb_appconfig_translators.h"
#include "apps/services/worker_manager/worker_manager_config.h"
#include "gnb_appconfig.h"

#ifdef JBPF_ENABLED
#include <iostream>
#include "jbpf_srsran_defs.h"
#endif

using namespace srsran;
using namespace std::chrono_literals;

#ifdef JBPF_ENABLED
void srsran::generate_jbpf_config(const gnb_appconfig& config, struct jbpf_config *jbpf_cfg)
{
  strncpy(jbpf_cfg->jbpf_run_path, config.jbpf_cfg.jbpf_run_path.c_str(), JBPF_RUN_PATH_LEN - 1);
  strncpy(jbpf_cfg->jbpf_namespace, config.jbpf_cfg.jbpf_namespace.c_str(), JBPF_NAMESPACE_LEN - 1);
  if (config.jbpf_cfg.jbpf_ipc_enabled == 0) {
    jbpf_cfg->io_config.io_type = JBPF_IO_THREAD_CONFIG;
    jbpf_cfg->io_config.io_thread_config.io_mem_size = config.jbpf_cfg.jbpf_io_mem_size_mb * 1024;
    if (config.jbpf_cfg.jbpf_standalone_io_cpu != 0) {
      jbpf_cfg->io_config.io_thread_config.has_affinity_io_thread = 1;
      jbpf_cfg->io_config.io_thread_config.io_thread_affinity_cores = config.jbpf_cfg.jbpf_standalone_io_cpu;
    }
    if (config.jbpf_cfg.jbpf_standalone_io_priority != 0) {
      jbpf_cfg->io_config.io_thread_config.has_sched_priority_io_thread = 1;
      jbpf_cfg->io_config.io_thread_config.io_thread_sched_priority = config.jbpf_cfg.jbpf_standalone_io_priority;
    }
    jbpf_cfg->io_config.io_thread_config.has_sched_policy_io_thread = 1;
    jbpf_cfg->io_config.io_thread_config.io_thread_sched_policy = config.jbpf_cfg.jbpf_standalone_io_policy;

    output_socket *sock = new output_socket();
    // Create a UDP socket to send messages out
    sock->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&sock->server_addr, 0, sizeof(sock->server_addr));
    sock->server_addr.sin_family = AF_INET;
    sock->server_addr.sin_port = htons(config.jbpf_cfg.jbpf_standalone_io_out_port);
    sock->server_addr.sin_addr.s_addr = inet_addr(config.jbpf_cfg.jbpf_standalone_io_out_ip.c_str());
    std::cout << "[JANUS_INFO] Creating UDP connection to " << config.jbpf_cfg.jbpf_standalone_io_out_ip.c_str() 
      << ":" << config.jbpf_cfg.jbpf_standalone_io_out_port << std::endl;
    jbpf_cfg->io_config.io_thread_config.output_handler_ctx = sock;
  } else if (config.jbpf_cfg.jbpf_ipc_enabled == 1) {
    jbpf_cfg->io_config.io_type = JBPF_IO_IPC_CONFIG;
    strncpy(jbpf_cfg->io_config.io_ipc_config.ipc_name, config.jbpf_cfg.jbpf_ipc_mem_name.c_str(), MAX_IPC_NAME_LEN - 1);
    jbpf_cfg->io_config.io_ipc_config.ipc_mem_size = config.jbpf_cfg.jbpf_io_mem_size_mb * 1024 * 1024;
  }
  
  if (config.jbpf_cfg.jbpf_has_lcm_ipc_thread) {
    strncpy(jbpf_cfg->lcm_ipc_config.lcm_ipc_name, config.jbpf_cfg.jbpf_lcm_ipc_name.c_str(), JBPF_LCM_IPC_NAME_LEN - 1);
  }
  
  if (config.jbpf_cfg.jbpf_agent_cpu != 0) {
    jbpf_cfg->thread_config.has_affinity_agent_thread = 1;
    jbpf_cfg->thread_config.agent_thread_affinity_cores = config.jbpf_cfg.jbpf_agent_cpu;
  }
  jbpf_cfg->thread_config.has_sched_policy_agent_thread = 1;
  jbpf_cfg->thread_config.agent_thread_sched_policy = config.jbpf_cfg.jbpf_agent_policy;
 
  if (config.jbpf_cfg.jbpf_agent_priority != 0) {
    jbpf_cfg->thread_config.has_sched_priority_agent_thread = 1;
    jbpf_cfg->thread_config.agent_thread_sched_priority = config.jbpf_cfg.jbpf_agent_priority;
  }
  if (config.jbpf_cfg.jbpf_maint_cpu != 0) {
    jbpf_cfg->thread_config.has_affinity_maintenance_thread = 1;
    jbpf_cfg->thread_config.maintenance_thread_affinity_cores = config.jbpf_cfg.jbpf_maint_cpu;
  }
  jbpf_cfg->thread_config.has_sched_policy_maintenance_thread = 1;
  jbpf_cfg->thread_config.maintenance_thread_sched_policy = config.jbpf_cfg.jbpf_maint_policy;
  if (config.jbpf_cfg.jbpf_maint_priority != 0) {
    jbpf_cfg->thread_config.has_sched_priority_maintenance_thread = 1;
    jbpf_cfg->thread_config.maintenance_thread_sched_priority = config.jbpf_cfg.jbpf_maint_priority;
  }
}
#endif

void srsran::fill_gnb_worker_manager_config(worker_manager_config& config, const gnb_appconfig& unit_cfg)
{
  srsran_assert(config.cu_up_cfg, "CU-UP worker config does not exist");
  srsran_assert(config.du_hi_cfg, "DU high worker config does not exist");

  config.nof_low_prio_threads     = unit_cfg.expert_execution_cfg.threads.non_rt_threads.nof_non_rt_threads;
  config.low_prio_task_queue_size = unit_cfg.expert_execution_cfg.threads.non_rt_threads.non_rt_task_queue_size;
  config.low_prio_sched_config    = unit_cfg.expert_execution_cfg.affinities.low_priority_cpu_cfg;
}
