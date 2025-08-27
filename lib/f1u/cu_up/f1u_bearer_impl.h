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

#include "srsran/adt/batched_dispatch_queue.h"
#include "srsran/f1u/cu_up/f1u_bearer.h"
#include "srsran/f1u/cu_up/f1u_bearer_logger.h"
#include "srsran/f1u/cu_up/f1u_config.h"
#include "srsran/f1u/cu_up/f1u_rx_delivery_notifier.h"
#include "srsran/f1u/cu_up/f1u_rx_sdu_notifier.h"
#include "srsran/f1u/cu_up/f1u_tx_pdu_notifier.h"
#include "srsran/ran/rb_id.h"
#include "srsran/support/timers.h"

namespace srsran::srs_cu_up {

class f1u_bearer_impl final : public f1u_bearer, public f1u_rx_pdu_handler, public f1u_tx_sdu_handler
{
public:
  f1u_bearer_impl(uint32_t                       ue_index,
                  drb_id_t                       drb_id_,
                  const up_transport_layer_info& ul_tnl_info_,
                  const f1u_config&              config,
                  f1u_tx_pdu_notifier&           tx_pdu_notifier_,
                  f1u_rx_delivery_notifier&      rx_delivery_notifier_,
                  f1u_rx_sdu_notifier&           rx_sdu_notifier_,
                  timer_factory                  ue_ctrl_timer_factory,
                  unique_timer&                  ue_inactivity_timer_,
                  task_executor&                 dl_exec_,
                  task_executor&                 ul_exec_);
  f1u_bearer_impl(const f1u_bearer_impl&)            = delete;
  f1u_bearer_impl& operator=(const f1u_bearer_impl&) = delete;

  ~f1u_bearer_impl() override { stop(); }

  f1u_rx_pdu_handler& get_rx_pdu_handler() override { return *this; }
  f1u_tx_sdu_handler& get_tx_sdu_handler() override { return *this; }

  void stop() override
  {
    if (stopped) {
      return;
    }
    stopped = true;
    dl_notif_timer.stop();
  }

  void handle_pdu(nru_ul_message msg) override;
  void handle_sdu(byte_buffer sdu, bool is_retx) override;
  void discard_sdu(uint32_t pdcp_sn) override;

  /// \brief Returns the UL tunnel info that was assigned upon construction.
  const up_transport_layer_info& get_ul_tnl_info() const { return ul_tnl_info; }

  /// \brief Returns the UL TEID that was assigned upon construction.
  /// \return The UL TEID associated with this bearer.
  gtpu_teid_t get_ul_teid() const { return ul_tnl_info.gtp_teid; }

  /// \brief This function handles the periodic transmission of downlink notification towards lower layers with all
  /// discard blocks that have aggregated since the previous DL PDU.
  void on_expired_dl_notif_timer();

private:
  f1u_bearer_logger logger;

  /// Config storage
  const f1u_config cfg;

  f1u_tx_pdu_notifier&      tx_pdu_notifier;
  f1u_rx_delivery_notifier& rx_delivery_notifier;
  f1u_rx_sdu_notifier&      rx_sdu_notifier;
  up_transport_layer_info   ul_tnl_info;
  task_executor&            dl_exec;
  task_executor&            ul_exec;

  using nru_ul_batched_queue = batched_dispatch_queue<nru_ul_message>;
  std::unique_ptr<nru_ul_batched_queue> ul_batched_queue;

  bool stopped = false;

  /// Sentinel value representing a not-yet set PDCP SN
  static constexpr uint32_t unset_pdcp_sn = UINT32_MAX;

  /// Downlink notification timer that triggers periodic transmission of discard blocks towards lower layers. The
  /// purpose of this timer is to avoid excessive downlink notifications for every PDCP SN that is discarded by upper
  /// layers.
  unique_timer dl_notif_timer;

  /// UE inactivity timer that is injected from parent entities. The timer must run in the UL executor!
  /// The timer shall be restarted on each UL PDU (= UL activity) and on each transmit notification (= DL activity).
  unique_timer& ue_inactivity_timer;

  /// Holds the last highest transmitted PDCP SN that was reported to upper layers
  uint32_t notif_highest_transmitted_pdcp_sn = unset_pdcp_sn;
  /// Holds the last highest delivered PDCP SN that was reported to upper layers
  uint32_t notif_highest_delivered_pdcp_sn = unset_pdcp_sn;

  /// Collection of pending \c nru_pdcp_sn_discard_block objects.
  nru_pdcp_sn_discard_blocks discard_blocks;

  void handle_pdu_impl(nru_ul_message msg);

  /// \brief Fills the provided \c nru_dl_message with all SDU discard blocks that have been aggregated since the last
  /// call of this function (or since creation of this object).
  /// \param msg The message to be filled with SDU discard blocks.
  void fill_discard_blocks(nru_dl_message& msg);

  /// \brief Immediately transmits a \c nru_dl_message with all currently aggregated SDU discard blocks, if any.
  void flush_discard_blocks();
};

} // namespace srsran::srs_cu_up
