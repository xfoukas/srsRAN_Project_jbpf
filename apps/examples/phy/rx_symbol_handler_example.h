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

#include "srsran/phy/lower/lower_phy_timing_notifier.h"
#include "srsran/phy/support/shared_resource_grid.h"
#include "srsran/phy/upper/upper_phy_rx_symbol_handler.h"
#include <mutex>

namespace srsran {

class rx_symbol_handler_example : public upper_phy_rx_symbol_handler
{
private:
  srslog::basic_logger& logger;
  std::mutex            mutex;

public:
  rx_symbol_handler_example(srslog::basic_levels log_level) : logger(srslog::fetch_basic_logger("RxSyHan"))
  {
    logger.set_level(log_level);
  }

  void handle_rx_symbol(const upper_phy_rx_symbol_context& context, const shared_resource_grid& grid) override
  {
    std::unique_lock<std::mutex> lock(mutex);
    logger.debug(context.slot.sfn(),
                 context.slot.slot_index(),
                 "Rx symbol {} received for sector {}",
                 context.symbol,
                 context.sector);
  }

  void handle_rx_prach_window(const prach_buffer_context& context, const prach_buffer& buffer) override
  {
    std::unique_lock<std::mutex> lock(mutex);
    logger.debug(context.slot.sfn(),
                 context.slot.slot_index(),
                 "PRACH symbol {} received for sector {}",
                 context.start_symbol,
                 context.sector);
  }
};

} // namespace srsran
