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

#include "apps/services/metrics/metrics_consumer.h"
#include "apps/services/metrics/metrics_properties.h"
#include "apps/services/metrics/metrics_set.h"
#include "flexible_o_du_metrics.h"
#include "srsran/adt/span.h"
#include "srsran/adt/unique_function.h"
#include "srsran/srslog/logger.h"
#include "srsran/support/executors/task_executor.h"

namespace srsran {

/// Flexible O-RAN DU metrics properties implementation.
class flexible_o_du_app_service_metrics_properties_impl : public app_services::metrics_properties
{
public:
  std::string_view name() const override { return "Flexible O-DU metrics"; }
};

/// Application service metrics for the flexible O-DU.
class flexible_o_du_app_service_metrics_impl : public app_services::metrics_set
{
  flexible_o_du_app_service_metrics_properties_impl properties;
  flexible_o_du_metrics                             metrics;

public:
  explicit flexible_o_du_app_service_metrics_impl(const flexible_o_du_metrics& metrics_) : metrics(metrics_) {}

  // See interface for documentation.
  const app_services::metrics_properties& get_properties() const override { return properties; }

  // Returns the flexible O-DU metrics.
  const flexible_o_du_metrics& get_metrics() const { return metrics; }
};

/// Callback for the flexible O-DU metrics.
inline auto flexible_o_du_metrics_callback = [](const app_services::metrics_set&      report,
                                                span<app_services::metrics_consumer*> consumers,
                                                task_executor&                        executor,
                                                srslog::basic_logger&                 logger) {
  const auto& metric = static_cast<const flexible_o_du_app_service_metrics_impl&>(report);

  if (!executor.defer(TRACE_TASK([metric, consumers]() {
        for (auto& consumer : consumers) {
          consumer->handle_metric(metric);
        }
      }))) {
    logger.error("Failed to dispatch the metric '{}'", metric.get_properties().name());
  }
};

} // namespace srsran
