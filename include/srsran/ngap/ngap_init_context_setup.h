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
#include "srsran/security/security.h"

namespace srsran {
namespace srs_cu_cp {

struct ngap_init_context_setup_request {
  ue_index_t                               ue_index = ue_index_t::invalid;
  std::optional<std::string>               old_amf;
  std::optional<ngap_ue_aggr_max_bit_rate> ue_aggr_max_bit_rate;
  // TODO: Add optional core_network_assist_info_for_inactive
  guami_t                                                 guami;
  std::optional<cu_cp_pdu_session_resource_setup_request> pdu_session_res_setup_list_cxt_req;
  std::vector<s_nssai_t>                                  allowed_nssai;
  security::security_context                              security_context;
  // TODO: Add optional trace_activation
  // TODO: Add optional mob_restrict_list
  std::optional<byte_buffer> ue_radio_cap;
  std::optional<uint16_t>    idx_to_rfsp;
  std::optional<uint64_t>    masked_imeisv;
  std::optional<byte_buffer> nas_pdu;
  // TODO: Add optional emergency_fallback_ind
  // TODO: Add optional rrc_inactive_transition_report_request
  std::optional<cu_cp_ue_radio_cap_for_paging> ue_radio_cap_for_paging;
  // TODO: Add optional redirection_voice_fallback
  // TODO: Add optional location_report_request_type
  // TODO: Add optional cn_assisted_ran_tuning
};

struct ngap_init_context_setup_failure {
  ngap_cause_t                                                                 cause;
  slotted_id_vector<pdu_session_id_t, cu_cp_pdu_session_res_setup_failed_item> pdu_session_res_failed_to_setup_items;
  std::optional<crit_diagnostics_t>                                            crit_diagnostics;
};

struct ngap_init_context_setup_response {
  slotted_id_vector<pdu_session_id_t, cu_cp_pdu_session_res_setup_response_item> pdu_session_res_setup_response_items;
  slotted_id_vector<pdu_session_id_t, cu_cp_pdu_session_res_setup_failed_item>   pdu_session_res_failed_to_setup_items;
  std::optional<crit_diagnostics_t>                                              crit_diagnostics;
};

} // namespace srs_cu_cp
} // namespace srsran
