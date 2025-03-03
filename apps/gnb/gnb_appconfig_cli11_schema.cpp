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

#include "gnb_appconfig_cli11_schema.h"
#include "apps/services/buffer_pool/buffer_pool_appconfig_cli11_schema.h"
#include "apps/services/logger/logger_appconfig_cli11_schema.h"
#include "gnb_appconfig.h"
#include "srsran/adt/interval.h"
#include "srsran/support/cli11_utils.h"
#include "srsran/support/error_handling.h"
#include "CLI/CLI11.hpp"

using namespace srsran;

template <typename Integer>
static expected<Integer, std::string> parse_int(const std::string& value)
{
  try {
    return std::stoi(value);
  } catch (const std::invalid_argument& e) {
    return make_unexpected(e.what());
  } catch (const std::out_of_range& e) {
    return make_unexpected(e.what());
  }
}

static void configure_cli11_metrics_args(CLI::App& app, metrics_appconfig& metrics_params)
{
  app.add_option("--addr", metrics_params.addr, "Metrics address.")->capture_default_str();
  app.add_option("--port", metrics_params.port, "Metrics UDP port.")
      ->capture_default_str()
      ->check(CLI::Range(0, 65535));
}

static void configure_cli11_e2_args(CLI::App& app, e2_appconfig& e2_params)
{
  add_option(app, "--enable_du_e2", e2_params.enable_du_e2, "Enable DU E2 agent")->capture_default_str();
  add_option(app, "--enable_cu_e2", e2_params.enable_cu_e2, "Enable CU E2 agent")->capture_default_str();
  add_option(app, "--addr", e2_params.ip_addr, "RIC IP address")->capture_default_str();
  add_option(app, "--port", e2_params.port, "RIC port")->check(CLI::Range(20000, 40000))->capture_default_str();
  add_option(app, "--bind_addr", e2_params.bind_addr, "Local IP address to bind for RIC connection")
      ->capture_default_str()
      ->check(CLI::ValidIPV4);
  add_option(app, "--sctp_rto_initial", e2_params.sctp_rto_initial, "SCTP initial RTO value")->capture_default_str();
  add_option(app, "--sctp_rto_min", e2_params.sctp_rto_min, "SCTP RTO min")->capture_default_str();
  add_option(app, "--sctp_rto_max", e2_params.sctp_rto_max, "SCTP RTO max")->capture_default_str();
  add_option(app, "--sctp_init_max_attempts", e2_params.sctp_init_max_attempts, "SCTP init max attempts")
      ->capture_default_str();
  add_option(app, "--sctp_max_init_timeo", e2_params.sctp_max_init_timeo, "SCTP max init timeout")
      ->capture_default_str();
  add_option(app, "--e2sm_kpm_enabled", e2_params.e2sm_kpm_enabled, "Enable KPM service module")->capture_default_str();
  add_option(app, "--e2sm_rc_enabled", e2_params.e2sm_rc_enabled, "Enable RC service module")->capture_default_str();
}

#ifdef JBPF_ENABLED

static void configure_cli11_jbpf_args(CLI::App& app, jbpf_appconfig& config)
{
  app.add_option("--jbpf_enable_ipc", config.jbpf_ipc_enabled, "jbpf mode [0: Standalone mode 1: IPC mode]")
      ->capture_default_str()->check(CLI::Range(0,1));
  app.add_option("--jbpf_standalone_io_out_ip", config.jbpf_standalone_io_out_ip, "Standalone mode IO output IP")
      ->capture_default_str();
  app.add_option("--jbpf_standalone_io_out_port", config.jbpf_standalone_io_out_port, "Standalone mode IO out port")
      ->capture_default_str()->check(CLI::Range(0, 65535));
  app.add_option("--jbpf_standalone_io_in_port", config.jbpf_standalone_io_in_port, "Standalone mode IO in port")
      ->capture_default_str()->check(CLI::Range(0, 65535));
  app.add_option("--jbpf_standalone_io_cpu", config.jbpf_standalone_io_cpu, "Standalone mode IO thread CPU id")
      ->capture_default_str();
  app.add_option("--jbpf_standalone_io_policy", config.jbpf_standalone_io_policy, 
      "Standalone IO thread CPU scheduling policy [0 : SCHED_OTHER, 1 : SCHED_FIFO]")
      ->capture_default_str()->check(CLI::Range(0, 1));
  app.add_option("--jbpf_standalone_io_priority", config.jbpf_standalone_io_priority, 
      "Standalone mode IO thread CPU scheduling priority")
      ->capture_default_str()->check(CLI::Range(0, 99));
  app.add_option("--jbpf_io_mem_size_mb", config.jbpf_io_mem_size_mb, "jbpf IO memory size in MBs")
      ->capture_default_str();
  app.add_option("--jbpf_ipc_mem_name", config.jbpf_ipc_mem_name, "IPC mode memory name")
      ->capture_default_str();
  app.add_option("--jbpf_ipc_mem_size", config.jbpf_ipc_mem_size, "IPC mode memory size")
      ->capture_default_str();  
  app.add_option("--jbpf_enable_lcm_ipc", config.jbpf_has_lcm_ipc_thread, "Enable LCM IPC interface")
      ->capture_default_str()->check(CLI::Range(0, 1));
  app.add_option("--jbpf_lcm_ipc_name", config.jbpf_lcm_ipc_name, "LCM IPC socket name")
      ->capture_default_str();
  app.add_option("--jbpf_run_path", config.jbpf_run_path, "jbpf run path")
      ->capture_default_str();
  app.add_option("--jbpf_namespace", config.jbpf_namespace, "jbpf namespace")
      ->capture_default_str();
  app.add_option("--jbpf_agent_cpu", config.jbpf_agent_cpu, "Agent thread CPU id")
      ->capture_default_str();
  app.add_option("--jbpf_agent_policy", config.jbpf_agent_policy, 
      "Agent thread CPU scheduling policy [0 : SCHED_OTHER, 1 : SCHED_FIFO]")
      ->capture_default_str()->check(CLI::Range(0, 1));
  app.add_option("--jbpf_agent_priority", config.jbpf_agent_priority, 
      "Agent thread CPU scheduling priority")
      ->capture_default_str()->check(CLI::Range(0, 99));
  app.add_option("--jbpf_maint_cpu", config.jbpf_maint_cpu, "Maintenance thread CPU id")
      ->capture_default_str();
  app.add_option("--jbpf_maint_policy", config.jbpf_maint_policy, 
      "Maintenance thread CPU scheduling policy [0 : SCHED_OTHER, 1 : SCHED_FIFO]")
      ->capture_default_str()->check(CLI::Range(0, 1));
  app.add_option("--jbpf_maint_priority", config.jbpf_maint_priority, 
      "Maintenance thread CPU scheduling priority")
      ->capture_default_str()->check(CLI::Range(0, 99));
}

#endif

static void configure_cli11_hal_args(CLI::App& app, std::optional<hal_appconfig>& config)
{
  config.emplace();

  add_option(app, "--eal_args", config->eal_args, "EAL configuration parameters used to initialize DPDK");
}

static error_type<std::string> is_valid_cpu_index(unsigned cpu_idx)
{
  std::string error_message = fmt::format("Invalid CPU core selected '{}'. Valid CPU ids: {}",
                                          cpu_idx,
                                          os_sched_affinity_bitmask::available_cpus().get_cpu_ids());

  os_sched_affinity_bitmask one_cpu_mask;
  if (cpu_idx >= one_cpu_mask.size()) {
    return make_unexpected(error_message);
  }
  one_cpu_mask.set(cpu_idx);
  if (not one_cpu_mask.subtract(os_sched_affinity_bitmask::available_cpus()).empty()) {
    return make_unexpected(error_message);
  }
  return default_success_t();
}

static expected<unsigned, std::string> parse_one_cpu(const std::string& value)
{
  expected<int, std::string> result = parse_int<int>(value);

  if (not result.has_value()) {
    return make_unexpected(fmt::format("Could not parse '{}' string as a CPU index", value));
  }

  error_type<std::string> validation_result = is_valid_cpu_index(result.value());
  if (not validation_result.has_value()) {
    return make_unexpected(validation_result.error());
  }

  return result.value();
}

static expected<interval<unsigned, true>, std::string> parse_cpu_range(const std::string& value)
{
  std::vector<unsigned> range;
  std::stringstream     ss(value);
  while (ss.good()) {
    std::string str;
    getline(ss, str, '-');
    auto parse_result = parse_one_cpu(str);
    if (not parse_result.has_value()) {
      return make_unexpected(fmt::format("{}. Could not parse '{}' as a range", parse_result.error(), value));
    }

    range.push_back(parse_result.value());
  }

  // A range is defined by two numbers.
  if (range.size() != 2) {
    return make_unexpected(fmt::format("Could not parse '{}' as a range", value));
  }

  if (range[1] <= range[0]) {
    return make_unexpected(fmt::format("Invalid CPU core range detected [{}-{}]", range[0], range[1]));
  }

  return interval<unsigned, true>(range[0], range[1]);
}

static void
parse_affinity_mask(os_sched_affinity_bitmask& mask, const std::string& value, const std::string& property_name)
{
  std::stringstream ss(value);

  while (ss.good()) {
    std::string str;
    getline(ss, str, ',');
    if (str.find('-') != std::string::npos) {
      auto range = parse_cpu_range(str);
      if (not range.has_value()) {
        report_error("{} in the '{}' property", range.error(), property_name);
      }

      // Add 1 to the stop value as the fill method excludes the end position.
      mask.fill(range.value().start(), range.value().stop() + 1);
    } else {
      auto cpu_idx = parse_one_cpu(str);
      if (not cpu_idx.has_value()) {
        report_error("{} in the '{}' property", cpu_idx.error(), property_name);
      }

      mask.set(cpu_idx.value());
    }
  }
}

static void configure_cli11_non_rt_threads_args(CLI::App& app, non_rt_threads_appconfig& config)
{
  add_option(app,
             "--nof_non_rt_threads",
             config.nof_non_rt_threads,
             "Number of non real time threads for processing of CP and UP data in upper layers.")
      ->capture_default_str()
      ->check(CLI::Number);
}

static void configure_cli11_cpu_affinities_args(CLI::App& app, cpu_affinities_appconfig& config)
{
  auto parsing_isolated_cpus_fcn = [](std::optional<os_sched_affinity_bitmask>& isolated_cpu_cfg,
                                      const std::string&                        value,
                                      const std::string&                        property_name) {
    isolated_cpu_cfg.emplace();
    parse_affinity_mask(*isolated_cpu_cfg, value, property_name);

    if (isolated_cpu_cfg->all()) {
      report_error("Error in '{}' property: can not assign all available CPUs to the gNB app", property_name);
    }
  };

  add_option_function<std::string>(
      app,
      "--isolated_cpus",
      [&config, &parsing_isolated_cpus_fcn](const std::string& value) {
        parsing_isolated_cpus_fcn(config.isolated_cpus, value, "isolated_cpus");
      },
      "CPU cores isolated for gNB application");

  add_option_function<std::string>(
      app,
      "--low_priority_cpus",
      [&config](const std::string& value) {
        parse_affinity_mask(config.low_priority_cpu_cfg.mask, value, "low_priority_cpus");
      },
      "CPU cores assigned to low priority tasks");

  add_option_function<std::string>(
      app,
      "--low_priority_pinning",
      [&config](const std::string& value) {
        config.low_priority_cpu_cfg.pinning_policy = to_affinity_mask_policy(value);
        if (config.low_priority_cpu_cfg.pinning_policy == sched_affinity_mask_policy::last) {
          report_error("Incorrect value={} used in {} property", value, "low_priority_pinning");
        }
      },
      "Policy used for assigning CPU cores to low priority tasks");
}

static void configure_cli11_expert_execution_args(CLI::App& app, expert_execution_appconfig& config)
{
  // Affinity section.
  CLI::App* affinities_subcmd = add_subcommand(app, "affinities", "gNB CPU affinities configuration")->configurable();
  configure_cli11_cpu_affinities_args(*affinities_subcmd, config.affinities);

  // Threads section.
  CLI::App* threads_subcmd = add_subcommand(app, "threads", "Threads configuration")->configurable();

  // Non real time threads.
  CLI::App* non_rt_threads_subcmd =
      add_subcommand(*threads_subcmd, "non_rt", "Non real time thread configuration")->configurable();
  configure_cli11_non_rt_threads_args(*non_rt_threads_subcmd, config.threads.non_rt_threads);
}

static void manage_hal_optional(CLI::App& app, gnb_appconfig& gnb_cfg)
{
  // Clean the HAL optional.
  if (auto subcmd = app.get_subcommand("hal"); subcmd->count_all() == 0) {
    gnb_cfg.hal_config.reset();
    // As HAL configuration is optional, disable the command when it is not present in the configuration.
    subcmd->disabled();
  }
}

void srsran::configure_cli11_with_gnb_appconfig_schema(CLI::App& app, gnb_appconfig& gnb_parsed_cfg)
{
  gnb_appconfig& gnb_cfg = gnb_parsed_cfg;
  add_option(app, "--gnb_id", gnb_cfg.gnb_id.id, "gNodeB identifier")->capture_default_str();
  add_option(app, "--gnb_id_bit_length", gnb_cfg.gnb_id.bit_length, "gNodeB identifier length in bits")
      ->capture_default_str()
      ->check(CLI::Range(22, 32));
  add_option(app, "--ran_node_name", gnb_cfg.ran_node_name, "RAN node name")->capture_default_str();

  // Loggers section.
  configure_cli11_with_logger_appconfig_schema(app, gnb_cfg.log_cfg);

  // Buffer pool section.
  configure_cli11_with_buffer_pool_appconfig_schema(app, gnb_cfg.buffer_pool_config);

  // Metrics section.
  CLI::App* metrics_subcmd = app.add_subcommand("metrics", "Metrics configuration")->configurable();
  configure_cli11_metrics_args(*metrics_subcmd, gnb_cfg.metrics_cfg);

  // E2 section.
  CLI::App* e2_subcmd = add_subcommand(app, "e2", "E2 parameters")->configurable();
  configure_cli11_e2_args(*e2_subcmd, gnb_cfg.e2_cfg);

  // Expert section.
  CLI::App* expert_subcmd = app.add_subcommand("expert_execution", "Expert execution configuration")->configurable();
  configure_cli11_expert_execution_args(*expert_subcmd, gnb_cfg.expert_execution_cfg);

#ifdef JBPF_ENABLED

  // jbpf section.
  CLI::App* jbpf_subcmd = app.add_subcommand("jbpf", "jbpf configuration")->configurable();
  configure_cli11_jbpf_args(*jbpf_subcmd, gnb_cfg.jbpf_cfg);

#endif

  // HAL section.
  CLI::App* hal_subcmd = add_subcommand(app, "hal", "HAL configuration")->configurable();
  configure_cli11_hal_args(*hal_subcmd, gnb_cfg.hal_config);
}

void srsran::autoderive_gnb_parameters_after_parsing(CLI::App& app, gnb_appconfig& config)
{
  manage_hal_optional(app, config);
}
