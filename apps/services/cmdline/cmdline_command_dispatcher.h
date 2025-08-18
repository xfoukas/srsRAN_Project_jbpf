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

#include "cmdline_command.h"
#include "srsran/adt/span.h"
#include "srsran/srslog/logger.h"
#include "srsran/support/io/io_broker.h"
#include <string>
#include <unordered_map>

namespace srsran {
namespace app_services {

/// \brief Command-line command dispatcher.
///
/// The command-line command dispatcher parses commands from the STDIN and executes them if they were registered upon
/// creation, otherwise prints a list of the registered commands with their description.
class cmdline_command_dispatcher
{
public:
  cmdline_command_dispatcher(io_broker&                             io_broker,
                             task_executor&                         executor,
                             span<std::unique_ptr<cmdline_command>> commands_);

private:
  /// Parses any contents in the STDIN file descriptor.
  void parse_stdin();

  /// Handles the given command.
  void handle_command(const std::string& command);

  /// Prints the registered events registered in this console.
  void print_help();

private:
  srslog::basic_logger&                                             logger;
  io_broker::subscriber                                             stdin_handle;
  std::unordered_map<std::string, std::unique_ptr<cmdline_command>> commands;
};

} // namespace app_services
} // namespace srsran
