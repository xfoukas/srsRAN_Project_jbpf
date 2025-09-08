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
#include "srsran/ofh/timing/ofh_ota_symbol_boundary_notifier.h"
#include "srsran/support/executors/task_executor.h"
#include "srsran/support/rtsan.h"

namespace srsran {
namespace ofh {

/// OTA symbol task dispatcher for the transmitter.
class transmitter_ota_symbol_task_dispatcher : public ota_symbol_boundary_notifier
{
public:
  transmitter_ota_symbol_task_dispatcher(srslog::basic_logger&         logger_,
                                         task_executor&                executor_,
                                         ota_symbol_boundary_notifier& dl_window_checker_,
                                         ota_symbol_boundary_notifier& ul_window_checker_,
                                         ota_symbol_boundary_notifier& symbol_handler_) :
    logger(logger_),
    executor(executor_),
    dl_window_checker(dl_window_checker_),
    ul_window_checker(ul_window_checker_),
    symbol_handler(symbol_handler_)
  {
  }

  void on_new_symbol(const slot_symbol_point_context& symbol_point_context) override
  {
    dl_window_checker.on_new_symbol(symbol_point_context);
    ul_window_checker.on_new_symbol(symbol_point_context);

    if (!executor.execute([&, symbol_point_context]()
                              SRSRAN_RTSAN_NONBLOCKING { symbol_handler.on_new_symbol(symbol_point_context); })) {
      logger.warning("Failed to dispatch new symbol task in the message transmitter for slot '{}' and symbol '{}'",
                     symbol_point_context.symbol_point.get_slot(),
                     symbol_point_context.symbol_point.get_symbol_index());
    }
  }

private:
  srslog::basic_logger&         logger;
  task_executor&                executor;
  ota_symbol_boundary_notifier& dl_window_checker;
  ota_symbol_boundary_notifier& ul_window_checker;
  ota_symbol_boundary_notifier& symbol_handler;
};

} // namespace ofh
} // namespace srsran
