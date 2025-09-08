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

#include "srsran/adt/strong_type.h"
#include <cstdint>

namespace srsran {

/// RAN slice identifier that should be unique for a given cell,PLMN,S-NSSAI.
struct ran_slice_id_tag {};
using ran_slice_id_t = strong_type<uint8_t,
                                   struct ran_slice_id_tag,
                                   strong_increment_decrement,
                                   strong_equality,
                                   strong_conversion_to<std::size_t>>;

/// RAN slice ID for slice used to schedule SRB(s) traffic.
constexpr ran_slice_id_t SRB_RAN_SLICE_ID = ran_slice_id_t{0};

/// RAN slice ID for default slice used to schedule DRB(s) traffic.
constexpr ran_slice_id_t DEFAULT_DRB_RAN_SLICE_ID = ran_slice_id_t{1};

} // namespace srsran
