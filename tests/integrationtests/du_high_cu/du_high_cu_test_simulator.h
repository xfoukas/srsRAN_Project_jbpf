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

#include "tests/test_doubles/du/test_du_high_worker_manager.h"
#include "tests/test_doubles/f1ap/f1c_test_local_gateway.h"
#include "tests/test_doubles/mac/dummy_mac_result_notifier.h"
#include "tests/test_doubles/mac/dummy_scheduler_ue_metric_notifier.h"
#include "tests/unittests/ngap/test_helpers.h"
#include "srsran/cu_cp/cu_cp.h"
#include "srsran/du/du_high/du_high.h"
#include "srsran/du/du_high/du_high_configuration.h"
#include "srsran/support/executors/manual_task_worker.h"
#include "srsran/support/executors/task_worker.h"

namespace srsran {

class du_high_cu_cp_worker_manager
{
public:
  explicit du_high_cu_cp_worker_manager(unsigned nof_dus, timer_manager& timers);
  ~du_high_cu_cp_worker_manager();

  void stop();

  manual_task_worker                                                 test_worker;
  std::vector<std::unique_ptr<test_helpers::du_high_worker_manager>> dus;
  task_executor*                                                     cu_cp_exec = nullptr;
};

struct du_high_cu_cp_test_simulator_config {
  std::vector<std::vector<srs_du::du_cell_config>> dus;
};

class du_high_cu_test_simulator
{
public:
  struct du_sim {
    srs_du::du_high_configuration    du_high_cfg;
    phy_test_dummy                   phy;
    null_mac_pcap                    mac_pcap;
    null_rlc_pcap                    rlc_pcap;
    std::unique_ptr<srs_du::du_high> du_high_inst;

    slot_point next_slot;

    du_sim(task_executor& phy_exec) : phy(1, phy_exec) {}
  };

  explicit du_high_cu_test_simulator(const du_high_cu_cp_test_simulator_config& cfg);
  ~du_high_cu_test_simulator();

  void start_dus();

  bool add_ue(unsigned du_index, rnti_t rnti);

  void run_slot();

  bool run_until(unique_function<bool()> condition);

  const du_high_cu_cp_test_simulator_config cfg;

  srslog::basic_logger&        logger;
  timer_manager                timers;
  du_high_cu_cp_worker_manager workers;
  srs_cu_cp::dummy_n2_gateway  n2_gw;
  f1c_test_local_gateway       f1c_gw;

  std::unique_ptr<srs_cu_cp::cu_cp>    cu_cp_inst;
  std::vector<std::unique_ptr<du_sim>> dus;
};

} // namespace srsran
