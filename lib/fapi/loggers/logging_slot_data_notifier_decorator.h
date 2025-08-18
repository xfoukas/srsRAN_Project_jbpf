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

#include "srsran/fapi/slot_data_message_notifier.h"
#include "srsran/srslog/srslog.h"

namespace srsran {
namespace fapi {

/// Adds logging information over the implemented interface.
class logging_slot_data_notifier_decorator : public slot_data_message_notifier
{
public:
  logging_slot_data_notifier_decorator(unsigned sector_id_, srslog::basic_logger& logger_);

  // See interface for documentation.
  void on_rx_data_indication(const rx_data_indication_message& msg) override;

  // See interface for documentation.
  void on_crc_indication(const crc_indication_message& msg) override;

  // See interface for documentation.
  void on_uci_indication(const uci_indication_message& msg) override;

  // See interface for documentation.
  void on_srs_indication(const srs_indication_message& msg) override;

  // See interface for documentation.
  void on_rach_indication(const rach_indication_message& msg) override;

  /// Sets the slot data message notifier to the given one.
  void set_slot_data_message_notifier(slot_data_message_notifier& data_notifier);

private:
  const unsigned              sector_id;
  srslog::basic_logger&       logger;
  slot_data_message_notifier* notifier;
};

} // namespace fapi
} // namespace srsran
