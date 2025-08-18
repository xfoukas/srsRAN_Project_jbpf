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

#include "srsran/adt/blocking_queue.h"
#include "srsran/support/io/io_broker.h"
#include "srsran/support/io/unique_fd.h"
#include <future>
#include <unordered_map>
#include <utility>

namespace srsran {

/// \brief Implementation of an IO broker using epoll.
class io_broker_epoll final : public io_broker
{
public:
  explicit io_broker_epoll(const io_broker_config& config);
  ~io_broker_epoll() override;

  [[nodiscard]] subscriber
  register_fd(unique_fd fd, task_executor& executor, recv_callback_t handler, error_callback_t err_handler) override;

private:
  /// Event enqueued to be handled in the io_broker thread.
  struct control_event {
    enum class event_type { close_io_broker, register_fd, deregister_fd } type;
    unique_fd           fd;
    int                 raw_fd;
    task_executor*      executor = nullptr;
    recv_callback_t     handler;
    error_callback_t    err_handler;
    std::promise<bool>* completed = nullptr;
  };

  /// Event handler for a file descriptor.
  struct fd_handler {
    fd_handler(task_executor*   executor_,
               recv_callback_t  read_callback_,
               error_callback_t error_callback_,
               unique_fd        fd_) :
      executor(executor_),
      read_callback(std::move(read_callback_)),
      error_callback(std::move(error_callback_)),
      fd(std::move(fd_))
    {
    }

    task_executor*    executor;
    recv_callback_t   read_callback;
    error_callback_t  error_callback;
    std::atomic<bool> is_executing_recv_callback{false};
    unique_fd         fd;

    // Determines whether the io_broker has deregistered the event handler from the epoll.
    bool registed_in_epoll() const { return static_cast<bool>(read_callback); }
  };

  [[nodiscard]] bool unregister_fd(int fd, std::promise<bool>* complete_notifier) override;

  void thread_loop();

  // Enqueues event to be asynchronously processed by the epoll thread.
  bool enqueue_event(control_event&& event);

  // Handle events stored in the ctrl event queue.
  void handle_enqueued_events();

  // Handle the registration of a new file descriptor.
  bool handle_fd_registration(unique_fd               fd,
                              const recv_callback_t&  callback,
                              const error_callback_t& err_handler,
                              task_executor*          executor,
                              std::promise<bool>*     complete_notifier);

  // Handle the deregistration of an existing file descriptor.
  bool handle_fd_epoll_removal(int                       fd,
                               bool                      io_broker_deregistration_required,
                               std::optional<error_code> epoll_error,
                               std::promise<bool>*       complete_notifier);

  void stop_impl();

  void rearm_fd(int fd);

  srslog::basic_logger& logger;

  // Main epoll file descriptor
  unique_fd epoll_fd;
  // Event file descriptor used to interrupt epoll_wait call when a stop, fd registration, or fd deregistration is
  // requested.
  int ctrl_event_raw_fd = -1;

  // Lookup table mapping file descriptors to handlers.
  std::unordered_map<int, fd_handler> event_handler;

  // Queue used to communicate commands to the epoll broker.
  blocking_queue<control_event> event_queue;

  std::atomic<bool> running{true};
  unique_thread     thread;

  // File descriptors pending to be removed.
  std::vector<std::pair<int, std::promise<bool>*>> pending_fds_to_remove;
};

} // namespace srsran
