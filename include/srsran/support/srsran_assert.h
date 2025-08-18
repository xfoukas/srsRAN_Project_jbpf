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

#include "srsran/support/compiler.h"
#include "srsran/support/error_handling.h"

namespace srsran {
namespace detail {

/// \brief Helper function to format and print assertion messages, first stage.
///
/// \param filename file name where assertion failed.
/// \param line line in which assertion was placed.
/// \param funcname function name where assertion failed.
/// \param condstr assertion condition that failed.
[[gnu::noinline]] inline void
print_and_abort_1(const char* filename, int line, const char* funcname, const char* condstr) noexcept
{
  if (auto handler = error_report_handler.exchange(nullptr)) {
    handler();
  }

  ::fflush(stdout);
  fmt::print(stderr, "{}:{}: {}: \n", filename, line, funcname);
  if (condstr == nullptr) {
    fmt::print(stderr, "Assertion failed");
  } else {
    fmt::print(stderr, "Assertion `{}' failed", condstr);
  }
}

/// \brief Helper function to format and print assertion messages, second stage.
///
/// \param msg additional assertion message.
template <typename... Args>
[[gnu::noinline, noreturn]] inline void print_and_abort_2(fmt::string_view fmt, Args&&... args) noexcept
{
  if (fmt.size()) {
    fmt::print(stderr, " - ");
    fmt::print(stderr, fmt, std::forward<Args>(args)...);
  }
  fmt::print(stderr, "\n");

  std::abort();
}

} // namespace detail
} // namespace srsran

// NOLINTBEGIN

/// Helper macro to log assertion message and terminate program.
#define SRSRAN_ASSERT_FAILURE__(condmessage, fmtstr, ...)                                                              \
  (srsran::detail::print_and_abort_1(__FILE__, __LINE__, __PRETTY_FUNCTION__, condmessage),                            \
   srsran::detail::print_and_abort_2(fmtstr, ##__VA_ARGS__))

/// \brief Helper macro that asserts condition is true. If false, it logs the remaining macro args, flushes the log,
/// prints the backtrace (if it was activated) and closes the application.
#define SRSRAN_ALWAYS_ASSERT__(condition, fmtstr, ...)                                                                 \
  (void)((condition) || (SRSRAN_ASSERT_FAILURE__((#condition), fmtstr, ##__VA_ARGS__), 0))

/// Same as "SRSRAN_ALWAYS_ASSERT__" but it is only active when "enable_check" flag is defined
#define SRSRAN_ALWAYS_ASSERT_IFDEF__(enable_check, condition, fmtstr, ...)                                             \
  (void)((not SRSRAN_IS_DEFINED(enable_check)) || (SRSRAN_ALWAYS_ASSERT__(condition, fmtstr, ##__VA_ARGS__), 0))

/// \brief Terminates program with an assertion failure. No condition message is provided.
#define srsran_assertion_failure(fmtstr, ...)                                                                          \
  (void)((not SRSRAN_IS_DEFINED(ASSERTS_ENABLED)) || (SRSRAN_ASSERT_FAILURE__(nullptr, fmtstr, ##__VA_ARGS__), 0))

/// Specialization of "SRSRAN_ALWAYS_ASSERT_IFDEF__" for the ASSERTS_ENABLED flag.
#define srsran_assert(condition, fmtstr, ...)                                                                          \
  SRSRAN_ALWAYS_ASSERT_IFDEF__(ASSERTS_ENABLED, condition, fmtstr, ##__VA_ARGS__)

/// Specialization of "SRSRAN_ALWAYS_ASSERT_IFDEF__" for the PARANOID_ASSERTS_ENABLED flag.
#define srsran_sanity_check(condition, fmtstr, ...)                                                                    \
  SRSRAN_ALWAYS_ASSERT_IFDEF__(PARANOID_ASSERTS_ENABLED, condition, fmtstr, ##__VA_ARGS__)

#define srsran_assume(condition) static_cast<void>((condition) ? void(0) : SRSRAN_UNREACHABLE)

// NOLINTEND
