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

#include "apps/services/e2/e2_appconfig.h"
#include "srsran/cu_cp/cu_cp_configuration.h"
#include "srsran/cu_up/cu_up_configuration.h"
#include "srsran/du/du_cell_config.h"
#include "srsran/du/du_high/du_qos_config.h"
#include "srsran/du/du_high/du_srb_config.h"
#include "srsran/gateways/sctp_network_gateway.h"
#include "srsran/mac/mac_config.h"
#include "srsran/phy/upper/upper_phy_factories.h"
#include "srsran/ru/ru_configuration.h"
#include <map>
#include <vector>

#ifdef JBPF_ENABLED
#include "jbpf.h"
#endif


namespace srsran {

struct gnb_appconfig;
struct cu_cp_unit_amf_config;
struct cu_cp_unit_config;
struct cu_up_unit_config;
struct du_high_unit_config;
struct du_high_unit_cell_config;
struct du_low_unit_config;
struct dynamic_du_unit_config;
struct mac_lc_appconfig;
struct rlc_am_appconfig;
struct worker_manager_config;

/// Converts and returns the subcarrier spacing.
subcarrier_spacing generate_subcarrier_spacing(unsigned sc_spacing);

/// Converts and returns the given gnb application configuration to a E2AP Network Gateway configuration.
srsran::sctp_network_connector_config generate_e2ap_nw_config(const e2_appconfig& config, int ppid);

/// Fills the gNB worker manager parameters of the given worker manager configuration.
void fill_gnb_worker_manager_config(worker_manager_config& config, const gnb_appconfig& unit_cfg);

void generate_jbpf_config(const gnb_appconfig& config, struct jbpf_config *jcfg);

} // namespace srsran
