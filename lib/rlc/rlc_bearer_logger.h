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

#include "srsran/ran/gnb_du_id.h"
#include "srsran/ran/rb_id.h"
#include "srsran/support/format/fmt_to_c_str.h"
#include "srsran/support/format/prefixed_logger.h"
#include "fmt/format.h"

namespace srsran {

class rlc_bearer_log_prefix
{
public:
  rlc_bearer_log_prefix(gnb_du_id_t gnb_du_id, uint32_t ue_index, rb_id_t rb_id, const char* dir)
  {
    fmt::memory_buffer buffer;
    fmt::format_to(std::back_inserter(buffer), "du={} ue={} {} {}: ", fmt::underlying(gnb_du_id), ue_index, rb_id, dir);
    prefix = srsran::to_c_str(buffer);
  }
  const char* to_c_str() const { return prefix.c_str(); }

private:
  std::string prefix;
};

using rlc_bearer_logger = prefixed_logger<rlc_bearer_log_prefix>;

} // namespace srsran

namespace fmt {

// associated formatter
template <>
struct formatter<srsran::rlc_bearer_log_prefix> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::rlc_bearer_log_prefix& o, FormatContext& ctx) const
  {
    return format_to(ctx.out(), "{}", o.to_c_str());
  }
};
} // namespace fmt
