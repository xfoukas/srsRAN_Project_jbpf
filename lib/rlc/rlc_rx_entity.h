/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
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

#include "rlc_bearer_logger.h"
#include "rlc_metrics_aggregator.h"
#include "rlc_rx_metrics_container.h"
#include "srsran/pcap/rlc_pcap.h"
#include "srsran/rlc/rlc_rx.h"

namespace srsran {

/// Base class used for receiving RLC bearers.
/// It provides interfaces for the RLC bearers, for the lower layers
class rlc_rx_entity : public rlc_rx_lower_layer_interface
{
protected:
  rlc_rx_entity(gnb_du_id_t                       gnb_du_id_,
                du_ue_index_t                     ue_index_,
                rb_id_t                           rb_id_,
                rlc_rx_upper_layer_data_notifier& upper_dn_,
                rlc_metrics_aggregator&           metrics_agg_,
                rlc_pcap&                         pcap_,
                task_executor&                    ue_executor_,
                timer_manager&                    timers) :
    logger("RLC", {gnb_du_id_, ue_index_, rb_id_, "UL"}),
    gnb_du_id(gnb_du_id_),
    ue_index(ue_index_),
    rb_id(rb_id_),
    upper_dn(upper_dn_),
    metrics(metrics_agg_.get_metrics_period().count()),
    pcap(pcap_),
    ue_timer_factory{timers, ue_executor_},
    high_metrics_timer(ue_timer_factory.create_timer()),
    metrics_agg(metrics_agg_)
  {
    if (metrics_agg.get_metrics_period().count()) {
      high_metrics_timer.set(std::chrono::milliseconds(metrics_agg.get_metrics_period().count()),
                             [this](timer_id_t tid) {
                               metrics_agg.push_rx_high_metrics(metrics.get_and_reset_metrics());
                               high_metrics_timer.run();
                             });
      high_metrics_timer.run();
    }
  }

  rlc_bearer_logger                 logger;
  gnb_du_id_t                       gnb_du_id;
  du_ue_index_t                     ue_index;
  rb_id_t                           rb_id;
  rlc_rx_upper_layer_data_notifier& upper_dn;
  rlc_rx_metrics_container          metrics;
  rlc_pcap&                         pcap;
  timer_factory                     ue_timer_factory;

  unique_timer high_metrics_timer;

private:
  rlc_metrics_aggregator& metrics_agg;

public:
  /// \brief Stops all internal timers.
  ///
  /// This function is inteded to be called upon removal of the bearer before destroying it.
  /// It stops all timers with handlers that may delegate tasks to another executor that could face a deleted object at
  /// a later execution time.
  /// Before this function is called, the adjacent layers should already be disconnected so that no timer is restarted.
  ///
  /// Note: This function shall only be called from ue_executor.
  virtual void stop() = 0;

  rlc_rx_metrics get_metrics() { return metrics.get_metrics(); }
};

} // namespace srsran
