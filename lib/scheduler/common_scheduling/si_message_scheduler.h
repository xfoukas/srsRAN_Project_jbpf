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

#include "../pdcch_scheduling/pdcch_resource_allocator.h"
#include "srsran/srslog/logger.h"

namespace srsran {

class si_message_scheduler
{
public:
  si_message_scheduler(const cell_configuration&                  cfg_,
                       pdcch_resource_allocator&                  pdcch_sch,
                       const std::optional<si_scheduling_config>& si_sched_cfg_);

  /// \brief Performs broadcast SI message scheduling.
  ///
  /// \param[out,in] res_grid Resource grid with current allocations and scheduling results.
  void run_slot(cell_slot_resource_allocator& res_grid);

  /// \brief Update the SI messages.
  void handle_si_message_update_indication(unsigned                                   version,
                                           const std::optional<si_scheduling_config>& new_si_sched_cfg);

private:
  struct message_window_context {
    /// SI message window.
    interval<slot_point> window;
    /// Number of SI message transmissions within the current window.
    unsigned nof_tx_in_current_window = 0;
    /// Total number of SI message transmissions.
    unsigned long total_nof_tx = 0;
  };

  void update_si_message_windows(slot_point sl_tx);

  void schedule_pending_si_messages(cell_slot_resource_allocator& res_grid);

  bool allocate_si_message(unsigned si_message, cell_slot_resource_allocator& res_grid);

  void fill_si_grant(cell_slot_resource_allocator& res_grid,
                     unsigned                      si_message,
                     crb_interval                  si_crbs_grant,
                     uint8_t                       time_resource,
                     const dmrs_information&       dmrs_info,
                     unsigned                      tbs,
                     const message_window_context& message_context);

  // Configuration of the broadcast SI messages.
  const scheduler_si_expert_config&   expert_cfg;
  const cell_configuration&           cell_cfg;
  pdcch_resource_allocator&           pdcch_sch;
  std::optional<si_scheduling_config> si_sched_cfg;
  srslog::basic_logger&               logger;

  std::vector<message_window_context> pending_messages;
  unsigned                            version = 0;
};

} // namespace srsran
