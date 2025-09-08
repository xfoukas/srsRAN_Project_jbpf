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

#include "du_high/du_high_config_yaml_writer.h"
#include "e2/o_du_high_e2_config_yaml_writer.h"
#include "fapi/fapi_config_yaml_writer.h"
#include "o_du_high_unit_config.h"
#include <yaml-cpp/yaml.h>

namespace srsran {

/// Fills the given node with the O-DU high configuration values.
inline void fill_o_du_high_config_in_yaml_schema(YAML::Node& node, const o_du_high_unit_config& config)
{
  fill_du_high_config_in_yaml_schema(node, config.du_high_cfg.config);
  fill_fapi_config_in_yaml_schema(node, config.fapi_cfg);
  fill_o_du_high_e2_config_in_yaml_schema(node, config.e2_cfg);
}

} // namespace srsran
