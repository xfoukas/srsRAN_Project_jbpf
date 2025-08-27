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

#include "srsran/cu_up/cu_up_types.h"
#include "srsran/gtpu/gtpu_teid.h"
#include "srsran/support/format/fmt_to_c_str.h"
#include "srsran/support/format/prefixed_logger.h"
#include "fmt/format.h"

namespace srsran {

class gtpu_tunnel_log_prefix
{
public:
  gtpu_tunnel_log_prefix(std::optional<srs_cu_up::ue_index_t> ue_index, gtpu_teid_t teid, const char* dir)
  {
    fmt::memory_buffer buffer;
    if (ue_index.has_value()) {
      fmt::format_to(std::back_inserter(buffer), "ue={} {} teid={}: ", fmt::underlying(*ue_index), dir, teid);
    } else {
      fmt::format_to(std::back_inserter(buffer), "{} teid={}: ", dir, teid);
    }
    prefix = srsran::to_c_str(buffer);
  }
  const char* to_c_str() const { return prefix.c_str(); }

private:
  std::string prefix;
};

using gtpu_tunnel_logger = prefixed_logger<gtpu_tunnel_log_prefix>;

} // namespace srsran

namespace fmt {

// associated formatter
template <>
struct formatter<srsran::gtpu_tunnel_log_prefix> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::gtpu_tunnel_log_prefix o, FormatContext& ctx) const
  {
    return format_to(ctx.out(), "{}", o.to_c_str());
  }
};
} // namespace fmt
