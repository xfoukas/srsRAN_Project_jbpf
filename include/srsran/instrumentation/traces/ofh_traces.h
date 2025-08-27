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

#include "srsran/support/tracing/event_tracing.h"

namespace srsran {

/// Set to true for enabling OFH trace.
#ifndef SRSRAN_OFH_TRACE
constexpr bool OFH_TRACE_ENABLED = false;
#else
constexpr bool OFH_TRACE_ENABLED = true;
#endif

/// OFH event tracing. This tracer is used to analyze latencies in the OFH processing.
extern file_event_tracer<OFH_TRACE_ENABLED> ofh_tracer;

} // namespace srsran
