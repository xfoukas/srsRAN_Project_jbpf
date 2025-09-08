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

#include "srsran/e1ap/cu_up/e1ap_cu_up.h"
#include "srsran/e1ap/gateways/e1_connection_client.h"

namespace srsran {
namespace srs_cu_up {

class e1ap_cu_up_connection_handler
{
public:
  e1ap_cu_up_connection_handler(e1_connection_client& e1ap_client_handler_, e1ap_message_handler& e1ap_pdu_handler_);

  [[nodiscard]] e1ap_message_notifier* connect_to_cu_cp();
  [[nodiscard]] bool                   is_connected() const { return e1ap_notifier != nullptr; }

private:
  e1_connection_client& e1_client_handler;
  e1ap_message_handler& e1ap_pdu_handler;
  srslog::basic_logger& logger;

  std::unique_ptr<e1ap_message_notifier> e1ap_notifier;
};

} // namespace srs_cu_up
} // namespace srsran
