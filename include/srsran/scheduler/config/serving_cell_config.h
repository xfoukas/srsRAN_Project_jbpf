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

#include "srsran/ran/csi_rs/csi_meas_config.h"
#include "srsran/ran/pdcch/downlink_preemption.h"
#include "srsran/ran/pdsch/pdsch_mcs.h"
#include "srsran/ran/pdsch/pdsch_prb_bundling.h"
#include "srsran/ran/pdsch/pdsch_rate_match_pattern.h"
#include "srsran/ran/pucch/pucch_tpc.h"
#include "srsran/ran/pucch/srs_tpc.h"
#include "srsran/ran/pusch/pusch_configuration.h"
#include "srsran/ran/pusch/pusch_tpc.h"
#include "srsran/ran/resource_allocation/vrb_to_prb.h"
#include "srsran/ran/serv_cell_index.h"
#include "srsran/ran/srs/srs_configuration.h"
#include "srsran/ran/tci.h"
#include "srsran/ran/time_alignment_config.h"
#include "srsran/scheduler/config/bwp_configuration.h"
#include "srsran/scheduler/result/resource_block_group.h"

namespace srsran {

/// \c xOverhead options, as per TS 38.331. Used in \c PDSCH-ServingCellConfig and \c PUSCH-ServingCellConfig.
enum class x_overhead {
  not_set = 0,
  xoh6    = 6,
  xoh12   = 12,
  xoh18   = 18,
};

/// Converts a uint into a \ref serv_cell_index.
inline serv_cell_index_t uint_to_serv_cell_index(unsigned idx)
{
  return idx < MAX_NOF_SERVING_CELLS ? static_cast<serv_cell_index_t>(idx)
                                     : static_cast<serv_cell_index_t>(MAX_NOF_SERVING_CELLS);
}

/// "PDCCH-Config" - UE-dedicated PDCCH configuration as per TS 38.331.
struct pdcch_config {
  /// List of CORESETs to be used by the UE. In case of CORESET Id overlaps with commonControlResourceSet,
  /// the CORESET in this list takes precedence. The network configures at most 3 CORESETs per BWP per cell (including
  /// UE-specific and common CORESETs).
  std::vector<coreset_configuration> coresets;
  /// List of SearchSpaces to be used by the UE. The network configures at most 10 Search Spaces per BWP per cell
  /// (including UE-specific and common Search Spaces).
  std::vector<search_space_configuration> search_spaces;
  /// Configuration of downlink preemption indications to be monitored in this cell.
  std::optional<downlink_preemption> dl_preemption;
  /// TPC Commands Configuration to configure UE for extracting TPC commands for PUSCH from a group-TPC messages on DCI.
  std::optional<pusch_tpc_command_config> pusch_tpc_cmd_cfg;
  /// TPC Commands Configuration to configure UE for extracting TPC commands for PUCCH from a group-TPC messages on DCI.
  std::optional<pucch_tpc_command_config> pucch_tpc_cmd_cfg;
  /// TPC Commands Configuration to configure UE for extracting TPC commands for SRS from a group-TPC messages on DCI.
  std::optional<srs_tpc_command_config> srs_tpc_cmd_cfg;
  // TODO: add remaining fields.

  bool operator==(const pdcch_config& rhs) const
  {
    return coresets == rhs.coresets && search_spaces == rhs.search_spaces && dl_preemption == rhs.dl_preemption &&
           pusch_tpc_cmd_cfg == rhs.pusch_tpc_cmd_cfg && pucch_tpc_cmd_cfg == rhs.pucch_tpc_cmd_cfg &&
           srs_tpc_cmd_cfg == rhs.srs_tpc_cmd_cfg;
  }
  bool operator!=(const pdcch_config& rhs) const { return !(rhs == *this); }
};

/// "PDSCH-Config" - UE-dedicated PDSCH Configuration as per TS38.331.
struct pdsch_config {
  /// \brief Resource allocation type of to DCI format 1_1.
  /// \remark See TS 38.214, clause 5.1.2.2.
  enum class resource_allocation { resource_allocation_type_0, resource_allocation_type_1, dynamic_switch };

  /// \brief Aggregation factor.
  /// \remark See TS 38.214, clause 5.1.2.1.
  enum class pdsch_aggregation_factor { n2, n4, n8 };

  enum class max_no_codeword_scheduled_by_dci { n1, n2 };

  /// Identifier used to initialize data scrambling (c_init) for PDSCH. If the field is absent, the UE applies the PCI.
  /// See TS38.331, \e dataScramblingIdentityPDSCH, and TS38.211, 7.3.1.1. Values: {0,...,1023}.
  std::optional<uint16_t> data_scrambling_id_pdsch;
  /// DMRS configuration for PDSCH transmissions using PDSCH mapping type A (chosen dynamically via
  /// PDSCH-TimeDomainResourceAllocation).
  std::optional<dmrs_downlink_config> pdsch_mapping_type_a_dmrs;
  /// DMRS configuration for PDSCH transmissions using PDSCH mapping type B (chosen dynamically via
  /// PDSCH-TimeDomainResourceAllocation).
  std::optional<dmrs_downlink_config> pdsch_mapping_type_b_dmrs;
  /// VRB-to-PRB mapping type for PDSCH. The field vrb-ToPRB-Interleaver applies to DCI format 1_1.
  vrb_to_prb::mapping_type vrb_to_prb_interleaving;
  /// A list of Transmission Configuration Indicator (TCI) states indicating a transmission configuration which includes
  /// QCL-relationships between the DL RSs in one RS set and the PDSCH DMRS ports (see TS 38.214, clause 5.1.5).
  static_vector<tci_state, MAX_NOF_TCI_STATES> tci_states;
  /// Configuration of resource allocation type 0 and resource allocation type 1 for non-fallback DCI.
  resource_allocation res_alloc;
  /// Number of repetitions for data. When the field is absent the UE applies the value 1.
  std::optional<pdsch_aggregation_factor> aggr_factor;
  /// PDSCH time domain resource allocations. Size: (0..maxNrofDL-Allocations=16).
  std::vector<pdsch_time_domain_resource_allocation> pdsch_td_alloc_list;
  /// Resources patterns which the UE should rate match PDSCH around. The UE rate matches around the union of all
  /// resources indicated in the rate match patterns. Rate match patterns defined here on cell level apply only to PDSCH
  /// of the same numerology. See 38.214, clause 5.1.4,1.
  static_vector<rate_match_pattern, MAX_NOF_RATE_MATCH_PATTERNS> rate_match_pattrn;
  // TODO: RateMatchPatternGroup1 and RateMatchPatternGroup2

  /// Selection between config 1 and config 2 for RBG size for PDSCH. The UE ignores this field if resourceAllocation is
  /// set to resourceAllocationType1.
  rbg_size rbg_sz;
  /// Indicates which MCS table the UE shall use for PDSCH. (see TS 38.214 [19], clause 5.1.3.1). The field mcs-Table
  /// applies to DCI format 1_0 and DCI format 1_1.
  pdsch_mcs_table mcs_table{pdsch_mcs_table::qam64};
  /// Maximum number of code words a single DCI may schedule. This changes the number of MCS/RV/NDI bits in the DCI
  /// message from 1 to 2. See TS 38.331, "maxNrofCodeWordsScheduledByDCI".
  bool is_max_cw_sched_by_dci_is_two{false};
  /// Indicates the PRB bundle type and bundle size(s). If dynamic is chosen, the actual bundleSizeSet1 or
  /// bundleSizeSet2 to use is indicated via DCI. Constraints on bundleSize(Set) setting depending on
  /// vrb-ToPRB-Interleaver and rbg-Size settings are described in TS 38.214, clause 5.1.2.3. If a bundleSize(Set)
  /// value is absent, the UE applies the value n2. The field prb-BundlingType applies to DCI format 1_1.
  prb_bundling prb_bndlg;
  /// \brief List of zp-CSI-RS-Resources. Maximum size: 32.
  std::vector<zp_csi_rs_resource> zp_csi_rs_res_list;
  /// A set of periodically occurring ZP-CSI-RS-Resources. The network uses the ZP-CSI-RSResourceSetId=0 for this set.
  std::optional<zp_csi_rs_resource_set> p_zp_csi_rs_res;

  // TODO: Remaining.

  bool operator==(const pdsch_config& rhs) const
  {
    return data_scrambling_id_pdsch == rhs.data_scrambling_id_pdsch &&
           pdsch_mapping_type_a_dmrs == rhs.pdsch_mapping_type_a_dmrs &&
           pdsch_mapping_type_b_dmrs == rhs.pdsch_mapping_type_b_dmrs &&
           vrb_to_prb_interleaving == rhs.vrb_to_prb_interleaving && tci_states == rhs.tci_states &&
           res_alloc == rhs.res_alloc && aggr_factor == rhs.aggr_factor &&
           pdsch_td_alloc_list == rhs.pdsch_td_alloc_list && rate_match_pattrn == rhs.rate_match_pattrn &&
           rbg_sz == rhs.rbg_sz && mcs_table == rhs.mcs_table &&
           is_max_cw_sched_by_dci_is_two == rhs.is_max_cw_sched_by_dci_is_two && prb_bndlg == rhs.prb_bndlg &&
           zp_csi_rs_res_list == rhs.zp_csi_rs_res_list and p_zp_csi_rs_res == rhs.p_zp_csi_rs_res;
  }
  bool operator!=(const pdsch_config& rhs) const { return !(rhs == *this); }
};

/// "BWP-DownlinkDedicated" as per TS 38.331.
struct bwp_downlink_dedicated {
  std::optional<pdcch_config> pdcch_cfg;
  std::optional<pdsch_config> pdsch_cfg;
  // TODO: Remaining

  bool operator==(const bwp_downlink_dedicated& other) const
  {
    return pdcch_cfg == other.pdcch_cfg and pdsch_cfg == other.pdsch_cfg;
  }
};

/// "BWP-Downlink" as per TS 38.331.
struct bwp_downlink {
  /// Identifier of BWP. Value 0 is reserved for the initial BWP.
  bwp_id_t bwp_id;
  /// Common parameters of the BWP.
  bwp_downlink_common bwp_dl_common;
  /// UE-dedicated parameters of the BWP.
  bwp_downlink_dedicated bwp_dl_ded;

  bool operator==(const bwp_downlink& other) const
  {
    return bwp_id == other.bwp_id and bwp_dl_common == other.bwp_dl_common and bwp_dl_ded == other.bwp_dl_ded;
  }
};

/// Used to configure the dedicated UE-specific parameters of an UL BWP.
/// \remark See TS 38.331, BWP-UplinkDedicated.
struct bwp_uplink_dedicated {
  std::optional<pucch_config> pucch_cfg;
  std::optional<pusch_config> pusch_cfg;
  std::optional<srs_config>   srs_cfg;

  bool operator==(const bwp_uplink_dedicated& other) const
  {
    return pucch_cfg == other.pucch_cfg and pusch_cfg == other.pusch_cfg and srs_cfg == other.srs_cfg;
  }
};

/// \brief Used to configure UE specific PUSCH parameters that are common across the UE's BWPs of one serving cell.
/// \remark See TS 38.331, PUSCH-ServingCellConfig.
struct pusch_serving_cell_config {
  struct pusch_code_block_group_transmission {
    /// Maximum number of code-block-groups (CBGs) per TB. See TS 38.213, clause 9.1.
    enum class max_code_block_groups_per_transport_block { n2 = 2, n4 = 4, n6 = 6, n8 = 8 };

    max_code_block_groups_per_transport_block max_cgb_per_tb;

    bool operator==(const pusch_code_block_group_transmission& other) const
    {
      return max_cgb_per_tb == other.max_cgb_per_tb;
    }
  };

  std::optional<pusch_code_block_group_transmission> cbg_tx;
  x_overhead                                         x_ov_head{x_overhead::not_set};

  bool operator==(const pusch_serving_cell_config& other) const
  {
    return cbg_tx == other.cbg_tx and x_ov_head == other.x_ov_head;
  }
};

/// Uplink configuration, as per \c UplinkConfig, in \c ServingCellConfig, TS 38.331.
struct uplink_config {
  bwp_uplink_dedicated                     init_ul_bwp;
  std::optional<pusch_serving_cell_config> pusch_serv_cell_cfg;
  // TODO: add remaining fields.

  bool operator==(const uplink_config& other) const
  {
    return init_ul_bwp == other.init_ul_bwp and pusch_serv_cell_cfg == other.pusch_serv_cell_cfg;
  }
};

/// \c PDSCH-CodeBlockGroupTransmission, as per TS38.331.
struct pdsch_code_block_group_transmission {
  /// \c maxCodeBlockGroupsPerTransportBlock.
  /// \remark Maximum number of code-block-groups (CBGs) per TB. In case of multiple CW, the maximum CBG is 4.
  enum class max_code_block_groups_per_tb { n2 = 2, n4 = 4, n6 = 6, n8 = 8 };

  max_code_block_groups_per_tb max_cbg_per_tb;
  bool                         code_block_group_flush_indicator;

  bool operator==(const pdsch_code_block_group_transmission& rhs) const
  {
    return max_cbg_per_tb == rhs.max_cbg_per_tb &&
           code_block_group_flush_indicator == rhs.code_block_group_flush_indicator;
  }
  bool operator!=(const pdsch_code_block_group_transmission& rhs) const { return !(rhs == *this); }
};

/// \c PDSCH-ServingCellConfig, as per TS38.331.
struct pdsch_serving_cell_config {
  /// \c nrofHARQ-ProcessesForPDSCH.
  enum class nof_harq_proc_for_pdsch { n2 = 2, n4 = 4, n6 = 6, n8 = 8, n10 = 10, n12 = 12, n16 = 16 };

  std::optional<pdsch_code_block_group_transmission> code_block_group_tx;
  x_overhead                                         x_ov_head{x_overhead::not_set};
  /// See TS 38.331, \c nrofHARQ-ProcessesForPDSCH.
  nof_harq_proc_for_pdsch          nof_harq_proc{nof_harq_proc_for_pdsch::n16};
  std::optional<serv_cell_index_t> pucch_cell;
  /// Maximum number of MIMO layers to be used for PDSCH in all BWPs of this serving cell. Values {1,...,8};
  unsigned            max_mimo_layers;
  std::optional<bool> processing_type_2_enabled;

  bool operator==(const pdsch_serving_cell_config& rhs) const
  {
    return code_block_group_tx == rhs.code_block_group_tx && x_ov_head == rhs.x_ov_head &&
           nof_harq_proc == rhs.nof_harq_proc && pucch_cell == rhs.pucch_cell &&
           max_mimo_layers == rhs.max_mimo_layers && processing_type_2_enabled == rhs.processing_type_2_enabled;
  }
  bool operator!=(const pdsch_serving_cell_config& rhs) const { return !(rhs == *this); }
};

/// \c ServingCellConfig, as per TS38.331.
struct serving_cell_config {
  /// DU-specific cell identifier.
  du_cell_index_t cell_index;
  /// Initial Downlink BWP.
  bwp_downlink_dedicated init_dl_bwp;
  /// List of UE DL BWPs. Size: (0..maxNrofBWPs=4)
  static_vector<bwp_downlink, MAX_NOF_BWPS> dl_bwps;
  /// \c uplinkConfig, containing the UL configuration.
  std::optional<uplink_config> ul_config;
  /// \c pdsch-ServingCellConfig.
  std::optional<pdsch_serving_cell_config> pdsch_serv_cell_cfg;
  /// \c CSI-MeasConfig.
  std::optional<csi_meas_config> csi_meas_cfg;
  /// Timing Advance Group ID to which this cell belongs to.
  time_alignment_group::id_t tag_id{0};

  bool operator==(const serving_cell_config& other) const
  {
    return cell_index == other.cell_index and init_dl_bwp == other.init_dl_bwp and dl_bwps == other.dl_bwps and
           ul_config == other.ul_config and pdsch_serv_cell_cfg == other.pdsch_serv_cell_cfg and
           csi_meas_cfg == other.csi_meas_cfg and tag_id == other.tag_id;
  }
};

/// UE-dedicated configuration for serving cell.
struct cell_config_dedicated {
  serv_cell_index_t serv_cell_idx;
  /// Serving Cell Configuration as per TS 38.331.
  serving_cell_config serv_cell_cfg;
};

} // namespace srsran
