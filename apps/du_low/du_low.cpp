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

#include "apps/helpers/metrics/metrics_helpers.h"
#include "apps/services/app_resource_usage/app_resource_usage.h"
#include "apps/services/application_message_banners.h"
#include "apps/services/application_tracer.h"
#include "apps/services/cmdline/cmdline_command_dispatcher.h"
#include "apps/services/core_isolation_manager.h"
#include "apps/services/metrics/metrics_manager.h"
#include "apps/services/metrics/metrics_notifier_proxy.h"
#include "apps/services/remote_control/remote_server.h"
#include "apps/services/worker_manager/worker_manager.h"
#include "apps/units/application_unit.h"
#include "apps/units/flexible_o_du/split_6/o_du_low/split6_o_du_low_application_unit_impl.h"
#include "du_low_appconfig.h"
#include "du_low_appconfig_cli11_schema.h"
#include "du_low_appconfig_translators.h"
#include "du_low_appconfig_validators.h"
#include "du_low_appconfig_yaml_writer.h"
#include "srsran/support/backtrace.h"
#include "srsran/support/config_parsers.h"
#include "srsran/support/cpu_features.h"
#include "srsran/support/io/io_broker_factory.h"
#include "srsran/support/signal_handling.h"
#include "srsran/support/signal_observer.h"
#include "srsran/support/tracing/event_tracing.h"
#include "srsran/support/versioning/build_info.h"
#include "srsran/support/versioning/version.h"
#include <atomic>
#ifdef DPDK_FOUND
#include "srsran/hal/dpdk/dpdk_eal_factory.h"
#endif
// Include ThreadSanitizer (TSAN) options if thread sanitization is enabled.
// This include is not unused - it helps prevent false alarms from the thread sanitizer.
#include "srsran/support/tsan_options.h"

using namespace srsran;

/// \file
/// \brief Application of a distributed unit (DU) that runs the low part (PHY) of the split 6.

static std::string config_file;

/// Flag that indicates if the application is running or being shutdown.
static std::atomic<bool> is_app_running = {true};
/// Maximum number of configuration files allowed to be concatenated in the command line.
static constexpr unsigned MAX_CONFIG_FILES = 10;

static void populate_cli11_generic_args(CLI::App& app)
{
  fmt::memory_buffer buffer;
  format_to(std::back_inserter(buffer), "srsRAN 5G DU low version {} ({})", get_version(), get_build_hash());
  app.set_version_flag("-v,--version", srsran::to_c_str(buffer));
  app.set_config("-c,", config_file, "Read config from file", false)->expected(1, MAX_CONFIG_FILES);
}

/// Function to call when the application is interrupted.
static void interrupt_signal_handler(int signal)
{
  is_app_running = false;
}

static signal_dispatcher cleanup_signal_dispatcher;

/// Function to call when the application is going to be forcefully shutdown.
static void cleanup_signal_handler(int signal)
{
  cleanup_signal_dispatcher.notify_signal(signal);
  srslog::flush();
}

/// Function to call when an error is reported by the application.
static void app_error_report_handler()
{
  srslog::flush();
}

static void initialize_log(const std::string& filename)
{
  srslog::sink* log_sink = (filename == "stdout") ? srslog::create_stdout_sink() : srslog::create_file_sink(filename);
  if (log_sink == nullptr) {
    report_error("Could not create application main log sink.\n");
  }
  srslog::set_default_sink(*log_sink);
  srslog::init();
}

static void register_app_logs(const du_low_appconfig& du_cfg, application_unit& du_low_app_unit)
{
  const logger_appconfig& log_cfg = du_cfg.log_cfg;
  // Set log-level of app and all non-layer specific components to app level.

  auto& logger = srslog::fetch_basic_logger("ALL", false);
  logger.set_level(log_cfg.lib_level);
  logger.set_hex_dump_max_size(log_cfg.hex_max_size);

  auto& app_logger = srslog::fetch_basic_logger("APP", false);
  app_logger.set_level(srslog::basic_levels::info);
  app_services::application_message_banners::log_build_info(app_logger);
  app_logger.set_level(log_cfg.all_level);
  app_logger.set_hex_dump_max_size(log_cfg.hex_max_size);

  auto& config_logger = srslog::fetch_basic_logger("CONFIG", false);
  config_logger.set_level(log_cfg.config_level);
  config_logger.set_hex_dump_max_size(log_cfg.hex_max_size);

  // Metrics log channels.
  const app_helpers::metrics_config& metrics_cfg = du_cfg.metrics_cfg.rusage_config.metrics_consumers_cfg;
  app_helpers::initialize_metrics_log_channels(metrics_cfg, log_cfg.hex_max_size);

  // Register units logs.
  du_low_app_unit.on_loggers_registration();
}

int main(int argc, char** argv)
{
  // Set the application error handler.
  set_error_handler(app_error_report_handler);

  static constexpr std::string_view app_name = "DU low";
  app_services::application_message_banners::announce_app_and_version(app_name);

  // Set interrupt and cleanup signal handlers.
  register_interrupt_signal_handler(interrupt_signal_handler);
  register_cleanup_signal_handler(cleanup_signal_handler);

  // Enable backtrace.
  enable_backtrace();

  // Setup and configure config parsing.
  CLI::App app("srsDU low application");
  app.config_formatter(create_yaml_config_parser());
  app.allow_config_extras(CLI::config_extras_mode::error);
  // Fill the generic application arguments to parse.
  populate_cli11_generic_args(app);

  du_low_appconfig du_low_cfg;
  // Configure CLI11 with the DU application configuration schema.
  configure_cli11_with_du_low_appconfig_schema(app, du_low_cfg);

  auto o_du_app_unit = create_flexible_o_du_low_application_unit(app_name);
  o_du_app_unit->on_parsing_configuration_registration(app);

  // Set the callback for the app calling all the autoderivation functions.
  app.callback([&app, &du_low_cfg, &o_du_app_unit]() {
    autoderive_du_low_parameters_after_parsing(app, du_low_cfg);
    o_du_app_unit->on_configuration_parameters_autoderivation(app);
  });

  // Parse arguments.
  CLI11_PARSE(app, argc, argv);

  // Dry run mode, exit.
  if (du_low_cfg.enable_dryrun) {
    return 0;
  }

  // Check the modified configuration.
  if (!validate_du_low_appconfig(du_low_cfg) ||
      !o_du_app_unit->on_configuration_validation((du_low_cfg.expert_execution_cfg.affinities.isolated_cpus)
                                                      ? du_low_cfg.expert_execution_cfg.affinities.isolated_cpus.value()
                                                      : os_sched_affinity_bitmask::available_cpus())) {
    report_error("Invalid configuration detected.\n");
  }

  // Set up logging.
  initialize_log(du_low_cfg.log_cfg.filename);
  register_app_logs(du_low_cfg, *o_du_app_unit);

  // Check the metrics and metrics consumers.
  srslog::basic_logger& app_logger = srslog::fetch_basic_logger("APP");
  bool metrics_enabled = o_du_app_unit->are_metrics_enabled() || du_low_cfg.metrics_cfg.rusage_config.enable_app_usage;

  if (!metrics_enabled && du_low_cfg.metrics_cfg.rusage_config.metrics_consumers_cfg.enabled()) {
    app_logger.warning("Logger or JSON metrics output enabled but no metrics will be reported as no layer was enabled");
    fmt::println("Logger or JSON metrics output enabled but no metrics will be reported as no layer was enabled");
  }

  // Log input configuration.
  srslog::basic_logger& config_logger = srslog::fetch_basic_logger("CONFIG");
  if (config_logger.debug.enabled()) {
    YAML::Node node;
    fill_du_low_appconfig_in_yaml_schema(node, du_low_cfg);
    o_du_app_unit->dump_config(node);
    config_logger.debug("Input configuration (all values): \n{}", YAML::Dump(node));
  } else {
    config_logger.info("Input configuration (only non-default values): \n{}", app.config_to_str(false, false));
  }

  app_services::application_tracer app_tracer;
  if (not du_low_cfg.log_cfg.tracing_filename.empty()) {
    app_tracer.enable_tracer(du_low_cfg.log_cfg.tracing_filename, app_logger);
  }

  app_services::core_isolation_manager core_isolation_mngr;
  if (du_low_cfg.expert_execution_cfg.affinities.isolated_cpus) {
    if (!core_isolation_mngr.isolate_cores(*du_low_cfg.expert_execution_cfg.affinities.isolated_cpus)) {
      report_error("Failed to isolate specified CPUs");
    }
  }

  // Log CPU architecture.
  cpu_architecture_info::get().print_cpu_info(app_logger);

  // Check and log included CPU features and check support by current CPU
  if (cpu_supports_included_features()) {
    app_logger.debug("Required CPU features: {}", get_cpu_feature_info());
  } else {
    // Quit here until we complete selection of the best matching implementation for the current CPU at runtime.
    app_logger.error("The CPU does not support the required CPU features that were configured during compile time: {}",
                     get_cpu_feature_info());
    report_error("The CPU does not support the required CPU features that were configured during compile time: {}\n",
                 get_cpu_feature_info());
  }

  // Check some common causes of performance issues and print a warning if required.
  check_cpu_governor(app_logger);
  check_drm_kms_polling(app_logger);

#ifdef DPDK_FOUND
  std::unique_ptr<dpdk::dpdk_eal> eal;
  if (du_low_cfg.hal_config) {
    // Prepend the application name in argv[0] as it is expected by EAL.
    eal = dpdk::create_dpdk_eal(std::string(argv[0]) + " " + du_low_cfg.hal_config->eal_args,
                                srslog::fetch_basic_logger("EAL", false));
  }
#endif

  // Create manager of timers for DU, which will be driven by the PHY slot ticks.
  timer_manager app_timers{256};

  // Instantiate worker manager.
  worker_manager_config worker_manager_cfg;
  fill_du_low_worker_manager_config(worker_manager_cfg, du_low_cfg);
  o_du_app_unit->fill_worker_manager_config(worker_manager_cfg);
  worker_manager_cfg.app_timers = &app_timers;

  worker_manager workers{worker_manager_cfg};

  // Set layer-specific pcap options.
  const auto& low_prio_cpu_mask = du_low_cfg.expert_execution_cfg.affinities.low_priority_cpu_cfg.mask;

  // Create IO broker.
  io_broker_config           io_broker_cfg(low_prio_cpu_mask);
  std::unique_ptr<io_broker> epoll_broker = create_io_broker(io_broker_type::epoll, io_broker_cfg);

  // Register the commands.
  app_services::cmdline_command_dispatcher command_parser(*epoll_broker, *workers.non_rt_low_prio_exec, {});

  app_services::metrics_notifier_proxy_impl metrics_notifier_forwarder;

  // Create app-level resource usage service and metrics.
  auto app_resource_usage_service = app_services::build_app_resource_usage_service(
      metrics_notifier_forwarder, du_low_cfg.metrics_cfg.rusage_config, srslog::fetch_basic_logger("APP"));

  std::vector<app_services::metrics_config> app_metrics = std::move(app_resource_usage_service.metrics);

  auto du = o_du_app_unit->create_flexible_o_du_low(
      workers, metrics_notifier_forwarder, app_timers, srslog::fetch_basic_logger("APP"));

  for (auto& metric : du.metrics) {
    app_metrics.push_back(std::move(metric));
  }

  // Only DU has metrics now.
  app_services::metrics_manager metrics_mngr(
      srslog::fetch_basic_logger("APP"),
      *workers.metrics_exec,
      app_metrics,
      app_timers,
      std::chrono::milliseconds(du_low_cfg.metrics_cfg.metrics_service_cfg.app_usage_report_period));

  // Connect the forwarder to the metrics manager.
  metrics_notifier_forwarder.connect(metrics_mngr);

  // :TODO: how to manage cmdline and remote commands??

  metrics_mngr.start();
  du.odu_low->start();

  {
    app_services::application_message_banners app_banner(app_name, du_low_cfg.log_cfg.filename);

    while (is_app_running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
  }

  du.odu_low->stop();
  metrics_mngr.stop();

  workers.stop();

  srslog::flush();

  return 0;
}
