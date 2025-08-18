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

namespace fapi {
class slot_data_message_notifier;
class error_message_notifier;
class slot_last_message_notifier;
class slot_message_gateway;
class slot_time_message_notifier;
} // namespace fapi

class upper_phy_error_notifier;
class upper_phy_timing_notifier;
class upper_phy_rx_results_notifier;

namespace fapi_adaptor {

/// \brief PHY&ndash;FAPI bidirectional sector adaptor interface.
///
/// This adaptor is a collection of interfaces to translate FAPI messages into their PHY layer counterpart and vice
/// versa.
///
/// \note All implementations of this public interface must hold the ownership of all its internal components.
class phy_fapi_sector_adaptor
{
public:
  virtual ~phy_fapi_sector_adaptor() = default;

  /// Returns a reference to the error notifier used by the adaptor.
  virtual upper_phy_error_notifier& get_error_notifier() = 0;

  /// Returns a reference to the timing notifier used by the adaptor.
  virtual upper_phy_timing_notifier& get_timing_notifier() = 0;

  /// Returns a reference to the results notifier used by the adaptor.
  virtual upper_phy_rx_results_notifier& get_rx_results_notifier() = 0;

  /// Returns a reference to the slot-based message gateway used by the adaptor.
  virtual fapi::slot_message_gateway& get_slot_message_gateway() = 0;

  /// Returns a reference to the slot-based last message notifier used by the adaptor.
  virtual fapi::slot_last_message_notifier& get_slot_last_message_notifier() = 0;

  /// Configures the slot-based, time-specific message notifier to the given one.
  virtual void set_slot_time_message_notifier(fapi::slot_time_message_notifier& fapi_time_notifier) = 0;

  /// Configures the error-specific message notifier to the given one.
  virtual void set_error_message_notifier(fapi::error_message_notifier& fapi_error_notifier) = 0;

  /// Configures the slot-based, data-specific message notifier to the given one.
  virtual void set_slot_data_message_notifier(fapi::slot_data_message_notifier& fapi_data_notifier) = 0;
};

} // namespace fapi_adaptor
} // namespace srsran
