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

#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/ngap/ngap_types.h"
#include "srsran/ran/crit_diagnostics.h"
#include <variant>

namespace srsran {
namespace srs_cu_cp {

// enum class ngap_handov_type { intra5gs = 0, fivegs_to_eps, eps_to_5gs, fivegs_to_utran };

struct ngap_broadcast_plmn_item {
  plmn_identity                     plmn_id = plmn_identity::test_value();
  std::vector<slice_support_item_t> tai_slice_support_list;
};

struct ngap_supported_ta_item {
  tac_t                                 tac;
  std::vector<ngap_broadcast_plmn_item> broadcast_plmn_list;
};

struct ngap_served_guami_item {
  guami_t                    guami;
  std::optional<std::string> backup_amf_name;
};

struct ngap_plmn_support_item {
  std::string                       plmn_id;
  std::vector<slice_support_item_t> slice_support_list;
};

struct ngap_ng_setup_response {
  std::string                         amf_name;
  std::vector<ngap_served_guami_item> served_guami_list;
  uint16_t                            relative_amf_capacity;
  std::vector<ngap_plmn_support_item> plmn_support_list;
  std::optional<crit_diagnostics_t>   crit_diagnostics;
  // TODO: Add optional ue_retention_info;
  // TODO: Add optional iab_supported;
  // TODO: Add optional extended_amf_name;
};

struct ngap_ng_setup_failure {
  ngap_cause_t                      cause;
  std::optional<crit_diagnostics_t> crit_diagnostics;
};

using ngap_ng_setup_result = std::variant<ngap_ng_setup_response, ngap_ng_setup_failure>;

} // namespace srs_cu_cp
} // namespace srsran
