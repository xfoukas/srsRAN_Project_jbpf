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

#include "srsran/adt/byte_buffer.h"
#include "srsran/adt/static_vector.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/cu_cp/up_context.h"
#include "srsran/rrc/meas_types.h"
#include "srsran/security/security.h"
#include <optional>

namespace srsran {
namespace srs_cu_cp {

/// \brief RRC context transfered from one UE object to the other during mobility.
struct rrc_ue_transfer_context {
  security::security_context            sec_context;
  std::optional<rrc_meas_cfg>           meas_cfg;
  up_context                            up_ctx;
  static_vector<srb_id_t, MAX_NOF_SRBS> srbs; // List of active SRBs (TODO: add PDCP config).
  byte_buffer                           handover_preparation_info;
  byte_buffer                           ue_cap_rat_container_list;
  bool                                  is_inter_cu_handover = false;
};

} // namespace srs_cu_cp
} // namespace srsran
