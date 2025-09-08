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

#include "srsran/ran/subcarrier_spacing.h"
#include "srsran/ru/ru_error_notifier.h"
#include "srsran/ru/ru_timing_notifier.h"
#include "srsran/ru/ru_uplink_plane.h"
#include "srsran/srslog/logger.h"
#include "srsran/support/executors/task_executor.h"

namespace srsran {

/// Collects the necessary parameters for the dummy radio unit.
struct ru_dummy_configuration {
  /// Flag that enables (or not) metrics.
  bool are_metrics_enabled;
  /// Subcarrier spacing - Determines the slot timing.
  subcarrier_spacing scs;
  /// Number of sectors.
  unsigned nof_sectors;
  /// Receive resource grid number of PRB.
  unsigned rx_rg_nof_prb;
  /// Receive resource grid number of ports.
  unsigned rx_rg_nof_ports;
  /// Receive PRACH buffer number of ports.
  unsigned rx_prach_nof_ports;
  /// \brief Number of slots is notified in advance of the transmission time.
  ///
  /// Sets the maximum allowed processing delay in slots.
  unsigned max_processing_delay_slots;
  /// DL processing processing delay in slots. See \ref ru_dummy_appconfig::dl_processing_delay for more information.
  unsigned dl_processing_delay;
  /// Time scaling, \ref ru_dummy_unit_config::time_scaling for more details.
  float time_scaling;
};

/// Collects the necessary dependencies for the dummy radio unit.
struct ru_dummy_dependencies {
  /// Logger.
  srslog::basic_logger& logger;
  /// Asynchronous task executor.
  task_executor* executor = nullptr;
  /// Radio Unit uplink plane received symbol notifier.
  ru_uplink_plane_rx_symbol_notifier& symbol_notifier;
  /// Radio Unit timing notifier.
  ru_timing_notifier& timing_notifier;
  /// Radio Unit error notifier.
  ru_error_notifier& error_notifier;
};

} // namespace srsran
