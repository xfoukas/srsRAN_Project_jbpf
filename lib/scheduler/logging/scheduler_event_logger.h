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

#include "srsran/ran/pusch/pusch_tpmi_select.h"
#include "srsran/scheduler/mac_scheduler.h"
#include "srsran/srslog/srslog.h"

namespace srsran {

class scheduler_event_logger
{
public:
  struct prach_event {
    slot_point      slot_rx;
    du_cell_index_t cell_index;
    unsigned        preamble_id;
    rnti_t          ra_rnti;
    rnti_t          tc_rnti;
    unsigned        ta;
  };
  struct ue_creation_event {
    du_ue_index_t   ue_index;
    rnti_t          rnti;
    du_cell_index_t pcell_index;
  };
  struct ue_reconf_event {
    du_ue_index_t ue_index;
    rnti_t        rnti;
  };
  struct ue_cfg_applied_event {
    du_ue_index_t ue_index;
    rnti_t        rnti;
  };
  struct crc_event {
    du_ue_index_t        ue_index;
    rnti_t               rnti;
    du_cell_index_t      cell_index;
    slot_point           sl_rx;
    harq_id_t            h_id;
    bool                 crc;
    std::optional<float> ul_sinr_db;
  };
  struct harq_ack_event {
    du_ue_index_t              ue_index;
    rnti_t                     rnti;
    du_cell_index_t            cell_index;
    slot_point                 sl_ack_rx;
    harq_id_t                  h_id;
    mac_harq_ack_report_status ack;
    units::bytes               tbs;
  };
  struct sr_event {
    du_ue_index_t ue_index;
    rnti_t        rnti;
  };
  struct csi_report_event {
    du_ue_index_t   ue_index;
    rnti_t          rnti;
    csi_report_data csi;
  };
  struct bsr_event {
    du_ue_index_t          ue_index;
    rnti_t                 rnti;
    bsr_format             type;
    ul_bsr_lcg_report_list reported_lcgs;
    units::bytes           tot_ul_pending_bytes;
  };
  struct phr_event {
    du_ue_index_t                   ue_index;
    rnti_t                          rnti;
    du_cell_index_t                 cell_index;
    ph_db_range                     ph;
    std::optional<p_cmax_dbm_range> p_cmax;
  };
  struct error_indication_event {
    slot_point                            sl_tx;
    scheduler_slot_handler::error_outcome outcome;
  };
  struct srs_indication_event {
    du_ue_index_t                         ue_index;
    rnti_t                                rnti;
    std::optional<pusch_tpmi_select_info> tpmi_info;
  };

  scheduler_event_logger(du_cell_index_t cell_index_, pci_t pci_);

  void log()
  {
    if (mode == none or fmtbuf.size() == 0) {
      return;
    }
    log_impl();
  }

  template <typename Event>
  void enqueue(Event&& ev)
  {
    if (mode == none) {
      return;
    }
    enqueue_impl(std::forward<Event>(ev));
  }

  bool enabled() const { return mode != none; }

private:
  enum mode_t { none, info, debug };

  const char* separator() const;

  void log_impl();

  void enqueue_impl(const prach_event& rach_ev);
  void enqueue_impl(const rach_indication_message& rach_ind);

  void enqueue_impl(const ue_creation_event& ue_request);
  void enqueue_impl(const ue_reconf_event& ue_request);
  void enqueue_impl(const sched_ue_delete_message& ue_request);
  void enqueue_impl(const ue_cfg_applied_event& ue_cfg_applied);

  void enqueue_impl(const error_indication_event& err_ind);

  void enqueue_impl(const sr_event& sr);
  void enqueue_impl(const bsr_event& bsr);
  void enqueue_impl(const harq_ack_event& harq_ev);
  void enqueue_impl(const csi_report_event& csi);
  void enqueue_impl(const crc_event& crc_ev);
  void enqueue_impl(const dl_mac_ce_indication& mac_ce);
  void enqueue_impl(const dl_buffer_state_indication_message& bs);
  void enqueue_impl(const phr_event& phr_ev);
  void enqueue_impl(const srs_indication_event& srs_ev);

  const du_cell_index_t cell_index;
  const pci_t           pci;
  srslog::basic_logger& logger;
  mode_t                mode = none;

  fmt::memory_buffer fmtbuf;
};

} // namespace srsran
