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

namespace srsran {

/// Port channel estimator frequency domain smoothing strategy.
enum class port_channel_estimator_fd_smoothing_strategy {
  /// No smoothing strategy.
  none = 0,
  /// Averages all frequency domain estimates.
  mean,
  /// Filters in the frequency domain with a low pass filter.
  filter,
};

/// Port channel estimator time domain interpolation strategy.
enum class port_channel_estimator_td_interpolation_strategy {
  /// Performs linear interpolation between the symbols containing DM-RS.
  interpolate = 0,
  /// Averages all time domain estimates.
  average
};

} // namespace srsran
