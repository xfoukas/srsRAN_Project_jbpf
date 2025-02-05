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

#include "srsran/adt/span.h"
#include "srsran/du/du_low/du_low_wrapper_config.h"

namespace srsran {

namespace srs_du {
struct du_cell_config;
}

struct du_low_unit_config;
struct worker_manager;

hal_upper_phy_config make_du_low_hal_config_and_dependencies(const du_low_unit_config& du_low_unit_cfg,
                                                             unsigned                  nof_cells);

void make_du_low_wrapper_config_and_dependencies(srs_du::du_low_wrapper_config&              out_cfg,
                                                 const du_low_unit_config&                   du_low_unit_cfg,
                                                 std::vector<srs_du::cell_prach_ports_entry> prach_ports,
                                                 span<const srs_du::du_cell_config>          du_cells,
                                                 span<const unsigned>                        max_puschs_per_slot,
                                                 upper_phy_rg_gateway&                       rg_gateway,
                                                 upper_phy_rx_symbol_request_notifier&       rx_symbol_request_notifier,
                                                 worker_manager&                             workers,
                                                 unsigned                                    du_id,
                                                 const hal_upper_phy_config&                 hal_config);

} // namespace srsran
