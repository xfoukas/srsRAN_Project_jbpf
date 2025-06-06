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

#include "rlc_rx_tm_entity.h"

#ifdef JBPF_ENABLED
#include "jbpf_srsran_hooks.h"
#endif

using namespace srsran;

rlc_rx_tm_entity::rlc_rx_tm_entity(gnb_du_id_t                       gnb_du_id_,
                                   du_ue_index_t                     ue_index_,
                                   rb_id_t                           rb_id_,
                                   const rlc_rx_tm_config&           config,
                                   rlc_rx_upper_layer_data_notifier& upper_dn_,
                                   rlc_metrics_aggregator&           metrics_agg_,
                                   rlc_pcap&                         pcap_,
                                   task_executor&                    ue_executor,
                                   timer_manager&                    timers) :
  rlc_rx_entity(gnb_du_id_, ue_index_, rb_id_, upper_dn_, metrics_agg_, pcap_, ue_executor, timers),
  cfg(config),
  pcap_context(ue_index, rb_id_, /* is_uplink */ true)
{
  metrics.metrics_set_mode(rlc_mode::tm);
  logger.log_info("RLC TM created. {}", cfg);

#ifdef JBPF_ENABLED
  {
    int rb_id_value = rb_id.is_srb() ? srb_id_to_uint(rb_id.get_srb_id()) 
                                    : drb_id_to_uint(rb_id.get_drb_id());
    struct jbpf_rlc_ctx_info ctx_info = {0, (uint64_t)gnb_du_id, ue_index, rb_id.is_srb(), 
      (uint8_t)rb_id_value, JBPF_RLC_MODE_TM};
    hook_rlc_ul_creation(&ctx_info);
  }
#endif  
}

void rlc_rx_tm_entity::handle_pdu(byte_buffer_slice buf)
{
  size_t pdu_len = buf.length();
  metrics.metrics_add_pdus(1, pdu_len);

  pcap.push_pdu(pcap_context, buf);

  expected<byte_buffer_chain> sdu = byte_buffer_chain::create(std::move(buf));
  if (!sdu) {
    logger.log_error("Dropped SDU, failed to create SDU buffer. sdu_len={}", pdu_len);
    metrics.metrics_add_lost_pdus(1);
    return;
  }

#ifdef JBPF_ENABLED
      {
        int rb_id_value = rb_id.is_srb() ? srb_id_to_uint(rb_id.get_srb_id()) 
                                        : drb_id_to_uint(rb_id.get_drb_id());
        struct jbpf_rlc_ctx_info ctx_info = {0, (uint64_t)gnb_du_id, ue_index, rb_id.is_srb(), 
          (uint8_t)rb_id_value, JBPF_RLC_MODE_TM};
        hook_rlc_ul_sdu_delivered(&ctx_info, 0, 0, sdu.value().length());
      }
#endif

  logger.log_info(sdu.value().begin(), sdu.value().end(), "RX SDU. sdu_len={}", sdu.value().length());
  metrics.metrics_add_sdus(1, sdu.value().length());
  upper_dn.on_new_sdu(std::move(sdu.value()));
}
