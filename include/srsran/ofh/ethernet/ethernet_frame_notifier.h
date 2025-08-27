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

#include "srsran/adt/span.h"
#include "srsran/ofh/ethernet/ethernet_unique_buffer.h"

namespace srsran {
namespace ether {

/// Describes an Ethernet frame notifier.
class frame_notifier
{
public:
  /// Default destructor.
  virtual ~frame_notifier() = default;

  /// Notifies the reception of an Ethernet frame coming from the underlying Ethernet link.
  virtual void on_new_frame(unique_rx_buffer buffer) = 0;
};

} // namespace ether
} // namespace srsran
