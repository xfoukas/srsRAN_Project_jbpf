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

#include "srsran/f1u/du/f1u_bearer.h"
#include "srsran/f1u/du/f1u_config.h"
#include "srsran/f1u/du/f1u_tx_pdu_notifier.h"
#include "srsran/ran/gnb_du_id.h"
#include "srsran/ran/qos/five_qi.h"
#include "srsran/ran/rb_id.h"
#include "srsran/ran/up_transport_layer_info.h"
#include "srsran/support/timers.h"

namespace srsran::srs_du {

/// This class provides a notifier for the RX bearer
/// inside the DU F1-U gateway. This provides an adapter
/// to the NR-U bearer to pass SDUs into.
class f1u_du_gateway_bearer_rx_notifier
{
public:
  virtual ~f1u_du_gateway_bearer_rx_notifier() = default;

  virtual void on_new_pdu(nru_dl_message msg) = 0;
};

/// This class provides the interface for an F1-U GW bearer
class f1u_du_gateway_bearer : public srs_du::f1u_tx_pdu_notifier
{
  virtual void stop() = 0;

public:
  virtual expected<std::string> get_bind_address() const = 0;
};

/// This class will be used to provide the interfaces to
/// the DU to create and manage F1-U bearers.
class f1u_du_gateway : public srs_du::f1u_bearer_disconnector
{
public:
  f1u_du_gateway()                                 = default;
  ~f1u_du_gateway() override                       = default;
  f1u_du_gateway(const f1u_du_gateway&)            = default;
  f1u_du_gateway& operator=(const f1u_du_gateway&) = default;
  f1u_du_gateway(f1u_du_gateway&&)                 = default;
  f1u_du_gateway& operator=(f1u_du_gateway&&)      = default;

  virtual std::unique_ptr<f1u_du_gateway_bearer> create_du_bearer(uint32_t                       ue_index,
                                                                  drb_id_t                       drb_id,
                                                                  five_qi_t                      five_qi,
                                                                  srs_du::f1u_config             config,
                                                                  const gtpu_teid_t&             dl_up_tnl_info,
                                                                  const up_transport_layer_info& ul_up_tnl_info,
                                                                  srs_du::f1u_du_gateway_bearer_rx_notifier& du_rx,
                                                                  timer_factory                              timers,
                                                                  task_executor& ue_executor) = 0;

  virtual expected<std::string> get_du_bind_address(gnb_du_id_t gnb_du_id) const = 0;
};

/// This class will be used to provide the interfaces to
/// the CU-UP to create and manage F1-U bearers.
class f1u_du_udp_gateway : public f1u_du_gateway
{
public:
  f1u_du_udp_gateway()                                     = default;
  ~f1u_du_udp_gateway() override                           = default;
  f1u_du_udp_gateway(const f1u_du_udp_gateway&)            = default;
  f1u_du_udp_gateway& operator=(const f1u_du_udp_gateway&) = default;
  f1u_du_udp_gateway(f1u_du_udp_gateway&&)                 = default;
  f1u_du_udp_gateway& operator=(f1u_du_udp_gateway&&)      = default;

  virtual std::optional<uint16_t> get_bind_port() const = 0;
};

} // namespace srsran::srs_du
