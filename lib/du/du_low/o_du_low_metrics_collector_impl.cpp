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

#include "o_du_low_metrics_collector_impl.h"
#include "srsran/du/du_low/du_low_metrics_collector.h"
#include "srsran/du/du_low/o_du_low_metrics.h"

using namespace srsran;
using namespace srs_du;

void o_du_low_metrics_collector_impl::collect_metrics(o_du_low_metrics& metrics)
{
  if (collector == nullptr) {
    return;
  }

  // Collect DU low metrics.
  collector->collect_metrics(metrics.du_lo_metrics);
}
