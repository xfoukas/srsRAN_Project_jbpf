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

#include "ethernet_tx_metrics_collector_impl.h"
#include "srsran/ofh/ethernet/ethernet_transmitter.h"
#include "srsran/ofh/ethernet/ethernet_transmitter_config.h"
#include "srsran/srslog/logger.h"
#include <linux/if_packet.h>

namespace srsran {
namespace ether {

/// Implementation for the Ethernet transmitter.
class transmitter_impl : public transmitter
{
public:
  transmitter_impl(const transmitter_config& config, srslog::basic_logger& logger_);
  ~transmitter_impl() override;

  // See interface for documentation.
  void send(span<span<const uint8_t>> frames) override;

  // See interface for documentation.
  transmitter_metrics_collector* get_metrics_collector() override;

private:
  srslog::basic_logger&              logger;
  int                                socket_fd = -1;
  transmitter_metrics_collector_impl metrics_collector;
  ::sockaddr_ll                      socket_address;
};

} // namespace ether
} // namespace srsran
