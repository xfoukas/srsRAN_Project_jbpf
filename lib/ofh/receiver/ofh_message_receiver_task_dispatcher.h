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

#include "ofh_message_receiver.h"
#include "srsran/srslog/srslog.h"
#include "srsran/support/executors/task_executor.h"

namespace srsran {
namespace ofh {

/// Open Fronthaul message receiver interface implementation that dispatches tasks to the actual receiver.
class ofh_message_receiver_task_dispatcher : public message_receiver
{
public:
  ofh_message_receiver_task_dispatcher(srslog::basic_logger& logger_,
                                       message_receiver&     msg_receiver_,
                                       task_executor&        executor_,
                                       unsigned              sector_) :
    logger(logger_), msg_receiver(msg_receiver_), executor(executor_), sector(sector_)
  {
  }

  // See interface for documentation.
  void on_new_frame(ether::unique_rx_buffer buffer) override
  {
    if (!executor.execute([this, buff = std::move(buffer)]() mutable { msg_receiver.on_new_frame(std::move(buff)); })) {
      logger.warning("Failed to dispatch receiver task for sector#{}", sector);
    }
  }

  // See interface for documentation.
  ether::receiver& get_ethernet_receiver() override { return msg_receiver.get_ethernet_receiver(); }

  // See interface for the documentation.
  message_receiver_metrics_collector* get_metrics_collector() override { return msg_receiver.get_metrics_collector(); }

private:
  srslog::basic_logger& logger;
  message_receiver&     msg_receiver;
  task_executor&        executor;
  const unsigned        sector;
};

} // namespace ofh
} // namespace srsran
