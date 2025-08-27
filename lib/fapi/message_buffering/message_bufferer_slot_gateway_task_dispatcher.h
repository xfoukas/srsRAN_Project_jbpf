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

#include "message_bufferer_slot_gateway_impl.h"
#include "srsran/fapi/slot_time_message_notifier.h"

namespace srsran {

class task_executor;

namespace fapi {

/// Buffered slot gateway task dispatcher.
class message_bufferer_slot_gateway_task_dispatcher : public slot_message_gateway
{
public:
  message_bufferer_slot_gateway_task_dispatcher(unsigned              sector_id_,
                                                unsigned              l2_nof_slots_ahead,
                                                subcarrier_spacing    scs_,
                                                slot_message_gateway& gateway,
                                                task_executor&        executor_);

  /// Updates the current slot of the message bufferer slot gateway.
  void update_current_slot(slot_point slot);

  /// Forwards cached messages for the given slot.
  void forward_cached_messages(slot_point slot);

  // See interface for documentation.
  void dl_tti_request(const dl_tti_request_message& msg) override;

  // See interface for documentation.
  void ul_tti_request(const ul_tti_request_message& msg) override;

  // See interface for documentation.
  void ul_dci_request(const ul_dci_request_message& msg) override;

  // See interface for documentation.
  void tx_data_request(const tx_data_request_message& msg) override;

private:
  const unsigned                     sector_id;
  const subcarrier_spacing           scs;
  srslog::basic_logger&              logger;
  task_executor&                     executor;
  message_bufferer_slot_gateway_impl message_bufferer_gateway;
};

} // namespace fapi
} // namespace srsran
