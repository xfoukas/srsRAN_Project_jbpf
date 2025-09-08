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

#include "srsran/hal/phy/upper/channel_processors/pusch/ext_harq_buffer_context_repository.h"
#include <memory>

namespace srsran {
namespace hal {

/// Returns a ext_harq_buffer_context_repository instance on success, otherwise returns nullptr.
std::shared_ptr<ext_harq_buffer_context_repository>
create_ext_harq_buffer_context_repository(unsigned nof_codeblocks,
                                          uint64_t ext_harq_buff_size,
                                          bool     debug_mode = false);

} // namespace hal
} // namespace srsran
