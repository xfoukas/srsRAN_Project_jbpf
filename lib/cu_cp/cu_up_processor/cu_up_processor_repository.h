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

#include "../task_schedulers/cu_up_task_scheduler.h"
#include "cu_up_processor_impl_interface.h"
#include "srsran/cu_cp/cu_cp_e1_handler.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include <map>
#include <unordered_map>

namespace srsran {
namespace srs_cu_cp {

struct cu_cp_configuration;

struct cu_up_repository_config {
  const cu_cp_configuration& cu_cp;
  e1ap_cu_cp_notifier&       e1ap_ev_notifier;
  srslog::basic_logger&      logger;
};

class cu_up_processor_repository
{
public:
  explicit cu_up_processor_repository(cu_up_repository_config cfg_);

  /// \brief Adds a CU-UP processor object to the CU-CP.
  /// \return The CU-UP index of the added CU-UP processor object.
  cu_up_index_t add_cu_up(std::unique_ptr<e1ap_message_notifier> e1ap_tx_pdu_notifier);

  /// \brief Removes the specified CU-UP processor object from the CU-CP.
  /// \param[in] cu_up_index The index of the CU-UP processor to delete.
  async_task<void> remove_cu_up(cu_up_index_t cu_up_index);

  size_t get_nof_cu_ups() const { return cu_up_db.size(); }

  cu_up_processor_e1ap_interface& get_cu_up(cu_up_index_t cu_up_index);

  /// \brief Find a CU-UP object.
  /// \param[in] cu_up_index The index of the CU-UP processor object.
  /// \return The CU-UP processor object if it exists, nullptr otherwise.
  cu_up_processor_impl_interface* find_cu_up_processor(cu_up_index_t cu_up_index);

  size_t get_nof_e1ap_ues();

private:
  struct cu_up_context {
    std::unique_ptr<cu_up_processor_impl_interface> processor;

    /// Notifier used by the CU-CP to push E1AP Tx messages to the respective CU-UP.
    std::unique_ptr<e1ap_message_notifier> e1ap_tx_pdu_notifier;
  };

  /// \brief Get the next available index from the CU-UP processor database.
  /// \return The CU-UP index.
  cu_up_index_t allocate_cu_up_index();

  cu_up_repository_config cfg;
  srslog::basic_logger&   logger;

  cu_up_task_scheduler cu_up_task_sched;

  std::map<cu_up_index_t, cu_up_context> cu_up_db;

  // TODO: CU-UP removal not yet fully supported. Instead we just move the CU-UP context to a separate map.
  std::map<cu_up_index_t, cu_up_context> removed_cu_up_db;
};

} // namespace srs_cu_cp
} // namespace srsran
