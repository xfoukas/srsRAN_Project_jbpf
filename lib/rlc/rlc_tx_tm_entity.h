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

#include "rlc_sdu_queue_lockfree.h"
#include "rlc_tx_entity.h"
#include "srsran/support/executors/task_executor.h"

namespace srsran {

class rlc_tx_tm_entity : public rlc_tx_entity
{
private:
  // Config storage
  const rlc_tx_tm_config cfg;

  rlc_sdu_queue_lockfree sdu_queue;
  rlc_sdu                sdu;

  pcap_rlc_pdu_context pcap_context;

  /// This atomic_flag indicates whether a buffer state update task has been queued but not yet run by pcell_executor.
  /// It helps to avoid queuing of redundant notification tasks in case of frequent changes of the buffer status.
  /// If the flag is set, no further notification needs to be scheduled, because the already queued task will pick the
  /// latest buffer state upon execution.
  std::atomic_flag pending_buffer_state = ATOMIC_FLAG_INIT;

public:
  rlc_tx_tm_entity(gnb_du_id_t                          du_id_,
                   du_ue_index_t                        ue_index_,
                   rb_id_t                              rb_id_,
                   const rlc_tx_tm_config&              config,
                   rlc_tx_upper_layer_data_notifier&    upper_dn_,
                   rlc_tx_upper_layer_control_notifier& upper_cn_,
                   rlc_tx_lower_layer_notifier&         lower_dn_,
                   rlc_metrics_aggregator&              metrics_agg_,
                   rlc_pcap&                            pcap_,
                   task_executor&                       pcell_executor_,
                   task_executor&                       ue_executor_,
                   timer_manager&                       timers);

  ~rlc_tx_tm_entity() override { stop(); }

  void stop() final
  {
    // Stop all timers. Any queued handlers of timers that just expired before this call are canceled automatically
    if (not stopped) {
      high_metrics_timer.stop();
      low_metrics_timer.stop();
      stopped = true;
    }
  }

  // Interfaces for higher layers
  void handle_sdu(byte_buffer sdu_buf, bool is_retx) override;
  void discard_sdu(uint32_t pdcp_sn) override;

  // Interfaces for lower layers
  size_t   pull_pdu(span<uint8_t> rlc_pdu_buf) override;
  uint32_t get_buffer_state() override;

private:
  /// Called whenever the buffer state has been changed by upper layers (new SDUs or SDU discard) so that lower layers
  /// need to be informed about the new buffer state. This function defers the actual notification \c
  /// handle_changed_buffer_state to pcell_executor. The notification is discarded if another notification is already
  /// queued for execution. This function should not be called from \c pull_pdu, since the lower layer accounts for the
  /// amount of extracted data itself.
  ///
  /// Safe execution from: Any executor
  void handle_changed_buffer_state();

  /// Immediately informs the lower layer of the current buffer state. This function is called from pcell_executor and
  /// its execution is queued by \c handle_changed_buffer_state.
  ///
  /// Safe execution from: pcell_executor
  void update_mac_buffer_state();

  bool stopped = false;
};

} // namespace srsran
