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

#include "e2sm_kpm_metric_defs.h"
#include "e2sm_kpm_utils.h"
#include "srsran/asn1/asn1_utils.h"
#include "srsran/asn1/e2sm/e2sm_kpm_ies.h"
#include "srsran/e2/e2_du.h"
#include "srsran/e2/e2sm/e2sm.h"
#include "srsran/e2/e2sm/e2sm_kpm.h"
#include "srsran/f1ap/du/f1ap_du.h"
#include <deque>
#include <map>
#include <numeric>

namespace srsran {

class e2sm_kpm_du_meas_provider_impl : public e2sm_kpm_meas_provider, public e2_du_metrics_notifier
{
public:
  // constructor takes logger as argument
  e2sm_kpm_du_meas_provider_impl(srs_du::f1ap_ue_id_translator& f1ap_ue_id_translator);

  e2sm_kpm_du_meas_provider_impl(srs_du::f1ap_ue_id_translator& f1ap_ue_id_translator, int max_rlc_metrics_);

  ~e2sm_kpm_du_meas_provider_impl() = default;

  /// scheduler_ue_metrics_notifier functions.
  void report_metrics(const scheduler_cell_metrics& ue_metrics) override;
  void report_metrics(const rlc_metrics& metrics) override;

  /// e2sm_kpm_meas_provider functions.
  std::vector<std::string> get_supported_metric_names(e2sm_kpm_metric_level_enum level) override;

  bool is_cell_supported(const asn1::e2sm::cgi_c& cell_global_id) override;

  bool is_ue_supported(const asn1::e2sm::ue_id_c& ueid) override;

  bool is_test_cond_supported(const asn1::e2sm::test_cond_type_c& test_cond_type) override;

  bool is_metric_supported(const asn1::e2sm::meas_type_c&   meas_type,
                           const asn1::e2sm::meas_label_s&  label,
                           const e2sm_kpm_metric_level_enum level,
                           const bool&                      cell_scope) override;

  bool get_ues_matching_test_conditions(const asn1::e2sm::matching_cond_list_l& matching_cond_list,
                                        std::vector<asn1::e2sm::ue_id_c>&       ues) override;

  bool get_ues_matching_test_conditions(const asn1::e2sm::matching_ue_cond_per_sub_list_l& matching_ue_cond_list,
                                        std::vector<asn1::e2sm::ue_id_c>&                  ues) override;

  bool get_meas_data(const asn1::e2sm::meas_type_c&               meas_type,
                     const asn1::e2sm::label_info_list_l          label_info_list,
                     const std::vector<asn1::e2sm::ue_id_c>&      ues,
                     const std::optional<asn1::e2sm::cgi_c>       cell_global_id,
                     std::vector<asn1::e2sm::meas_record_item_c>& items) override;

private:
  typedef bool(metric_meas_getter_func_t)(const asn1::e2sm::label_info_list_l          label_info_list,
                                          const std::vector<asn1::e2sm::ue_id_c>&      ues,
                                          const std::optional<asn1::e2sm::cgi_c>       cell_global_id,
                                          std::vector<asn1::e2sm::meas_record_item_c>& items);

  typedef metric_meas_getter_func_t(e2sm_kpm_du_meas_provider_impl::*metric_meas_getter_func_ptr);

  struct e2sm_kpm_supported_metric_t {
    uint32_t                    supported_labels;
    uint32_t                    supported_levels;
    bool                        cell_scope_supported;
    metric_meas_getter_func_ptr func;
  };

  bool check_e2sm_kpm_metrics_definitions(span<const e2sm_kpm_metric_t> metrics_defs);

  // Helper functions.
  float bytes_to_kbits(float value);
  bool  handle_no_meas_data_available(const std::vector<asn1::e2sm::ue_id_c>&        ues,
                                      std::vector<asn1::e2sm::meas_record_item_c>&   items,
                                      asn1::e2sm::meas_record_item_c::types::options value_type);

  // Measurement getter functions.
  metric_meas_getter_func_t get_cqi;
  metric_meas_getter_func_t get_rsrp;
  metric_meas_getter_func_t get_rsrq;
  metric_meas_getter_func_t get_prb_avail_dl;
  metric_meas_getter_func_t get_prb_avail_ul;
  metric_meas_getter_func_t get_prb_used_dl;
  metric_meas_getter_func_t get_prb_used_ul;
  metric_meas_getter_func_t get_prb_use_perc_dl;
  metric_meas_getter_func_t get_prb_use_perc_ul;
  metric_meas_getter_func_t get_delay_ul;
  metric_meas_getter_func_t get_drb_rlc_packet_drop_rate_dl;
  metric_meas_getter_func_t get_drb_rlc_sdu_transmitted_volume_dl;
  metric_meas_getter_func_t get_drb_rlc_sdu_transmitted_volume_ul;
  metric_meas_getter_func_t get_drb_dl_mean_throughput;
  metric_meas_getter_func_t get_drb_ul_mean_throughput;
  metric_meas_getter_func_t get_drb_dl_rlc_sdu_latency;
  metric_meas_getter_func_t get_drb_ul_rlc_sdu_latency;
  metric_meas_getter_func_t get_prach_cell_count;

  srslog::basic_logger&                              logger;
  srs_du::f1ap_ue_id_translator&                     f1ap_ue_id_provider;
  unsigned                                           nof_cell_prbs;
  unsigned                                           nof_dl_slots;
  unsigned                                           nof_ul_slots;
  unsigned                                           nof_ded_cell_preambles;
  std::vector<scheduler_ue_metrics>                  last_ue_metrics;
  std::map<uint16_t, std::deque<rlc_metrics>>        ue_aggr_rlc_metrics;
  size_t                                             max_rlc_metrics = 1;
  std::map<std::string, e2sm_kpm_supported_metric_t> supported_metrics;
};

} // namespace srsran
