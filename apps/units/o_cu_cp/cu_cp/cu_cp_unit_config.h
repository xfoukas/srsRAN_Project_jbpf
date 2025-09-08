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

#include "apps/helpers/metrics/metrics_config.h"
#include "apps/units/o_cu_cp/cu_cp/cu_cp_unit_pcap_config.h"
#include "cu_cp_unit_logger_config.h"
#include "srsran/ran/gnb_id.h"
#include "srsran/ran/nr_band.h"
#include "srsran/ran/pci.h"
#include "srsran/ran/qos/five_qi.h"
#include "srsran/ran/s_nssai.h"
#include "srsran/ran/tac.h"
#include <vector>

namespace srsran {

struct cu_cp_unit_plmn_item {
  struct tai_slice_t {
    uint8_t  sst = 0;
    uint32_t sd  = 0xffffffU;
  };

  std::string plmn_id;
  /// Supported Slices by the RAN node.
  std::vector<tai_slice_t> tai_slice_support_list;
};

struct cu_cp_unit_supported_ta_item {
  tac_t                             tac;
  std::vector<cu_cp_unit_plmn_item> plmn_list;
};

struct cu_cp_unit_amf_config_item {
  std::string ip_addr                = "127.0.1.100";
  uint16_t    port                   = 38412;
  std::string bind_addr              = "127.0.0.1";
  std::string bind_interface         = "auto";
  int         sctp_rto_initial_ms    = 120;
  int         sctp_rto_min_ms        = 120;
  int         sctp_rto_max_ms        = 500;
  int         sctp_init_max_attempts = 3;
  int         sctp_max_init_timeo_ms = 500;
  int         sctp_hb_interval_ms    = 30000;
  int         sctp_assoc_max_retx    = 10;
  bool        sctp_nodelay           = false;

  /// List of all tracking areas supported by the AMF.
  std::vector<cu_cp_unit_supported_ta_item> supported_tas = {{7, {{"00101", {cu_cp_unit_plmn_item::tai_slice_t{1}}}}}};
  bool                                      is_default_supported_tas = true;
};

struct cu_cp_unit_amf_config {
  cu_cp_unit_amf_config_item amf;
  /// Allow CU-CP to run without a core, e.g. for test mode.
  bool no_core = false;
  /// Time to wait after a failed AMF reconnection attempt in ms.
  unsigned amf_reconnection_retry_time = 1000;
};

/// Report configuration, for now only supporting the A3 event.
struct cu_cp_unit_report_config {
  unsigned    report_cfg_id;
  std::string report_type;
  unsigned    report_interval_ms;

  std::optional<std::string> event_triggered_report_type;
  std::optional<std::string> meas_trigger_quantity;
  std::optional<int>         meas_trigger_quantity_threshold_db;
  std::optional<int>         meas_trigger_quantity_threshold_2_db;
  std::optional<int> meas_trigger_quantity_offset_db; ///< [-30..30] Note the actual value is field value * 0.5 dB. E.g.
                                                      ///< putting a value of -6 here results in -3dB offset.
  std::optional<unsigned> hysteresis_db;
  std::optional<unsigned> time_to_trigger_ms;
};

struct cu_cp_unit_neighbor_cell_config_item {
  /// Cell id.
  uint64_t nr_cell_id;
  /// Report config ids.
  std::vector<uint64_t> report_cfg_ids;
};

/// Each item describes the relationship between one cell to all other cells.
struct cu_cp_unit_cell_config_item {
  /// Cell id.
  uint64_t                nr_cell_id;
  std::optional<unsigned> periodic_report_cfg_id;

  // These parameters must only be set for external cells
  /// gNodeB identifier bit length.
  std::optional<unsigned> gnb_id_bit_length;
  /// PCI.
  std::optional<pci_t> pci;
  /// NR band.
  std::optional<nr_band> band;
  /// SSB ARFCN.
  std::optional<unsigned> ssb_arfcn;
  /// SSB subcarrier spacing.
  std::optional<unsigned> ssb_scs;
  /// SSB period.
  std::optional<unsigned> ssb_period;
  /// SSB offset.
  std::optional<unsigned> ssb_offset;
  /// SSB duration.
  std::optional<unsigned> ssb_duration;
  /// Vector of cells that are a neighbor of this cell.
  std::vector<cu_cp_unit_neighbor_cell_config_item> ncells;
  // TODO: Add optional SSB parameters.
};

/// All mobility related configuration parameters.
struct cu_cp_unit_mobility_config {
  /// List of all cells known to the CU-CP.
  std::vector<cu_cp_unit_cell_config_item> cells;
  /// Report config.
  std::vector<cu_cp_unit_report_config> report_configs;
  /// Whether to start HO if neighbor cell measurements arrive.
  bool trigger_handover_from_measurements = false;
};

/// RRC specific configuration parameters.
struct cu_cp_unit_rrc_config {
  bool force_reestablishment_fallback = false;
  /// Timeout for RRC procedures (2 * default SRB maxRetxThreshold * t-PollRetransmit = 2 * 8 * 45ms = 720ms, see
  /// TS 38.331 Sec 9.2.1).
  unsigned rrc_procedure_timeout_ms = 720;
};

/// Security configuration parameters.
struct cu_cp_unit_security_config {
  std::string integrity_protection       = "not_needed";
  std::string confidentiality_protection = "required";
  std::string nea_preference_list        = "nea0,nea2,nea1,nea3";
  std::string nia_preference_list        = "nia2,nia1,nia3";
};

/// F1AP-CU configuration parameters.
struct cu_cp_unit_f1ap_config {
  /// Timeout for the F1AP procedures in milliseconds.
  unsigned procedure_timeout = 1000;
};

/// E1AP-CU-CP configuration parameters.
struct cu_cp_unit_e1ap_config {
  /// Timeout for the E1AP procedures in milliseconds.
  unsigned procedure_timeout = 1000;
};

/// RLC UM TX configuration
struct cu_cp_unit_rlc_tx_um_config {
  /// Number of bits used for sequence number.
  uint16_t sn_field_length;
  /// RLC SDU queue size.
  uint32_t queue_size;
};

/// RLC UM RX configuration
struct cu_cp_unit_rlc_rx_um_config {
  /// Number of bits used for sequence number.
  uint16_t sn_field_length;
  /// Timer used by rx to detect PDU loss (ms).
  int32_t t_reassembly;
};

/// RLC UM configuration
struct cu_cp_unit_rlc_um_config {
  cu_cp_unit_rlc_tx_um_config tx;
  cu_cp_unit_rlc_rx_um_config rx;
};

/// RLC UM TX configuration
struct cu_cp_unit_rlc_tx_am_config {
  /// Number of bits used for sequence number.
  uint16_t sn_field_length;
  /// Poll retx timeout (ms).
  int32_t t_poll_retx;
  /// Max retx threshold.
  uint32_t max_retx_thresh;
  /// Insert poll bit after this many PDUs.
  int32_t poll_pdu;
  /// Insert poll bit after this much data (bytes).
  int32_t poll_byte;
  /// Custom parameter to limit the maximum window size for memory reasons. 0 means no limit.
  uint32_t max_window = 0;
  /// RLC SDU queue size.
  uint32_t queue_size = 4096;
};

/// RLC UM RX configuration
struct cu_cp_unit_rlc_rx_am_config {
  /// Number of bits used for sequence number.
  uint16_t sn_field_length;
  /// Timer used by rx to detect PDU loss (ms).
  int32_t t_reassembly;
  /// Timer used by rx to prohibit tx of status PDU (ms).
  int32_t t_status_prohibit;

  /// Implementation-specific parameters that are not specified by 3GPP

  /// Maximum number of visited SNs in the RX window when building a status report. 0 means no limit.
  uint32_t max_sn_per_status = 0;
};

/// RLC AM configuration
struct cu_cp_unit_rlc_am_config {
  cu_cp_unit_rlc_tx_am_config tx;
  cu_cp_unit_rlc_rx_am_config rx;
};

/// RLC configuration
struct cu_cp_unit_rlc_config {
  std::string              mode = "am";
  cu_cp_unit_rlc_um_config um;
  cu_cp_unit_rlc_am_config am;
};

struct cu_cp_unit_pdcp_rx_config {
  /// Number of bits used for sequence number.
  uint16_t sn_field_length;
  /// Timer used to detect PDUs losses (ms).
  int32_t t_reordering;
  /// Whether out-of-order delivery to upper layers is enabled.
  bool out_of_order_delivery;
};

struct cu_cp_unit_pdcp_tx_config {
  /// Number of bits used for sequence number.
  uint16_t sn_field_length;
  /// Timer used to notify lower layers to discard PDUs (ms).
  int32_t discard_timer;
  /// Whether PDCP status report is required.
  bool status_report_required;
};

struct cu_cp_unit_pdcp_config {
  cu_cp_unit_pdcp_tx_config tx;
  cu_cp_unit_pdcp_rx_config rx;
};

/// QoS configuration.
struct cu_cp_unit_qos_config {
  five_qi_t              five_qi = uint_to_five_qi(9);
  cu_cp_unit_rlc_config  rlc;
  cu_cp_unit_pdcp_config pdcp;
};

/// Configuration to enable/disable metrics per layer.
struct cu_cp_unit_metrics_layer_config {
  bool enable_pdcp           = false;
  bool enable_cu_cp_executor = false;

  /// Returns true if one or more layers are enabled, false otherwise.
  bool are_metrics_enabled() const { return enable_pdcp; }
};

/// Metrics configuration.
struct cu_cp_unit_metrics_config {
  /// CU-CP statistics report period in milliseconds.
  unsigned                        cu_cp_report_period = 1000;
  app_helpers::metrics_config     common_metrics_cfg;
  cu_cp_unit_metrics_layer_config layers_cfg;
};

/// CU-CP application unit configuration.
struct cu_cp_unit_config {
  /// Node name.
  std::string ran_node_name = "srscucp01";
  /// gNB identifier.
  gnb_id_t gnb_id = {411, 22};
  /// Maximum number of DUs.
  uint16_t max_nof_dus = 6;
  /// Maximum number of CU-UPs.
  uint16_t max_nof_cu_ups = 6;
  /// Maximum number of UEs.
  uint64_t max_nof_ues = 8192;
  /// Maximum number of DRBs per UE.
  uint8_t max_nof_drbs_per_ue = 8;
  /// Inactivity timer in seconds.
  int inactivity_timer = 120;
  /// PDU session request timeout in seconds (must be larger than T310).
  unsigned request_pdu_session_timeout = 3;
  /// Loggers configuration.
  cu_cp_unit_logger_config loggers;
  /// PCAPs configuration.
  cu_cp_unit_pcap_config pcap_cfg;
  /// Metrics configuration.
  cu_cp_unit_metrics_config metrics;
  /// AMF configuration.
  cu_cp_unit_amf_config amf_config;
  // List of all AMFs the CU-CP should connect to.
  std::vector<cu_cp_unit_amf_config_item> extra_amfs;
  /// Mobility configuration.
  cu_cp_unit_mobility_config mobility_config;
  /// RRC configuration.
  cu_cp_unit_rrc_config rrc_config;
  /// Security configuration.
  cu_cp_unit_security_config security_config;
  /// F1AP configuration.
  cu_cp_unit_f1ap_config f1ap_config;
  /// E1AP configuration.
  cu_cp_unit_e1ap_config e1ap_config;
  /// QoS configuration.
  std::vector<cu_cp_unit_qos_config> qos_cfg;
  /// Network slice configuration.
  std::vector<s_nssai_t> slice_cfg = {s_nssai_t{slice_service_type{1}}};
};

} // namespace srsran
