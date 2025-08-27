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

#include "srsran/ran/cause/f1ap_cause_converters.h"
#include "srsran/support/error_handling.h"
#include <variant>

using namespace srsran;

static constexpr uint8_t f1ap_to_ngap_cause_radio_network[] = {
    (uint8_t)ngap_cause_radio_network_t::unspecified,                 // unspecified
    (uint8_t)ngap_cause_radio_network_t::radio_conn_with_ue_lost,     // rl_fail_rlc
    (uint8_t)ngap_cause_radio_network_t::unspecified,                 // unknown_or_already_allocated_gnb_cu_ue_f1ap_id
    (uint8_t)ngap_cause_radio_network_t::unspecified,                 // unknown_or_already_allocated_gnb_du_ue_f1ap_id
    (uint8_t)ngap_cause_radio_network_t::unspecified,                 // unknown_or_inconsistent_pair_of_ue_f1ap_id
    (uint8_t)ngap_cause_radio_network_t::interaction_with_other_proc, // interaction_with_other_proc
    (uint8_t)ngap_cause_radio_network_t::unspecified,                 // not_supported_qci_value
    (uint8_t)ngap_cause_radio_network_t::unspecified,                 // action_desirable_for_radio_reasons
    (uint8_t)ngap_cause_radio_network_t::no_radio_res_available_in_target_cell, // no_radio_res_available
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // proc_cancelled
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // normal_release
    (uint8_t)ngap_cause_radio_network_t::cell_not_available,                    // cell_not_available
    (uint8_t)ngap_cause_radio_network_t::radio_conn_with_ue_lost,               // rl_fail_others
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // ue_rejection
    (uint8_t)ngap_cause_radio_network_t::res_not_available_for_the_slice,       // res_not_available_for_the_slice
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // amf_initiated_abnormal_release
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // release_due_to_pre_emption
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // plmn_not_served_by_the_gnb_cu
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // multiple_drb_id_instances
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // unknown_drb_id
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // multiple_bh_rlc_ch_id_instances
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // unknown_bh_rlc_ch_id
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // cho_cpc_res_tobechanged
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // npn_not_supported
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // npn_access_denied
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // gnb_cu_cell_capacity_exceeded
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // report_characteristics_empty
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // existing_meas_id
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // meas_temporarily_not_available
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // meas_not_supported_for_the_obj
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // unknown_bh_address
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // unknown_bap_routing_id
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // insufficient_ue_cap
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // scg_activation_deactivation_fail
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // scg_deactivation_fail_due_to_data_tx
    (uint8_t)ngap_cause_radio_network_t::unspecified,                           // requested_item_not_supported_on_time
    (uint8_t)ngap_cause_radio_network_t::unspecified, // unknown_or_already_allocated_gnb_cu_mbs_f1ap_id
    (uint8_t)ngap_cause_radio_network_t::unspecified, // unknown_or_already_allocated_gnb_du_mbs_f1ap_id
    (uint8_t)ngap_cause_radio_network_t::unspecified, // unknown_or_inconsistent_pair_of_mbs_f1ap_id
    (uint8_t)ngap_cause_radio_network_t::unspecified, // unknown_or_inconsistent_mrb_id
    (uint8_t)ngap_cause_radio_network_t::unspecified  // tat_sdt_expiry
};

static constexpr uint8_t f1ap_to_ngap_cause_transport[] = {
    (uint8_t)ngap_cause_transport_t::transport_res_unavailable, // transport_res_unavailable
    (uint8_t)ngap_cause_transport_t::unspecified,               // unspecified
    (uint8_t)ngap_cause_transport_t::unspecified,               // unknown_tnl_address_for_iab
    (uint8_t)ngap_cause_transport_t::unspecified,               // unknown_up_tnl_info_for_iab
};

static constexpr uint8_t f1ap_to_ngap_cause_misc[] = {
    (uint8_t)ngap_cause_misc_t::ctrl_processing_overload,             // ctrl_processing_overload
    (uint8_t)ngap_cause_misc_t::not_enough_user_plane_processing_res, // not_enough_user_plane_processing_res
    (uint8_t)ngap_cause_misc_t::hardware_fail,                        // hardware_fail
    (uint8_t)ngap_cause_misc_t::om_intervention,                      // om_intervention
    (uint8_t)ngap_cause_misc_t::unspecified                           // unspecified
};

ngap_cause_t srsran::f1ap_to_ngap_cause(f1ap_cause_t f1ap_cause)
{
  ngap_cause_t ngap_cause;

  if (const auto* result = std::get_if<f1ap_cause_radio_network_t>(&f1ap_cause)) {
    ngap_cause = ngap_cause_radio_network_t(f1ap_to_ngap_cause_radio_network[uint8_t(*result)]);
    return ngap_cause;
  }
  if (const auto* result = std::get_if<f1ap_cause_transport_t>(&f1ap_cause)) {
    ngap_cause = ngap_cause_transport_t(f1ap_to_ngap_cause_transport[uint8_t(*result)]);
    return ngap_cause;
  }
  if (const auto* result = std::get_if<cause_protocol_t>(&f1ap_cause)) {
    ngap_cause = *result;
    return ngap_cause;
  }
  if (const auto* result = std::get_if<cause_misc_t>(&f1ap_cause)) {
    ngap_cause = ngap_cause_misc_t(f1ap_to_ngap_cause_misc[uint8_t(*result)]);
    return ngap_cause;
  }

  report_fatal_error("Cannot convert cause to F1AP type: {}", f1ap_cause);
  return ngap_cause;
}

static constexpr uint8_t f1ap_to_nrppa_cause_protocol[] = {
    (uint8_t)nrppa_cause_protocol_t::transfer_syntax_error,                   // transfer_syntax_error
    (uint8_t)nrppa_cause_protocol_t::abstract_syntax_error_reject,            // abstract_syntax_error_reject
    (uint8_t)nrppa_cause_protocol_t::abstract_syntax_error_ignore_and_notify, // abstract_syntax_error_ignore_and_notify
    (uint8_t)nrppa_cause_protocol_t::msg_not_compatible_with_receiver_state,  // msg_not_compatible_with_receiver_state
    (uint8_t)nrppa_cause_protocol_t::semantic_error,                          // semantic_error
    (uint8_t)nrppa_cause_protocol_t::
        abstract_syntax_error_falsely_constructed_msg, // abstract_syntax_error_falsely_constructed_msg
    (uint8_t)nrppa_cause_protocol_t::unspecified,      // unspecified
};

nrppa_cause_t srsran::f1ap_to_nrppa_cause(f1ap_cause_t f1ap_cause)
{
  nrppa_cause_t nrppa_cause;

  if (std::holds_alternative<f1ap_cause_radio_network_t>(f1ap_cause)) {
    nrppa_cause = nrppa_cause_radio_network_t::unspecified;
    return nrppa_cause;
  }
  if (std::holds_alternative<f1ap_cause_transport_t>(f1ap_cause)) {
    nrppa_cause = nrppa_cause_misc_t::unspecified;
    return nrppa_cause;
  }
  if (const auto* result = std::get_if<cause_protocol_t>(&f1ap_cause)) {
    nrppa_cause = nrppa_cause_protocol_t(f1ap_to_nrppa_cause_protocol[uint8_t(*result)]);
    return nrppa_cause;
  }
  if (std::holds_alternative<cause_misc_t>(f1ap_cause)) {
    nrppa_cause = nrppa_cause_misc_t::unspecified;
    return nrppa_cause;
  }

  report_fatal_error("Cannot convert cause to F1AP type: {}", f1ap_cause);
  return nrppa_cause;
}
