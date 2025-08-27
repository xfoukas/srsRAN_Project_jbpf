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

#include "srsran/srslog/srslog.h"
#include "srsran/support/executors/task_execution_manager.h"
#include <future>
#include <gtest/gtest.h>

using namespace srsran;

class task_execution_manager_test : public ::testing::Test
{
public:
  task_execution_manager_test() { srslog::init(); }
};

TEST_F(task_execution_manager_test, creation_of_single_task_worker)
{
  using namespace execution_config_helper;
  single_worker cfg{"WORKER", {"EXEC", concurrent_queue_policy::lockfree_spsc, 8}, std::chrono::microseconds{100}};

  task_execution_manager mng;
  ASSERT_TRUE(mng.add_execution_context(create_execution_context(cfg)));

  ASSERT_EQ(mng.executors().size(), 1);
  ASSERT_EQ(mng.executors().count("EXEC"), 1);

  // Run single task in created execution environment.
  std::promise<std::string> p;
  std::future<std::string>  f       = p.get_future();
  bool                      success = mng.executors().at("EXEC")->execute([&p]() { p.set_value(this_thread_name()); });
  ASSERT_TRUE(success);
  std::string thread_name = f.get();
  ASSERT_EQ(thread_name, "WORKER");
}

TEST_F(task_execution_manager_test, creation_of_task_worker_pool)
{
  using namespace execution_config_helper;
  worker_pool cfg{"WORKER_POOL", 4, {{"EXEC", concurrent_queue_policy::locking_mpmc, 8}}};

  task_execution_manager mng;
  ASSERT_TRUE(mng.add_execution_context(create_execution_context(cfg)));

  ASSERT_EQ(mng.executors().size(), 1);
  ASSERT_EQ(mng.executors().count("EXEC"), 1);

  // Run single task in created execution environment.
  std::promise<std::string> p;
  std::future<std::string>  f       = p.get_future();
  bool                      success = mng.executors().at("EXEC")->execute([&p]() { p.set_value(this_thread_name()); });
  ASSERT_TRUE(success);
  std::string thread_name = f.get();
  ASSERT_EQ(thread_name.find("WORKER_POOL#"), 0);
}

TEST_F(task_execution_manager_test, worker_with_queues_of_different_priorities)
{
  using namespace execution_config_helper;
  priority_multiqueue_worker cfg{
      "WORKER",
      {task_queue{"EXEC1", concurrent_queue_policy::lockfree_spsc, 8},
       task_queue{"EXEC2", concurrent_queue_policy::locking_mpsc, 8}},
      std::chrono::microseconds{10},
  };

  task_execution_manager mng;
  ASSERT_TRUE(mng.add_execution_context(create_execution_context(cfg)));

  ASSERT_EQ(mng.executors().size(), 2);
  ASSERT_EQ(mng.executors().count("EXEC1"), 1);
  ASSERT_EQ(mng.executors().count("EXEC2"), 1);
  ASSERT_NE(mng.executors().at("EXEC1"), mng.executors().at("EXEC2"));

  std::atomic<int> counter{0};
  std::vector<int> execs_called;
  bool             success = mng.executors().at("EXEC1")->execute([&mng, &execs_called, &counter]() {
    ASSERT_TRUE(mng.executors().at("EXEC2")->defer([&execs_called, &counter]() {
      execs_called.push_back(2);
      counter++;
    }));
    ASSERT_TRUE(mng.executors().at("EXEC1")->defer([&execs_called, &counter]() {
      execs_called.push_back(1);
      counter++;
    }));
  });
  ASSERT_TRUE(success);

  while (counter != 2) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::vector<int> expected{1, 2};
  ASSERT_EQ(execs_called, expected) << "The highest priority executed should have been called first";
}
