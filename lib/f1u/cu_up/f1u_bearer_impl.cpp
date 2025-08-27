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

#include "f1u_bearer_impl.h"

using namespace srsran;
using namespace srs_cu_up;

f1u_bearer_impl::f1u_bearer_impl(uint32_t                       ue_index,
                                 drb_id_t                       drb_id_,
                                 const up_transport_layer_info& ul_tnl_info_,
                                 const f1u_config&              config,
                                 f1u_tx_pdu_notifier&           tx_pdu_notifier_,
                                 f1u_rx_delivery_notifier&      rx_delivery_notifier_,
                                 f1u_rx_sdu_notifier&           rx_sdu_notifier_,
                                 timer_factory                  ue_dl_timer_factory,
                                 unique_timer&                  ue_inactivity_timer_,
                                 task_executor&                 dl_exec_,
                                 task_executor&                 ul_exec_) :
  logger("CU-F1-U", {ue_index, drb_id_, ul_tnl_info_}),
  cfg(config),
  tx_pdu_notifier(tx_pdu_notifier_),
  rx_delivery_notifier(rx_delivery_notifier_),
  rx_sdu_notifier(rx_sdu_notifier_),
  ul_tnl_info(ul_tnl_info_),
  dl_exec(ul_exec_),
  ul_exec(ul_exec_),
  dl_notif_timer(ue_dl_timer_factory.create_timer()),
  ue_inactivity_timer(ue_inactivity_timer_)
{
  dl_notif_timer.set(std::chrono::milliseconds(config.dl_t_notif_timer),
                     [this](timer_id_t tid) { on_expired_dl_notif_timer(); });

  auto dispatch_fn = [this](span<nru_ul_message> msg_span) {
    for (nru_ul_message& msg : msg_span) {
      handle_pdu_impl(std::move(msg));
    }
  };
  ul_batched_queue = std::make_unique<nru_ul_batched_queue>(
      config.queue_size, ul_exec, logger.get_basic_logger(), dispatch_fn, config.batch_size);

  logger.log_info("F1-U bearer configured. {}", cfg);
}

void f1u_bearer_impl::handle_pdu(nru_ul_message msg)
{
  if (not ul_batched_queue->try_push(std::move(msg))) {
    if (!cfg.warn_on_drop) {
      logger.log_info("Dropped F1-U PDU, queue is full.");
    } else {
      logger.log_warning("Dropped F1-U PDU, queue is full.");
    }
  }
}

void f1u_bearer_impl::handle_pdu_impl(nru_ul_message msg)
{
  if (stopped) {
    return;
  }
  logger.log_debug("F1-U bearer received PDU");

  // handle T-PDU
  if (msg.t_pdu.has_value() && !msg.t_pdu->empty()) {
    ue_inactivity_timer.run(); // restart inactivity timer due to UL PDU
    logger.log_debug("Delivering T-PDU of size={}", msg.t_pdu->length());
    rx_sdu_notifier.on_new_sdu(std::move(*msg.t_pdu));
  }

  // handle transmit notifications
  if (msg.data_delivery_status.has_value()) {
    nru_dl_data_delivery_status& status = msg.data_delivery_status.value();

    // Desired buffer size
    if (not dl_exec.defer(TRACE_TASK([this, status]() {
          rx_delivery_notifier.on_desired_buffer_size_notification(status.desired_buffer_size_for_drb);
        }))) {
      logger.log_warning("Could not pass desired buffer size notification to PDCP");
    }

    // Highest transmitted PDCP SN
    if (status.highest_transmitted_pdcp_sn.has_value()) {
      uint32_t pdcp_sn = status.highest_transmitted_pdcp_sn.value();
      if (pdcp_sn != notif_highest_transmitted_pdcp_sn) {
        ue_inactivity_timer.run(); // restart inactivity timer due to confirmed transmission of DL PDU
        logger.log_debug("Notifying highest transmitted pdcp_sn={}", pdcp_sn);
        notif_highest_transmitted_pdcp_sn = pdcp_sn;
        if (not dl_exec.defer(
                TRACE_TASK([this, pdcp_sn]() { rx_delivery_notifier.on_transmit_notification(pdcp_sn); }))) {
          logger.log_warning("Could not pass desired buffer size notification to PDCP");
        }
      } else {
        logger.log_debug("Ignored duplicate notification of highest transmitted pdcp_sn={}", pdcp_sn);
      }
    }
    // Highest successfully delivered PDCP SN
    if (status.highest_delivered_pdcp_sn.has_value()) {
      uint32_t pdcp_sn = status.highest_delivered_pdcp_sn.value();
      if (pdcp_sn != notif_highest_delivered_pdcp_sn) {
        logger.log_debug("Notifying highest successfully delivered pdcp_sn={}", pdcp_sn);
        notif_highest_delivered_pdcp_sn = pdcp_sn;
        if (not dl_exec.defer(
                TRACE_TASK([this, pdcp_sn]() { rx_delivery_notifier.on_delivery_notification(pdcp_sn); }))) {
          logger.log_warning("Could not pass highest delivered notification to PDCP");
        }
      } else {
        logger.log_debug("Ignored duplicate notification of highest successfully delivered pdcp_sn={}", pdcp_sn);
      }
    }
    // Highest retransmitted PDCP SN
    if (status.highest_retransmitted_pdcp_sn.has_value()) {
      uint32_t pdcp_sn = status.highest_retransmitted_pdcp_sn.value();
      logger.log_debug("Notifying highest retransmitted pdcp_sn={}", pdcp_sn);
      if (not dl_exec.defer(
              TRACE_TASK([this, pdcp_sn]() { rx_delivery_notifier.on_retransmit_notification(pdcp_sn); }))) {
        logger.log_warning("Could not pass highest retransmitted notification to PDCP");
      }
    }
    // Highest successfully delivered retransmitted PDCP SN
    if (status.highest_delivered_retransmitted_pdcp_sn.has_value()) {
      uint32_t pdcp_sn = status.highest_delivered_retransmitted_pdcp_sn.value();
      logger.log_debug("Notifying highest successfully delivered retransmitted pdcp_sn={}", pdcp_sn);
      if (not dl_exec.defer(TRACE_TASK(
              [this, pdcp_sn]() { rx_delivery_notifier.on_delivery_retransmitted_notification(pdcp_sn); }))) {
        logger.log_warning("Could not pass highest retransmitted notification to PDCP");
      }
    }
  }
}

void f1u_bearer_impl::handle_sdu(byte_buffer sdu, bool is_retx)
{
  if (stopped) {
    return;
  }

  logger.log_debug("F1-U bearer received SDU. size={} is_retx={}", sdu.length(), is_retx);
  nru_dl_message msg = {};

  // attach the SDU
  msg.t_pdu = std::move(sdu);

  // set retransmission flag
  msg.dl_user_data.retransmission_flag = is_retx;

  // attach discard blocks (if any)
  fill_discard_blocks(msg);

  // stop backoff timer
  dl_notif_timer.stop();

  tx_pdu_notifier.on_new_pdu(std::move(msg));
}

void f1u_bearer_impl::discard_sdu(uint32_t pdcp_sn)
{
  // start backoff timer
  if (!dl_notif_timer.is_running()) {
    dl_notif_timer.run();
  }
  if (discard_blocks.empty()) {
    discard_blocks.push_back(nru_pdcp_sn_discard_block{});
    nru_pdcp_sn_discard_block& block = discard_blocks.back();
    block.pdcp_sn_start              = pdcp_sn;
    block.block_size                 = 1;
    logger.log_debug("Queued first SDU discard block with pdcp_sn={}", pdcp_sn);
  } else {
    nru_pdcp_sn_discard_block& last_block = discard_blocks.back();
    if (last_block.pdcp_sn_start + last_block.block_size == pdcp_sn) {
      last_block.block_size++;
      logger.log_debug("Expanded previous SDU discard block with pdcp_sn={}. pdcp_sn_start={} block_size={}",
                       pdcp_sn,
                       last_block.pdcp_sn_start,
                       last_block.block_size);
    } else {
      discard_blocks.push_back(nru_pdcp_sn_discard_block{});
      nru_pdcp_sn_discard_block& block = discard_blocks.back();
      block.pdcp_sn_start              = pdcp_sn;
      block.block_size                 = 1;
      logger.log_debug("Queued next SDU discard block with pdcp_sn={}", pdcp_sn);
      if (discard_blocks.full()) {
        logger.log_debug("Flushing SDU discard block notification. nof_blocks={}", discard_blocks.capacity());
        flush_discard_blocks();
      }
    }
  }
}

void f1u_bearer_impl::fill_discard_blocks(nru_dl_message& msg)
{
  if (!discard_blocks.empty()) {
    msg.dl_user_data.discard_blocks = std::move(discard_blocks);
    discard_blocks                  = {};
  }
}

void f1u_bearer_impl::on_expired_dl_notif_timer()
{
  if (stopped) {
    return;
  }
  logger.log_debug("DL notification timer expired");
  flush_discard_blocks();
}

void f1u_bearer_impl::flush_discard_blocks()
{
  nru_dl_message msg = {};

  // attach discard blocks (if any)
  fill_discard_blocks(msg);

  // stop backoff timer
  dl_notif_timer.stop();

  if (msg.dl_user_data.discard_blocks.has_value()) {
    logger.log_debug("Sending discard blocks");
    tx_pdu_notifier.on_new_pdu(std::move(msg));
  }
}
