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

#include <cstdint>
#include <string>

namespace srsran {
namespace app_helpers {

/// Metrics JSON configuration.
struct metrics_json_config {
  /// Enables the metrics in JSON format.
  bool enable_json_metrics = false;
  /// IP address the JSON metrics will be written into.
  std::string addr = "127.0.0.1";
  /// Port the JSON metrics will be written into.
  uint16_t port = 55555;
};

/// Metrics configuration structure.
struct metrics_config {
  /// Enables the metrics in the log.
  bool enable_log_metrics = false;
  /// Enable verbose metrics.
  bool enable_verbose = false;

  /// JSON metrics configuration.
  metrics_json_config json_config;

  /// Returns true if the metrics are enabled, otherwise false.
  bool enabled() const { return enable_log_metrics || json_config.enable_json_metrics; }
};

} // namespace app_helpers
} // namespace srsran
