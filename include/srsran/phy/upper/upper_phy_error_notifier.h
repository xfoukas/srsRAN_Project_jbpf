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

#include "srsran/ran/slot_point.h"

namespace srsran {

/// Upper physical layer error notifier.
class upper_phy_error_notifier
{
public:
  /// Default destructor.
  virtual ~upper_phy_error_notifier() = default;

  /// \brief Notifies a late downlink message.
  ///
  /// \param[in] dl_msg_slot Slot of the downlink message.
  virtual void on_late_downlink_message(slot_point dl_msg_slot) = 0;

  /// \brief Notifies a late uplink request message.
  ///
  /// \param[in] ul_msg_slot Slot of the uplink message.
  virtual void on_late_uplink_message(slot_point ul_msg_slot) = 0;

  /// \brief Notifies a late PRACH request message.
  ///
  /// \param[in] prach_msg_slot Slot of the uplink message.
  virtual void on_late_prach_message(slot_point prach_msg_slot) = 0;
};

} // namespace srsran
