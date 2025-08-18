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

#include "srsran/adt/static_vector.h"
#include "srsran/fapi/messages/dl_tti_request.h"
#include "srsran/fapi/messages/tx_data_request.h"
#include "srsran/fapi/messages/ul_dci_request.h"
#include "srsran/fapi/messages/ul_tti_request.h"
#include "srsran/fapi/slot_message_gateway.h"
#include "srsran/srslog/logger.h"
#include <atomic>

namespace srsran {
namespace fapi {

/// Maximum number of buffered messages.
constexpr unsigned MAX_NUM_BUFFERED_MESSAGES = 8U;

/// \brief Buffers incoming FAPI request messages and forwards them when the corresponding slot indication is notified.
///
/// This class buffers incoming FAPI request messages and forwards them through the gateway when the current slot
/// matches the message slot. The slot indication notification is used to look for available messages for that slot,
/// forwarding them.
/// For incoming messages, there are 3 possibilities:
/// - The message slot is bigger than the current slot plus the configured delay. In this case the message is dropped as
///   it should not be possible.
/// - The message slot is smaller or equal than the current slot. In this case, there is no need to store it, and the
///   message is forwarded.
/// - In the rest of the cases, the incoming message is stored in a buffer, waiting to be forwarded when the current
///   slot matches the message slot.
class message_bufferer_slot_gateway_impl
{
public:
  message_bufferer_slot_gateway_impl(unsigned              sector_id_,
                                     unsigned              l2_nof_slots_ahead_,
                                     subcarrier_spacing    scs_,
                                     slot_message_gateway& gateway_);

  /// Handles a DL_TTI.request message.
  void handle_dl_tti_request(const dl_tti_request_message& msg);

  /// Handles a UL_TTI.request message.
  void handle_ul_tti_request(const ul_tti_request_message& msg);

  /// Handles a UL_DCI.request message.
  void handle_ul_dci_request(const ul_dci_request_message& msg);

  /// Handles a TX_Data.request message.
  void handle_tx_data_request(const tx_data_request_message& msg);

  /// Updates the current slot with the given slot.
  void update_current_slot(slot_point slot)
  {
    // Update the atomic variable that holds the slot point.
    current_slot_count_val.store(slot.system_slot(), std::memory_order_relaxed);
  }

  /// Forwards through the gateway the cached messages at the given slot.
  void forward_cached_messages(slot_point slot);

private:
  /// Returns this adaptor current slot.
  slot_point get_current_slot() const
  {
    return slot_point(scs, current_slot_count_val.load(std::memory_order_relaxed));
  }

  /// \brief Handles the given FAPI message.
  ///
  /// Handling a FAPI message consists of:
  ///  - Sending the message if the slot of the message equals or is smaller than the current slot.
  ///  - Storing the given message in the pools if it was not sent.
  ///
  /// \param msg FAPI message to handle.
  /// \param pool Pool of messages.
  /// \param func Function to call to forward the messages
  template <typename T, typename P, typename Function>
  void handle_message(T&& msg, P pool, Function func);

private:
  const unsigned                                                                   sector_id;
  const unsigned                                                                   l2_nof_slots_ahead;
  const subcarrier_spacing                                                         scs;
  slot_message_gateway&                                                            gateway;
  srslog::basic_logger&                                                            logger;
  std::atomic<uint32_t>                                                            current_slot_count_val;
  static_vector<std::optional<dl_tti_request_message>, MAX_NUM_BUFFERED_MESSAGES>  dl_tti_pool;
  static_vector<std::optional<ul_tti_request_message>, MAX_NUM_BUFFERED_MESSAGES>  ul_tti_pool;
  static_vector<std::optional<ul_dci_request_message>, MAX_NUM_BUFFERED_MESSAGES>  ul_dci_pool;
  static_vector<std::optional<tx_data_request_message>, MAX_NUM_BUFFERED_MESSAGES> tx_data_pool;
};

} // namespace fapi
} // namespace srsran
