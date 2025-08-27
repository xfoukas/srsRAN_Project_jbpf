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

#include <chrono>
#include <cmath>

namespace srsran {
namespace ofh {

/// Structure storing the transmission window timing parameters expressed in a number of symbols.
struct tx_window_timing_parameters {
  /// Offset from the current OTA symbol to the first symbol at which DL Control-Plane message can be sent, or in
  /// other words it is the offset to the start of DL Control-Plane transmission window. Must be calculated based on
  /// \c T1a_max_cp_dl parameter.
  unsigned sym_cp_dl_start;
  /// Offset from the current OTA symbol to the last symbol at which DL Control-Plane message can be sent within its
  /// transmission window. Must be calculated based on \c T1a_min_cp_dl parameter.
  unsigned sym_cp_dl_end;
  /// Offset from the current OTA symbol to the first symbol at which UL Control-Plane message can be sent within its
  /// transmission window. Must be calculated based on \c T1a_max_cp_ul parameter.
  unsigned sym_cp_ul_start;
  /// Offset from the current OTA symbol to the last symbol at which UL Control-Plane message can be sent within its
  /// transmission window. Must be calculated based on \c T1a_min_cp_ul parameter.
  unsigned sym_cp_ul_end;
  /// Offset from the current OTA symbol to the first symbol at which DL User-Plane message can be sent within its
  /// transmission window. Must be calculated based on \c T1a_max_up parameter.
  unsigned sym_up_dl_start;
  /// Offset from the current OTA symbol to the last symbol at which DL User-Plane message can be sent within its
  /// transmission window. Must be calculated based on \c T1a_min_up parameter.
  unsigned sym_up_dl_end;
};

} // namespace ofh
} // namespace srsran
