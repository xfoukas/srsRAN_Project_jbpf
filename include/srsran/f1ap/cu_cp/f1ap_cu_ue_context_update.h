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
#include "srsran/f1ap/f1ap_ue_id_types.h"
#include "srsran/f1ap/ue_context_management_configs.h"
#include "srsran/ran/cause/f1ap_cause.h"
#include "srsran/ran/cu_types.h"
#include "srsran/ran/nr_cgi.h"
#include "srsran/ran/rb_id.h"
#include "srsran/ran/rnti.h"
#include "srsran/ran/s_nssai.h"
#include <vector>

namespace srsran {
namespace srs_cu_cp {

enum class f1ap_cell_ul_cfg { none = 0, ul, sul, ul_and_sul };

struct f1ap_cu_to_du_rrc_info_ext_ies_container {
  std::optional<byte_buffer> ho_prep_info;
  std::optional<byte_buffer> cell_group_cfg;
  std::optional<byte_buffer> meas_timing_cfg;
  std::optional<byte_buffer> ue_assist_info;
  std::optional<byte_buffer> cg_cfg;
  std::optional<byte_buffer> ue_assist_info_eutra;
  std::optional<byte_buffer> location_meas_info;
  std::optional<byte_buffer> mu_si_m_gap_cfg;
  std::optional<byte_buffer> sdt_mac_phy_cg_cfg;
  std::optional<byte_buffer> mb_si_nterest_ind;
  std::optional<byte_buffer> need_for_gaps_info_nr;
  std::optional<byte_buffer> need_for_gap_ncsg_info_nr;
  std::optional<byte_buffer> need_for_gap_ncsg_info_eutra;
  std::optional<byte_buffer> cfg_restrict_info_daps;
};

struct f1ap_cu_to_du_rrc_info {
  byte_buffer                                             cg_cfg_info;
  byte_buffer                                             ue_cap_rat_container_list;
  byte_buffer                                             meas_cfg;
  std::optional<f1ap_cu_to_du_rrc_info_ext_ies_container> ie_exts;
};

struct f1ap_candidate_sp_cell_item {
  nr_cell_global_id_t candidate_sp_cell_id;
};

struct f1ap_drx_cycle {
  uint16_t                long_drx_cycle_len;
  std::optional<uint16_t> short_drx_cycle_len;
  std::optional<uint8_t>  short_drx_cycle_timer;
};

struct f1ap_scell_to_be_setup_mod_item {
  nr_cell_global_id_t             scell_id;
  uint8_t                         scell_idx;
  std::optional<f1ap_cell_ul_cfg> scell_ul_cfg;
};

struct f1ap_rat_freq_prio_info {
  // choice
  std::optional<uint16_t> endc;
  std::optional<uint16_t> ngran;
};

struct f1ap_res_coordination_transfer_info {
  nr_cell_identity m_enb_cell_id;
};

/// \brief Request from CU to F1AP-CU to start an F1AP "UE Context Setup" procedure, as per TS38.473 8.3.1.
struct f1ap_ue_context_setup_request {
  ue_index_t                                         ue_index = ue_index_t::invalid;
  nr_cell_global_id_t                                sp_cell_id;
  uint8_t                                            serv_cell_idx;
  std::optional<f1ap_cell_ul_cfg>                    sp_cell_ul_cfg;
  f1ap_cu_to_du_rrc_info                             cu_to_du_rrc_info;
  std::vector<f1ap_candidate_sp_cell_item>           candidate_sp_cell_list; // max size = 64
  std::optional<f1ap_drx_cycle>                      drx_cycle;
  byte_buffer                                        res_coordination_transfer_container;
  std::vector<f1ap_scell_to_be_setup_mod_item>       scell_to_be_setup_list; // max size = 32
  std::vector<f1ap_srb_to_setup>                     srbs_to_be_setup_list;  // max size = 8
  std::vector<f1ap_drb_to_setup>                     drbs_to_be_setup_list;  // max size = 64
  std::optional<bool>                                inactivity_monitoring_request;
  std::optional<f1ap_rat_freq_prio_info>             rat_freq_prio_info;
  byte_buffer                                        rrc_container;
  std::optional<uint64_t>                            masked_imeisv;
  std::optional<std::string>                         serving_plmn;
  std::optional<uint64_t>                            gnb_du_ue_ambr_ul;
  std::optional<bool>                                rrc_delivery_status_request;
  std::optional<f1ap_res_coordination_transfer_info> res_coordination_transfer_info;
  std::optional<uint8_t>                             serving_cell_mo;
  std::optional<gnb_cu_ue_f1ap_id_t>                 new_gnb_cu_ue_f1ap_id;
  std::optional<ran_ue_id_t>                         ran_ue_id;
};

struct f1ap_du_to_cu_rrc_info {
  byte_buffer cell_group_cfg;
  byte_buffer meas_gap_cfg;
  byte_buffer requested_p_max_fr1;
};

struct f1ap_scell_failed_to_setup_mod_item {
  nr_cell_global_id_t         scell_id;
  std::optional<f1ap_cause_t> cause;
};

struct f1ap_srbs_setup_mod_item {
  srb_id_t srb_id = srb_id_t::nulltype;
  lcid_t   lcid   = lcid_t::INVALID_LCID;
};

struct f1ap_potential_sp_cell_item {
  nr_cell_global_id_t potential_sp_cell_id;
};

/// \brief Response from F1AP-CU to CU once "UE Context Setup" procedure is complete.
struct f1ap_ue_context_setup_response {
  bool       success  = false;
  ue_index_t ue_index = ue_index_t::invalid;

  // UE Context Setup Response
  f1ap_du_to_cu_rrc_info                           du_to_cu_rrc_info;
  std::optional<rnti_t>                            c_rnti;
  std::optional<byte_buffer>                       res_coordination_transfer_container;
  std::optional<bool>                              full_cfg;
  std::vector<f1ap_drb_setupmod>                   drbs_setup_list;
  std::vector<f1ap_srb_failed_to_setup>            srbs_failed_to_be_setup_list;
  std::vector<f1ap_drb_failed_to_setupmod>         drbs_failed_to_be_setup_list;
  std::vector<f1ap_scell_failed_to_setup_mod_item> scell_failed_to_setup_list;
  std::optional<bool>                              inactivity_monitoring_resp;
  std::vector<f1ap_srbs_setup_mod_item>            srbs_setup_list;

  // UE Context Setup Failure
  std::optional<f1ap_cause_t>              cause;
  std::vector<f1ap_potential_sp_cell_item> potential_sp_cell_list; // max size = 64

  // Common
  std::optional<crit_diagnostics_t> crit_diagnostics;
};

enum class f1ap_tx_action_ind { stop = 0, restart };

enum class f1ap_rrc_recfg_complete_ind { true_value = 0, fail };

struct f1ap_scell_to_be_remd_item {
  nr_cell_global_id_t scell_id;
};

struct f1ap_rlc_fail_ind {
  lcid_t assocated_lcid = lcid_t::INVALID_LCID;
};

/// \brief Request from CU to F1AP-CU to start an F1AP "UE Context Modification" procedure, as per TS38.473 8.3.4.
struct f1ap_ue_context_modification_request {
  ue_index_t                                         ue_index = ue_index_t::invalid;
  std::optional<nr_cell_global_id_t>                 sp_cell_id;
  std::optional<uint8_t>                             serv_cell_idx;
  std::optional<f1ap_cell_ul_cfg>                    sp_cell_ul_cfg;
  std::optional<f1ap_drx_cycle>                      drx_cycle;
  std::optional<f1ap_cu_to_du_rrc_info>              cu_to_du_rrc_info;
  std::optional<f1ap_tx_action_ind>                  tx_action_ind;
  byte_buffer                                        res_coordination_transfer_container;
  std::optional<f1ap_rrc_recfg_complete_ind>         rrc_recfg_complete_ind;
  byte_buffer                                        rrc_container;
  std::vector<f1ap_scell_to_be_setup_mod_item>       scell_to_be_setup_mod_list;
  std::vector<f1ap_scell_to_be_remd_item>            scell_to_be_remd_list;
  std::vector<f1ap_srb_to_setup>                     srbs_to_be_setup_mod_list;
  std::vector<f1ap_drb_to_setup>                     drbs_to_be_setup_mod_list;
  std::vector<f1ap_drb_to_modify>                    drbs_to_be_modified_list;
  std::vector<srb_id_t>                              srbs_to_be_released_list;
  std::vector<drb_id_t>                              drbs_to_be_released_list;
  std::optional<bool>                                inactivity_monitoring_request;
  std::optional<f1ap_rat_freq_prio_info>             rat_freq_prio_info;
  std::optional<bool>                                drx_cfg_ind;
  std::optional<f1ap_rlc_fail_ind>                   rlc_fail_ind;
  byte_buffer                                        ul_tx_direct_current_list_info;
  std::optional<bool>                                gnb_du_cfg_query;
  std::optional<uint64_t>                            gnb_du_ue_ambr_ul;
  std::optional<bool>                                execute_dupl;
  std::optional<bool>                                rrc_delivery_status_request;
  std::optional<f1ap_res_coordination_transfer_info> res_coordination_transfer_info;
  std::optional<uint8_t>                             serving_cell_mo;
  std::optional<bool>                                need_for_gap;
  std::optional<bool>                                full_cfg;
};

struct f1ap_associated_scell_item {
  nr_cell_identity scell_id;
};

/// \brief Response from F1AP-CU to CU once "UE Context Modification" procedure is complete.
struct f1ap_ue_context_modification_response {
  bool success = false;
  // ue context modification response
  byte_buffer                                      res_coordination_transfer_container;
  f1ap_du_to_cu_rrc_info                           du_to_cu_rrc_info;
  std::vector<f1ap_drb_setupmod>                   drbs_setup_list;
  std::vector<f1ap_drb_setupmod>                   drbs_modified_list;
  std::vector<f1ap_srb_failed_to_setup>            srbs_failed_to_be_setup_list;
  std::vector<f1ap_drb_failed_to_setupmod>         drbs_failed_to_be_setup_list;
  std::vector<f1ap_scell_failed_to_setup_mod_item> scell_failed_to_be_setup_list;
  std::vector<f1ap_drb_failed_to_setupmod>         drbs_failed_to_be_modified_list;
  std::optional<bool>                              inactivity_monitoring_resp;
  std::optional<srsran::rnti_t>                    c_rnti;
  std::vector<f1ap_associated_scell_item>          associated_scell_list;
  std::vector<f1ap_srbs_setup_mod_item>            srbs_setup_mod_list;
  std::vector<f1ap_srbs_setup_mod_item>            srbs_modified_list;
  std::optional<bool>                              full_cfg;

  // UE Context Modification Failure
  std::optional<f1ap_cause_t> cause;

  // Common
  std::optional<crit_diagnostics_t> crit_diagnostics;
};

/// \brief Request Command for F1AP UE CONTEXT Release Request.
struct f1ap_ue_context_release_request {
  ue_index_t   ue_index;
  f1ap_cause_t cause;
};

} // namespace srs_cu_cp
} // namespace srsran
