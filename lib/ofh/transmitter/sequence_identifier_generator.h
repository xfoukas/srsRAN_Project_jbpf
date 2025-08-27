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

#include "srsran/ofh/ofh_constants.h"
#include "srsran/support/srsran_assert.h"
#include <array>
#include <atomic>

namespace srsran {
namespace ofh {

/// Sequence identifier generator.
class sequence_identifier_generator
{
  std::array<std::atomic<uint8_t>, MAX_SUPPORTED_EAXC_ID_VALUE> counters;

public:
  /// Default constructor.
  explicit sequence_identifier_generator(unsigned init_value = 0)
  {
    for (unsigned i = 0; i != MAX_SUPPORTED_EAXC_ID_VALUE; ++i) {
      counters[i] = init_value;
    }
  }

  /// Generates a new sequence identifier and returns it.
  uint8_t generate(unsigned eaxc)
  {
    srsran_assert(eaxc < MAX_SUPPORTED_EAXC_ID_VALUE,
                  "Invalid eAxC value '{}'. Maximum eAxC value is '{}'",
                  eaxc,
                  MAX_SUPPORTED_EAXC_ID_VALUE);

    return counters[eaxc].fetch_add(1, std::memory_order_relaxed);
  }
};

} // namespace ofh
} // namespace srsran
