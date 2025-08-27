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

#include "pucch_builder_params.h"
#include "srsran/adt/expected.h"

namespace srsran {

struct serving_cell_config;

namespace config_helpers {

/// The following values have to be set according to the \ref pucch_resource_manager capabilities.
/// The following values have to be set according to the \ref pucch_resource_manager capabilities.
/// Maximum number of PUCCH F0/F1 resources per UE for HARQ-ACK reporting.
constexpr unsigned max_ue_f0_f1_res_harq = 8;
/// Maximum number of PUCCH F2/F3/F4 resources per UE for HARQ-ACK reporting.
constexpr unsigned max_ue_f2_f3_f4_res_harq = 8;

/// \brief Validates the user-defined parameters for building the PUCCH resource list.
/// \param[in] nof_res_f0_f1 number of PUCCH F0/F1 resources to be generated.
/// \param[in] nof_res_f2_f3_f4 number of PUCCH F2/F3/F4 resources to be generated.
/// \param[in] f0_f1_params PUCCH F0/F1 resource parameters.
/// \param[in] f2_f3_f4_params PUCCH F2/F3/F4 resource parameters.
/// \param[in] bwp_size_rbs size of the BWP in RBs.
/// \param[in] max_nof_symbols maximum number of symbols.
/// \return In case an invalid parameter is detected, returns a string containing an error message.
error_type<std::string>
pucch_parameters_validator(unsigned                                                               nof_res_f0_f1,
                           unsigned                                                               nof_res_f2_f3_f4,
                           const std::variant<pucch_f1_params, pucch_f0_params>&                  f0_f1_params,
                           const std::variant<pucch_f2_params, pucch_f3_params, pucch_f4_params>& f2_f3_f4_params,
                           unsigned                                                               bwp_size_rbs,
                           bounded_integer<unsigned, 1, 14>                                       max_nof_symbols);

/// \brief Generates the list of cell PUCCH resources (Format 0/1 and 2/3/4) given the number of requested resources.
///
/// PUCCH resources F0/F1 and F2/F3/F4 are allocated on different RBs. The function attempts to spread the resources on
/// both sides of the BWP.
///
/// \param[in] nof_res_f0_f1   number of PUCCH F0/F1 resources to be generated.
/// \param[in] nof_res_f2_f3_f4   number of PUCCH F2/F3/F4 resources to be generated.
/// \param[in] f0_f1_params    PUCCH F0/F1 resource parameters.
/// \param[in] f2_f3_f4_params    PUCCH F2/F3/F4 resource parameters.
/// \param[in] bwp_size_rbs    Size of the BWP in RBs.
/// \param[in] max_nof_symbols Maximum number of symbols.
/// \return The list of PUCCH resources for a cell. The list has the PUCCH Format 0/1 resources in front of the list,
/// and the PUCCH Format 2/3/4 in the back of the list.
/// \remark The function returns an empty list in the following cases: (i) If overall the RBs occupancy is larger than
/// the BWP size. (ii) If F2 intra-slot frequency hopping is enabled with only 1 symbol.
std::vector<pucch_resource>
generate_cell_pucch_res_list(unsigned                                                               nof_res_f0_f1,
                             unsigned                                                               nof_res_f2_f3_f4,
                             const std::variant<pucch_f1_params, pucch_f0_params>&                  f0_f1_params,
                             const std::variant<pucch_f2_params, pucch_f3_params, pucch_f4_params>& f2_f3_f4_params,
                             unsigned                                                               bwp_size_rbs,
                             bounded_integer<unsigned, 1, 14>                                       max_nof_symbols);

/// \brief Generates the list of PUCCH resources for a given UE.
///
/// This function generates the list of PUCCH F0/F1 and F2/F3/F4 resources for a given UE, including the resources for
/// HARQ-ACK reporting, SR and CSI. It also updates the PUCCH resource sets accordingly, as well as the pointers to the
/// PUCCH F0/F1 resource for SR and to the PUCCH F2/F3/F4 resource for CSI. This function overwrites the default \c
/// ServingCellConfig passed as a function input.
///
/// The UE's PUCCH resource list composed of:
/// - \ref nof_ue_pucch_f0_f1_res_harq PUCCH Format 0/1 resources for HARQ-ACK reporting, chosen from
///   \ref nof_harq_pucch_sets possible sets of PUCCH Format 0/1 cell resources.
/// - 1 PUCCH Format 0/1 resource for SR chosen from \ref nof_cell_pucch_f0_f1_res_sr possible sets of PUCCH Format 0/1
///   cell resources.
/// - \ref nof_ue_pucch_f2_f3_f4_res_harq PUCCH Format 2/3/4 resources for HARQ-ACK reporting, chosen from
///   \ref nof_harq_pucch_sets possible sets of PUCCH Format 2/3/4 cell resources.
/// - 1 PUCCH Format 2/3/4 resource for CSI chosen from \ref nof_cell_pucch_f2_f3_f4_res_csi possible sets of PUCCH
/// Format
///   2/3/4 cell resources.
///
/// The returned UE PUCCH resource list \ref pucch_res_list contains the following resources, sorted as follows:
///       [ F0/F1-HARQ_0 ... F0/F1-HARQ_N-1 F0/F1-SR F2/F3-HARQ_0 ... F2/F3/F4-HARQ_M-1 F2/F3/F4-CSI ]
/// where N = nof_ue_pucch_f0_f1_res_harq and M = nof_ue_pucch_f2_f3_f4_res_harq,
/// and with the following indices \ref res_id:
/// - The first \ref nof_ue_pucch_f0_f1_res_harq are the PUCCH F0/F1 resources for HARQ-ACK and have index
///   [ (cell_harq_set_idx % nof_harq_pucch_sets) * nof_ue_pucch_f0_f1_res_harq,
///     (cell_harq_set_idx % nof_harq_pucch_sets) * nof_ue_pucch_f0_f1_res_harq + nof_ue_pucch_f0_f1_res_harq ).
/// - The next resource in the list is the PUCCH F0/F1 resource for SR, which have index:
///      nof_harq_pucch_sets * nof_ue_pucch_f0_f1_res_harq + cell_sr_res_idx % nof_cell_pucch_f0_f1_res_sr.
/// - The next \ref nof_ue_pucch_f2_f3_f4_res_harq are the PUCCH F2/F3/F4 resources for HARQ-ACK and have index
///   [  nof_harq_pucch_sets * nof_ue_pucch_f0_f1_res_harq + nof_cell_pucch_f0_f1_res_sr +
///                     (cell_harq_set_idx % nof_harq_pucch_sets) * nof_ue_pucch_f2_f3_f4_res_harq,
///      nof_harq_pucch_sets * nof_ue_pucch_f0_f1_res_harq + nof_cell_pucch_f0_f1_res_sr +
///                     (cell_harq_set_idx % nof_harq_pucch_sets) * nof_ue_pucch_f2_f3_f4_res_harq +
///                     nof_ue_pucch_f2_f3_f4_res_harq).
/// - The last resource in the list is the PUCCH F2/F3/F4 resource for CSI, which has index:
////     nof_harq_pucch_sets * nof_ue_pucch_f0_f1_res_harq + nof_cell_pucch_f0_f1_res_sr +
///                     nof_ue_pucch_f2_f3_f4_res_harq * nof_harq_pucch_sets + cell_csi_res_idx %
///                     nof_cell_pucch_f2_f3_f4_res_csi.
///
/// \param[in,out] serv_cell_cfg default \c ServingCellConfig that will be overwritten by this function.
/// \param[in] res_list cell PUCCH resource list from which the function picks the UE PUCCH resources.
/// \param[in] cell_harq_set_idx defines which PUCCH resource set for HARQ to be assigned to this UE among
///            \ref nof_harq_pucch_sets possible ones; the chosen set for this UE has index
///            cell_harq_set_idx % nof_harq_pucch_sets.
/// \param[in] cell_sr_res_idx defines which PUCCH resource for SR to be assigned to this UE among
///            \ref nof_cell_pucch_f0_f1_res_sr possible ones.  Values: {0, ..., nof_cell_pucch_f0_f1_res_sr-1}.
/// \param[in] cell_csi_res_idx defines which PUCCH resource for CSI to be assigned to this UE among
///            \ref nof_cell_pucch_f2_f3_f4_res_csi possible ones.  Values: {0, ..., nof_cell_pucch_f2_f3_f4_res_csi-1}.
/// \param[in] nof_ue_pucch_f0_f1_res_harq desired number of UE PUCCH F0/F1 resources (HARQ-ACK) in UE configuration.
/// \param[in] nof_ue_pucch_f2_f3_f4_res_harq desired number of UE PUCCH F2/F3/F4 resources (HARQ-ACK) in UE
/// configuration. \param[in] nof_harq_pucch_sets number of possible HARQ sets available in the cell. \param[in]
/// nof_cell_pucch_f0_f1_res_sr number of PUCCH F0/F1 resources for SR available in the cell. \param[in]
/// nof_cell_pucch_f2_f3_f4_res_csi number of PUCCH F2/F3/F4 resources for CSI available in the cell. \return true if
/// the building is successful, false otherwise.
bool ue_pucch_config_builder(serving_cell_config&                                   serv_cell_cfg,
                             const std::vector<pucch_resource>&                     res_list,
                             unsigned                                               cell_harq_set_idx,
                             unsigned                                               cell_sr_res_idx,
                             unsigned                                               cell_csi_res_idx,
                             bounded_integer<unsigned, 1, max_ue_f0_f1_res_harq>    nof_ue_pucch_f0_f1_res_harq,
                             bounded_integer<unsigned, 1, max_ue_f2_f3_f4_res_harq> nof_ue_pucch_f2_f3_f4_res_harq,
                             unsigned                                               nof_harq_pucch_sets,
                             unsigned                                               nof_cell_pucch_f0_f1_res_sr,
                             unsigned nof_cell_pucch_f2_f3_f4_res_csi = 1);

} // namespace config_helpers
} // namespace srsran
