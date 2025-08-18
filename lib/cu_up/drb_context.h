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

#include "adapters/f1u_adapters.h"
#include "adapters/gw_adapters.h"
#include "adapters/pdcp_adapters.h"
#include "qos_flow_context.h"
#include "srsran/f1u/cu_up/f1u_config.h"
#include "srsran/pdcp/pdcp_entity.h"
#include "srsran/ran/rb_id.h"
#include <map>

namespace srsran {
namespace srs_cu_up {

/// \brief DRB context with map to all QoS flows.
struct drb_context {
  drb_context(const drb_id_t& drb_id_) : drb_id(drb_id_) {}

  void stop()
  {
    pdcp->stop();
    f1u->stop();
    f1u_gw_bearer->stop();
  }

  drb_id_t    drb_id;
  gtpu_teid_t f1u_ul_teid;
  f1u_config  f1u_cfg;

  std::unique_ptr<f1u_cu_up_gateway_bearer> f1u_gw_bearer;
  std::unique_ptr<f1u_bearer>               f1u;
  std::unique_ptr<pdcp_entity>              pdcp;

  // Adapter PDCP->SDAP
  // FIXME: Currently, we assume only one DRB per PDU session and only one QoS flow per DRB.
  pdcp_sdap_adapter pdcp_to_sdap_adapter;
  pdcp_f1u_adapter  pdcp_to_f1u_adapter;
  f1u_pdcp_adapter  f1u_to_pdcp_adapter;

  // Adapter PDCP->E1AP
  pdcp_rx_e1ap_adapter pdcp_rx_to_e1ap_adapter;
  pdcp_tx_e1ap_adapter pdcp_tx_to_e1ap_adapter;

  // Adapter F1-U gateway -> NR-U
  f1u_gateway_rx_nru_adapter f1u_gateway_rx_to_nru_adapter;

  uint8_t cell_group_id; /// This can/should be a list of cell groups.

  std::map<qos_flow_id_t, std::unique_ptr<qos_flow_context>> qos_flows; // key is qos_flow_id
};

} // namespace srs_cu_up
} // namespace srsran
