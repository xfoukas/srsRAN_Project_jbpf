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

#include "srsran/ofh/ofh_controller.h"
#include "srsran/ofh/timing/ofh_ota_symbol_boundary_notifier.h"

namespace srsran {
namespace ofh {

class receiver_metrics_collector;

/// Open Fronthaul receiver interface.
class receiver
{
public:
  virtual ~receiver() = default;

  /// Returns an OTA symbol boundary notifier of this Open Fronthaul receiver or nullptr if not present.
  virtual ota_symbol_boundary_notifier* get_ota_symbol_boundary_notifier() = 0;

  /// Returns the controller of this Open Fronthaul receiver.
  virtual operation_controller& get_operation_controller() = 0;

  /// Returns the metrics collector of this Open Fronthaul receiver or nullptr if metrics are disabled.
  virtual receiver_metrics_collector* get_metrics_collector() = 0;
};

} // namespace ofh
} // namespace srsran
