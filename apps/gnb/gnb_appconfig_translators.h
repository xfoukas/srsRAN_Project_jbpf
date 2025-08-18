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

#ifdef JBPF_ENABLED
#include "jbpf.h"
#endif


namespace srsran {

struct gnb_appconfig;
struct worker_manager_config;

/// Fills the gNB worker manager parameters of the given worker manager configuration.
void fill_gnb_worker_manager_config(worker_manager_config& config, const gnb_appconfig& unit_cfg);

void generate_jbpf_config(const gnb_appconfig& config, struct jbpf_config *jcfg);

} // namespace srsran
