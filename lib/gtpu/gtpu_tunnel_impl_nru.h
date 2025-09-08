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

#include "gtpu_tunnel_nru_rx_impl.h"
#include "gtpu_tunnel_nru_tx_impl.h"
#include "srsran/gtpu/gtpu_config.h"
#include "srsran/gtpu/gtpu_tunnel_nru.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/srslog/logger.h"

namespace srsran {

class gtpu_tunnel_nru_impl : public gtpu_tunnel_nru
{
public:
  gtpu_tunnel_nru_impl(srs_cu_up::ue_index_t                       ue_index,
                       gtpu_tunnel_nru_config                      cfg,
                       dlt_pcap&                                   gtpu_pcap,
                       gtpu_tunnel_nru_rx_lower_layer_notifier&    rx_lower,
                       gtpu_tunnel_common_tx_upper_layer_notifier& tx_upper) :
    logger(srslog::fetch_basic_logger("GTPU"))
  {
    rx = std::make_unique<gtpu_tunnel_nru_rx_impl>(ue_index, cfg.rx, rx_lower);
    tx = std::make_unique<gtpu_tunnel_nru_tx_impl>(ue_index, cfg.tx, gtpu_pcap, tx_upper);
  }
  ~gtpu_tunnel_nru_impl() override = default;

  gtpu_tunnel_common_rx_upper_layer_interface* get_rx_upper_layer_interface() final { return rx.get(); }
  gtpu_tunnel_nru_tx_lower_layer_interface*    get_tx_lower_layer_interface() final { return tx.get(); }

private:
  srslog::basic_logger& logger;

  std::unique_ptr<gtpu_tunnel_nru_rx_impl> rx = {};
  std::unique_ptr<gtpu_tunnel_nru_tx_impl> tx = {};
};
} // namespace srsran
