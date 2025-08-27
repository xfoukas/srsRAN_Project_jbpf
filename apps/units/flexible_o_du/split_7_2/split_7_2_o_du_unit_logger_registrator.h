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

#include "apps/units/flexible_o_du/o_du_high/o_du_high_unit_logger_registrator.h"
#include "apps/units/flexible_o_du/o_du_low/du_low_logger_registrator.h"
#include "apps/units/flexible_o_du/split_7_2/helpers/ru_ofh_logger_registrator.h"
#include "split_7_2_o_du_unit_config.h"

namespace srsran {

/// Registers all the loggers for the O-RAN DU split 7.2.
inline void register_split_7_2_o_du_loggers(const split_7_2_o_du_unit_config& config)
{
  register_o_du_high_loggers(config.odu_high_cfg);
  register_du_low_loggers(config.du_low_cfg.loggers);
  register_ru_ofh_loggers(config.ru_cfg.config.loggers);
}

} // namespace srsran
