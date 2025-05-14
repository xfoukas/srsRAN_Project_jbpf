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

#include "rlc_rx_entity.h"
#include "srsran/rlc/rlc_metrics.h"

#ifdef JBPF_ENABLED
#include "jbpf_srsran_hooks.h"
#endif

namespace srsran {

class rlc_rx_tm_entity : public rlc_rx_entity
{
public:
  rlc_rx_tm_entity(gnb_du_id_t                       gnb_du_id,
                   du_ue_index_t                     ue_index,
                   rb_id_t                           rb_id,
                   const rlc_rx_tm_config&           config,
                   rlc_rx_upper_layer_data_notifier& upper_dn_,
                   rlc_metrics_aggregator&           metrics_agg_,
                   rlc_pcap&                         pcap_,
                   task_executor&                    ue_executor,
                   timer_manager&                    timers);

#ifdef JBPF_ENABLED
  ~rlc_rx_tm_entity() override {
    {
      int rb_id_value = rb_id.is_srb() ? srb_id_to_uint(rb_id.get_srb_id()) 
                                      : drb_id_to_uint(rb_id.get_drb_id());
      struct jbpf_rlc_ctx_info ctx_info = {0, (uint64_t)gnb_du_id, ue_index, rb_id.is_srb(), 
        (uint8_t)rb_id_value, JBPF_RLC_MODE_TM};
      hook_rlc_ul_deletion(&ctx_info);
    }
  }
#endif

  void stop() final
  {
    // There are no timers to be stopped here.
  }

  // Interfaces for higher layers
  void handle_pdu(byte_buffer_slice buf) override;

private:
  // Config storage
  const rlc_rx_tm_config cfg;

  pcap_rlc_pdu_context pcap_context;
};

} // namespace srsran
