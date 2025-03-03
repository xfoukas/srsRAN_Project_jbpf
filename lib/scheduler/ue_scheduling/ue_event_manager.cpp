/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
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

#include "ue_event_manager.h"
#include "../logging/scheduler_event_logger.h"
#include "../logging/scheduler_metrics_handler.h"
#include "../srs/srs_scheduler.h"
#include "../uci_scheduling/uci_scheduler_impl.h"

#ifdef JBPF_ENABLED
#include "jbpf_srsran_hooks.h"
DEFINE_JBPF_HOOK(mac_sched_ue_creation);
DEFINE_JBPF_HOOK(mac_sched_ue_reconfig);
DEFINE_JBPF_HOOK(mac_sched_ue_deletion);
DEFINE_JBPF_HOOK(mac_sched_ue_config_applied);
DEFINE_JBPF_HOOK(mac_sched_ul_bsr_indication);
DEFINE_JBPF_HOOK(mac_sched_crc_indication);
DEFINE_JBPF_HOOK(mac_sched_uci_indication);
DEFINE_JBPF_HOOK(mac_sched_dl_mac_ce_indication);
DEFINE_JBPF_HOOK(mac_sched_ul_phr_indication);
DEFINE_JBPF_HOOK(mac_sched_dl_buffer_state_indication);
DEFINE_JBPF_HOOK(mac_sched_srs_indication);
#endif


using namespace srsran;

/// \brief More than one DL buffer occupancy update may be received per slot for the same UE and bearer. This class
/// ensures that the UE DL buffer occupancy is updated only once per bearer per slot for efficiency reasons.
class ue_event_manager::ue_dl_buffer_occupancy_manager final : public scheduler_dl_buffer_state_indication_handler
{
  using bearer_key                        = uint32_t;
  static constexpr size_t NOF_BEARER_KEYS = MAX_NOF_DU_UES * MAX_NOF_RB_LCIDS;

  static bearer_key    get_bearer_key(du_ue_index_t ue_index, lcid_t lcid) { return lcid * MAX_NOF_DU_UES + ue_index; }
  static du_ue_index_t get_ue_index(bearer_key key) { return to_du_ue_index(key % MAX_NOF_DU_UES); }
  static lcid_t        get_lcid(bearer_key key) { return uint_to_lcid(key / MAX_NOF_DU_UES); }

public:
  ue_dl_buffer_occupancy_manager(ue_event_manager& parent_) : parent(parent_), pending_evs(NOF_BEARER_KEYS)
  {
    std::fill(ue_dl_bo_table.begin(), ue_dl_bo_table.end(), -1);
  }

  void handle_dl_buffer_state_indication(const dl_buffer_state_indication_message& rlc_dl_bo) override
  {
    // Update DL Buffer Occupancy for the given UE and bearer.
    unsigned key          = get_bearer_key(rlc_dl_bo.ue_index, rlc_dl_bo.lcid);
    bool     first_rlc_bo = ue_dl_bo_table[key].exchange(rlc_dl_bo.bs, std::memory_order_acquire) < 0;

    if (not first_rlc_bo) {
      // If another DL BO update has been received before for this same bearer, we do not need to enqueue a new event.
      return;
    }

    // Signal that this bearer needs its BO state updated.
    if (not pending_evs.try_push(key)) {
      parent.logger.warning("ue={} lcid={}: Discarding DL buffer occupancy update. Cause: Event queue is full",
                            rlc_dl_bo.ue_index,
                            rlc_dl_bo.lcid);
    }
  }

  void slot_indication(slot_point sl)
  {
    // Process RLC buffer updates of pending UEs.
    bearer_key key;
    while (pending_evs.try_pop(key)) {
      // Recreate latest DL BO update.
      dl_buffer_state_indication_message dl_bo;
      // > Extract UE index and LCID.
      dl_bo.ue_index = get_ue_index(key);
      dl_bo.lcid     = get_lcid(key);
      // > Extract last DL BO value for the respective bearer and reset BO table position.
      dl_bo.bs = ue_dl_bo_table[key].exchange(-1, std::memory_order_release);
      if (dl_bo.bs < 0) {
        parent.logger.warning(
            "ue={} lcid={}: Invalid DL buffer occupancy value: {}", dl_bo.ue_index, dl_bo.lcid, dl_bo.bs);
        continue;
      }

      // Retrieve UE.
      if (not parent.ue_db.contains(dl_bo.ue_index)) {
        parent.log_invalid_ue_index(dl_bo.ue_index);
        continue;
      }
      ue& u = parent.ue_db[dl_bo.ue_index];

      // Forward DL BO update to UE.
      u.handle_dl_buffer_state_indication(dl_bo);
      auto& du_pcell = parent.du_cells[u.get_pcell().cell_index];
      if (dl_bo.lcid == LCID_SRB0 or (u.get_pcell().is_in_fallback_mode() and dl_bo.lcid == LCID_SRB1)) {
        // Signal SRB fallback scheduler with the new SRB0/SRB1 buffer state.
        du_pcell.fallback_sched->handle_dl_buffer_state_indication_srb(
            dl_bo.ue_index, dl_bo.lcid == LCID_SRB0, sl, dl_bo.bs);
      }

      // Log event.
      du_pcell.ev_logger->enqueue(dl_bo);

      // Report event.
      du_pcell.metrics->handle_dl_buffer_state_indication(dl_bo);
    }
  }

private:
  using ue_event_queue =
      concurrent_queue<bearer_key, concurrent_queue_policy::lockfree_mpmc, concurrent_queue_wait_policy::non_blocking>;

  ue_event_manager& parent;

  // Table of pending DL Buffer Occupancy values. -1 means that no DL Buffer Occupancy is set.
  std::array<std::atomic<int>, NOF_BEARER_KEYS> ue_dl_bo_table;

  // Queue of {UE Id, LCID} pairs with pending DL Buffer Occupancy updates.
  ue_event_queue pending_evs;
};

// Initial capacity for the common and cell event lists, in order to avoid std::vector reallocations. We use the max
// nof UEs as a conservative estimate of the expected number of events per slot.
static constexpr size_t COMMON_EVENT_LIST_SIZE = MAX_NOF_DU_UES * 2;
static constexpr size_t CELL_EVENT_LIST_SIZE   = MAX_NOF_DU_UES * 2;

ue_event_manager::ue_event_manager(ue_repository& ue_db_) :
  ue_db(ue_db_),
  logger(srslog::fetch_basic_logger("SCHED")),
  common_events(COMMON_EVENT_LIST_SIZE),
  dl_bo_mng(std::make_unique<ue_dl_buffer_occupancy_manager>(*this))
{
}

ue_event_manager::~ue_event_manager() {}

void ue_event_manager::handle_ue_creation(ue_config_update_event ev)
{
  // Create UE object outside the scheduler slot indication handler to minimize latency.
  std::unique_ptr<ue> u = std::make_unique<ue>(
      ue_creation_command{ev.next_config(),
                          ev.get_fallback_command().has_value() and ev.get_fallback_command().value(),
                          *du_cells[ev.next_config().pcell_common_cfg().cell_index].cell_harqs});
  const du_ue_index_t ue_idx = u->ue_index;

  auto handle_ue_creation_impl = [this, u = std::move(u), ev = std::move(ev)]() mutable {
    if (ue_db.contains(u->ue_index)) {
      logger.error("ue={} rnti={}: Discarding UE creation. Cause: A UE with the same index already exists",
                   u->ue_index,
                   u->crnti);
      return;
    }

#ifdef JBPF_ENABLED
    hook_mac_sched_ue_creation((0), u->get_pcell().get_cell_cfg().pci, (uint16_t)u->crnti);
#endif

    // Insert UE in UE repository.
    du_ue_index_t   ueidx       = u->ue_index;
    rnti_t          rnti        = u->crnti;
    du_cell_index_t pcell_index = u->get_pcell().cell_index;
    ue_db.add_ue(std::move(u));

    const auto& added_ue = ue_db[ueidx];
    for (unsigned i = 0, e = added_ue.nof_cells(); i != e; ++i) {
      // Update UCI scheduler with new UE UCI resources.
      du_cells[pcell_index].uci_sched->add_ue(added_ue.get_cell(to_ue_cell_index(i)).cfg());

      // Update SRS scheduler with new UE SRS resources.
      du_cells[pcell_index].srs_sched->add_ue(added_ue.get_cell(to_ue_cell_index(i)).cfg());

      // Add UE to slice scheduler.
      // Note: This action only has effect when UE is created in non-fallback mode.
      du_cells[pcell_index].slice_sched->add_ue(ueidx);
    }

    // Log Event.
    du_cells[pcell_index].ev_logger->enqueue(scheduler_event_logger::ue_creation_event{ueidx, rnti, pcell_index});

    // Notify config manager that creation is complete with success.
    ev.notify_completion();
  };

  // Defer UE object addition to ue list to the slot indication handler.
  if (not common_events.try_push(common_event_t{INVALID_DU_UE_INDEX, std::move(handle_ue_creation_impl)})) {
    logger.warning("ue={}: Discarding UE creation. Cause: Event queue is full", ue_idx);
  }
}

void ue_event_manager::handle_ue_reconfiguration(ue_config_update_event ev)
{
  const du_ue_index_t ue_index = ev.get_ue_index();

  auto handle_ue_reconf_impl = [this, ev = std::move(ev)]() mutable {
    const du_ue_index_t ue_idx = ev.get_ue_index();
    if (not ue_db.contains(ue_idx)) {
      log_invalid_ue_index(ue_idx, "UE Reconfig Request");
      return;
    }
    auto& u = ue_db[ue_idx];

#ifdef JBPF_ENABLED
    hook_mac_sched_ue_reconfig((0), u.get_pcell().get_cell_cfg().pci, (uint16_t)u.crnti);
#endif

    // If a UE carrier has been removed, remove the UE from the respective slice scheduler.
    // Update UCI scheduler with cell changes.
    for (unsigned i = 0, e = u.nof_cells(); i != e; ++i) {
      auto& ue_cc = u.get_cell(to_ue_cell_index(i));
      if (not ev.next_config().contains(ue_cc.cell_index)) {
        // UE carrier is being removed.
        // Update UE UCI resources in UCI scheduler.
        du_cells[ue_cc.cell_index].uci_sched->rem_ue(ue_cc.cfg());
        // Update UE SRS resources in SRS scheduler.
        du_cells[ue_cc.cell_index].srs_sched->rem_ue(ue_cc.cfg());
        // Schedule removal of UE in slice scheduler.
        du_cells[ue_cc.cell_index].slice_sched->rem_ue(ue_idx);
      } else {
        // UE carrier is being reconfigured.
        du_cells[ue_cc.cell_index].uci_sched->reconf_ue(ev.next_config().ue_cell_cfg(ue_cc.cell_index), ue_cc.cfg());
        du_cells[ue_cc.cell_index].srs_sched->reconf_ue(ev.next_config().ue_cell_cfg(ue_cc.cell_index), ue_cc.cfg());
      }
    }
    for (unsigned i = 0, e = ev.next_config().nof_cells(); i != e; ++i) {
      const auto& new_ue_cc_cfg = ev.next_config().ue_cell_cfg(to_ue_cell_index(i));
      auto*       ue_cc         = u.find_cell(new_ue_cc_cfg.cell_cfg_common.cell_index);
      if (ue_cc == nullptr) {
        // New UE carrier is being added.
        du_cells[new_ue_cc_cfg.cell_cfg_common.cell_index].uci_sched->add_ue(new_ue_cc_cfg);

        du_cells[new_ue_cc_cfg.cell_cfg_common.cell_index].srs_sched->add_ue(new_ue_cc_cfg);
      }
    }

    // Configure existing UE.
    ue_db[ue_idx].handle_reconfiguration_request(ue_reconf_command{ev.next_config()});

    // Update slice scheduler.
    for (unsigned i = 0, e = u.nof_cells(); i != e; ++i) {
      const auto& ue_cc = u.get_cell(to_ue_cell_index(i));
      // Reconfigure UE in slice scheduler.
      du_cells[ue_cc.cell_index].slice_sched->reconf_ue(u.ue_index);
    }

    // Log event.
    du_cells[u.get_pcell().cell_index].ev_logger->enqueue(scheduler_event_logger::ue_reconf_event{ue_idx, u.crnti});

    // Notify config manager that creation is complete with success.
    ev.notify_completion();
  };

  if (not common_events.try_push(common_event_t{ue_index, std::move(handle_ue_reconf_impl)})) {
    logger.warning("ue={}: Discarding UE reconfiguration. Cause: Event queue is full", ue_index);
  }
}

void ue_event_manager::handle_ue_deletion(ue_config_delete_event ev)
{
  const du_ue_index_t ue_index = ev.ue_index();

  auto handle_ue_deletion_impl = [this, ev = std::move(ev)]() mutable {
    const du_ue_index_t ue_idx = ev.ue_index();
    if (not ue_db.contains(ue_idx)) {
      logger.warning("Received request to delete ue={} that does not exist", ue_idx);
      return;
    }
    const auto&     u         = ue_db[ue_idx];
    const rnti_t    rnti      = u.crnti;
    du_cell_index_t pcell_idx = u.get_pcell().cell_index;

#ifdef JBPF_ENABLED
    hook_mac_sched_ue_deletion((0), u.get_pcell().get_cell_cfg().pci, (uint16_t)u.crnti);
#endif

    for (unsigned i = 0, e = u.nof_cells(); i != e; ++i) {
      // Update UCI scheduling by removing existing UE UCI resources.
      du_cells[u.get_cell(to_ue_cell_index(i)).cell_index].uci_sched->rem_ue(u.get_pcell().cfg());
      // Update SRS scheduling by removing existing UE SRS resources.
      du_cells[u.get_cell(to_ue_cell_index(i)).cell_index].srs_sched->rem_ue(u.get_pcell().cfg());
      // Schedule removal of UE from slice scheduler.
      du_cells[u.get_cell(to_ue_cell_index(i)).cell_index].slice_sched->rem_ue(ue_idx);
    }

    // Schedule UE removal from repository.
    ue_db.schedule_ue_rem(std::move(ev));

    // Log UE removal event.
    du_cells[pcell_idx].ev_logger->enqueue(sched_ue_delete_message{ue_idx, rnti});
  };

  if (not common_events.try_push(common_event_t{ue_index, std::move(handle_ue_deletion_impl)})) {
    logger.warning("ue={}: Discarding UE deletion. Cause: Event queue is full", ue_index);
  }
}

void ue_event_manager::handle_ue_config_applied(du_ue_index_t ue_idx)
{
  auto handle_ue_config_applied_impl = [this, ue_idx]() {
    if (not ue_db.contains(ue_idx)) {
      logger.warning("Received config application confirmation for ue={} that does not exist", ue_idx);
      return;
    }
    ue&   u     = ue_db[ue_idx];
    auto& pcell = du_cells[u.get_pcell().cell_index];

#ifdef JBPF_ENABLED
    hook_mac_sched_ue_config_applied(0, u.get_pcell().get_cell_cfg().pci, (uint16_t)u.crnti);
#endif

    // Log UE config applied event.
    pcell.ev_logger->enqueue(scheduler_event_logger::ue_cfg_applied_event{ue_idx, u.crnti});

    // Remove UE from fallback mode.
    u.get_pcell().set_fallback_state(false);

    // Add UE to slice scheduler, once it leaves fallback mode.
    pcell.slice_sched->config_applied(ue_idx);
  };

  if (not common_events.try_push(common_event_t{ue_idx, handle_ue_config_applied_impl})) {
    logger.warning("ue={}: Discarding UE config applied event. Cause: Event queue is full", ue_idx);
  }
}

void ue_event_manager::handle_ul_bsr_indication(const ul_bsr_indication_message& bsr_ind)
{
  srsran_sanity_check(cell_exists(bsr_ind.cell_index), "Invalid cell index");

  auto handle_ul_bsr_ind_impl = [this, bsr_ind]() {
    if (not ue_db.contains(bsr_ind.ue_index)) {
      log_invalid_ue_index(bsr_ind.ue_index, "BSR");
      return;
    }
    auto&           u         = ue_db[bsr_ind.ue_index];
    du_cell_index_t pcell_idx = u.get_pcell().cell_index;

#ifdef JBPF_ENABLED
    hook_mac_sched_ul_bsr_indication(const_cast<void*>(static_cast<const void*>(&bsr_ind)),
      0, u.get_pcell().get_cell_cfg().pci, (uint16_t)bsr_ind.crnti, sizeof(ul_bsr_indication_message));
#endif

    // Handle event.
    u.handle_bsr_indication(bsr_ind);

    if (u.get_pcell().is_in_fallback_mode()) {
      // Signal SRB fallback scheduler with the new SRB0/SRB1 buffer state.
      du_cells[pcell_idx].fallback_sched->handle_ul_bsr_indication(bsr_ind.ue_index, bsr_ind);
    }

    // Log event.
    if (du_cells[pcell_idx].ev_logger->enabled()) {
      scheduler_event_logger::bsr_event event{};
      event.ue_index             = bsr_ind.ue_index;
      event.rnti                 = bsr_ind.crnti;
      event.type                 = bsr_ind.type;
      event.reported_lcgs        = bsr_ind.reported_lcgs;
      event.tot_ul_pending_bytes = units::bytes{u.pending_ul_newtx_bytes()};
      du_cells[pcell_idx].ev_logger->enqueue(event);
    }

    // Notify metrics handler.
    du_cells[pcell_idx].metrics->handle_ul_bsr_indication(bsr_ind);
  };

  if (not common_events.try_push(common_event_t{bsr_ind.ue_index, handle_ul_bsr_ind_impl})) {
    logger.warning("ue={}: Discarding UE BSR. Cause: Event queue is full", bsr_ind.ue_index);
  }
}

void ue_event_manager::handle_ul_phr_indication(const ul_phr_indication_message& phr_ind)
{
  for (const cell_ph_report& cell_phr : phr_ind.phr.get_phr()) {
    srsran_sanity_check(cell_exists(cell_phr.serv_cell_id), "Invalid serving cell index={}", cell_phr.serv_cell_id);

    if (not cell_specific_events[cell_phr.serv_cell_id].try_push(
            cell_event_t{phr_ind.ue_index,
                         [this, cell_phr, phr_ind](ue_cell& ue_cc) {
                           ue_cc.channel_state_manager().handle_phr(cell_phr);

#ifdef JBPF_ENABLED
                           hook_mac_sched_ul_phr_indication(const_cast<void*>(static_cast<const void*>(&phr_ind)),
                              0, ue_cc.get_cell_cfg().pci, (uint16_t)phr_ind.rnti, sizeof(ul_phr_indication_message));
#endif

                           // Log event.
                           scheduler_event_logger::phr_event event{};
                           event.ue_index   = phr_ind.ue_index;
                           event.rnti       = phr_ind.rnti;
                           event.cell_index = cell_phr.serv_cell_id;
                           event.ph         = cell_phr.ph;
                           event.p_cmax     = cell_phr.p_cmax;
                           du_cells[cell_phr.serv_cell_id].ev_logger->enqueue(event);

                           // Notify metrics handler.
                           du_cells[cell_phr.serv_cell_id].metrics->handle_ul_phr_indication(phr_ind);
                         },
                         "UL PHR",
                         true})) {
      logger.warning("Discarding PHR. Cause: Event queue is full");
    }
  }
}

void ue_event_manager::handle_crc_indication(const ul_crc_indication& crc_ind)
{
  srsran_assert(cell_exists(crc_ind.cell_index), "Invalid cell index");
  for (unsigned i = 0, e = crc_ind.crcs.size(); i != e; ++i) {
    if (not cell_specific_events[crc_ind.cell_index].try_push(cell_event_t{
            crc_ind.crcs[i].ue_index,
            [this, sl_rx = crc_ind.sl_rx, crc = crc_ind.crcs[i]](ue_cell& ue_cc) {
              const double delay_ms =
                  static_cast<double>(last_sl - sl_rx) *
                  (static_cast<double>(10) / static_cast<double>(du_cells[ue_cc.cell_index].cfg->nof_slots_per_frame));

#ifdef JBPF_ENABLED
              hook_mac_sched_crc_indication(const_cast<void*>(static_cast<const void*>(&crc)),
                0, ue_cc.get_cell_cfg().pci, (uint16_t)crc.rnti, sizeof(ul_crc_pdu_indication));
#endif

              const int tbs = ue_cc.handle_crc_pdu(sl_rx, crc);
              if (tbs < 0) {
                return;
              }

              // Process Timing Advance Offset.
              if (crc.tb_crc_success and crc.time_advance_offset.has_value() and crc.ul_sinr_dB.has_value()) {
                ue_db[ue_cc.ue_index].handle_ul_n_ta_update_indication(
                    ue_cc.cell_index, crc.ul_sinr_dB.value(), crc.time_advance_offset.value());
              }

              // Log event.
              du_cells[ue_cc.cell_index].ev_logger->enqueue(scheduler_event_logger::crc_event{
                  crc.ue_index, crc.rnti, ue_cc.cell_index, sl_rx, crc.harq_id, crc.tb_crc_success, crc.ul_sinr_dB});

              // Notify metrics handler.
              du_cells[ue_cc.cell_index].metrics->handle_crc_indication(crc, units::bytes{(unsigned)tbs});
              du_cells[ue_cc.cell_index].metrics->handle_ul_delay(crc.ue_index, delay_ms);
            },
            "CRC",
            true})) {
      logger.warning("Discarding CRC. Cause: Event queue is full");
    }
  }
}

void ue_event_manager::handle_harq_ind(ue_cell&                               ue_cc,
                                       slot_point                             uci_sl,
                                       span<const mac_harq_ack_report_status> harq_bits,
                                       std::optional<float>                   pucch_snr)
{
  for (unsigned harq_idx = 0, harq_end_idx = harq_bits.size(); harq_idx != harq_end_idx; ++harq_idx) {
    // Update UE HARQ state with received HARQ-ACK.
    std::optional<ue_cell::dl_ack_info_result> result =
        ue_cc.handle_dl_ack_info(uci_sl, harq_bits[harq_idx], harq_idx, pucch_snr);
    if (result.has_value()) {
      // Respective HARQ was found.
      const units::bytes tbs{result->h_dl.get_grant_params().tbs_bytes};

      // Log Event.
      du_cells[ue_cc.cell_index].ev_logger->enqueue(scheduler_event_logger::harq_ack_event{
          ue_cc.ue_index, ue_cc.rnti(), ue_cc.cell_index, uci_sl, result->h_dl.id(), harq_bits[harq_idx], tbs});

      // In case the HARQ process is not waiting for more HARQ-ACK bits. Notify metrics handler with HARQ outcome.
      if (result->update == dl_harq_process_handle::status_update::acked or
          result->update == dl_harq_process_handle::status_update::nacked) {
        du_cells[ue_cc.cell_index].metrics->handle_dl_harq_ack(
            ue_cc.ue_index, result->update == dl_harq_process_handle::status_update::acked, tbs);
      }
    }
  }
}

void ue_event_manager::handle_csi(ue_cell& ue_cc, const csi_report_data& csi_rep)
{
  // Forward CSI bits to UE.
  ue_cc.handle_csi_report(csi_rep);

  // Log event.
  du_cells[ue_cc.cell_index].ev_logger->enqueue(
      scheduler_event_logger::csi_report_event{ue_cc.ue_index, ue_cc.rnti(), csi_rep});
}

void ue_event_manager::handle_uci_indication(const uci_indication& ind)
{
  srsran_sanity_check(cell_exists(ind.cell_index), "Invalid cell index");

  for (unsigned i = 0, e = ind.ucis.size(); i != e; ++i) {
    const uci_indication::uci_pdu& uci = ind.ucis[i];

    if (not cell_specific_events[ind.cell_index].try_push(cell_event_t{
            uci.ue_index,
            [this, uci_sl = ind.slot_rx, uci_pdu = uci](ue_cell& ue_cc) {

#ifdef JBPF_ENABLED
              hook_mac_sched_uci_indication(const_cast<void*>(static_cast<const void*>(&uci_pdu)),
                0, ue_cc.get_cell_cfg().pci, (uint16_t)uci_pdu.crnti, sizeof(uci_indication::uci_pdu));
#endif

              if (const auto* pucch_f0f1 = std::get_if<uci_indication::uci_pdu::uci_pucch_f0_or_f1_pdu>(&uci_pdu.pdu)) {
                // Process DL HARQ ACKs.
                if (not pucch_f0f1->harqs.empty()) {
                  handle_harq_ind(ue_cc, uci_sl, pucch_f0f1->harqs, pucch_f0f1->ul_sinr_dB);
                }

                // Process SRs.
                if (pucch_f0f1->sr_detected) {
                  // Handle SR indication.
                  ue_db[ue_cc.ue_index].handle_sr_indication();
                  du_cells[ue_cc.cell_index].fallback_sched->handle_sr_indication(ue_cc.ue_index);

                  // Log SR event.
                  du_cells[ue_cc.cell_index].ev_logger->enqueue(
                      scheduler_event_logger::sr_event{ue_cc.ue_index, ue_cc.rnti()});
                }

                const bool is_uci_valid = not pucch_f0f1->harqs.empty() or pucch_f0f1->sr_detected;
                // Process Timing Advance Offset.
                if (is_uci_valid and pucch_f0f1->time_advance_offset.has_value() and
                    pucch_f0f1->ul_sinr_dB.has_value()) {
                  ue_db[ue_cc.ue_index].handle_ul_n_ta_update_indication(
                      ue_cc.cell_index, pucch_f0f1->ul_sinr_dB.value(), pucch_f0f1->time_advance_offset.value());
                }
              } else if (const auto* pusch_pdu = std::get_if<uci_indication::uci_pdu::uci_pusch_pdu>(&uci_pdu.pdu)) {
                // Process DL HARQ ACKs.
                if (not pusch_pdu->harqs.empty()) {
                  handle_harq_ind(ue_cc, uci_sl, pusch_pdu->harqs, std::nullopt);
                }

                // Process CSI.
                if (pusch_pdu->csi.has_value()) {
                  handle_csi(ue_cc, *pusch_pdu->csi);
                }
              } else if (const auto* pucch_f2f3f4 =
                             std::get_if<uci_indication::uci_pdu::uci_pucch_f2_or_f3_or_f4_pdu>(&uci_pdu.pdu)) {
                // Process DL HARQ ACKs.
                if (not pucch_f2f3f4->harqs.empty()) {
                  handle_harq_ind(ue_cc, uci_sl, pucch_f2f3f4->harqs, pucch_f2f3f4->ul_sinr_dB);
                }

                // Process SRs.
                const size_t sr_bit_position_with_1_sr_bit = 0;
                if (not pucch_f2f3f4->sr_info.empty() and pucch_f2f3f4->sr_info.test(sr_bit_position_with_1_sr_bit)) {
                  // Handle SR indication.
                  ue_db[ue_cc.ue_index].handle_sr_indication();

                  // Log SR event.
                  du_cells[ue_cc.cell_index].ev_logger->enqueue(
                      scheduler_event_logger::sr_event{ue_cc.ue_index, ue_cc.rnti()});
                }

                // Process CSI.
                if (pucch_f2f3f4->csi.has_value()) {
                  handle_csi(ue_cc, *pucch_f2f3f4->csi);
                }

                const bool is_uci_valid =
                    not pucch_f2f3f4->harqs.empty() or
                    (not pucch_f2f3f4->sr_info.empty() and pucch_f2f3f4->sr_info.test(sr_bit_position_with_1_sr_bit)) or
                    pucch_f2f3f4->csi.has_value();
                // Process Timing Advance Offset.
                if (is_uci_valid and pucch_f2f3f4->time_advance_offset.has_value() and
                    pucch_f2f3f4->ul_sinr_dB.has_value()) {
                  ue_db[ue_cc.ue_index].handle_ul_n_ta_update_indication(
                      ue_cc.cell_index, pucch_f2f3f4->ul_sinr_dB.value(), pucch_f2f3f4->time_advance_offset.value());
                }
              }

              // Report the UCI PDU to the metrics handler.
              du_cells[ue_cc.cell_index].metrics->handle_uci_pdu_indication(uci_pdu);
            },
            "UCI",
            // Note: We do not warn if the UE is not found, because there is this transient period when the UE
            // is about to receive and process the RRC Release, but it is still sending CSI or SR in the PUCCH. If we
            // stop the PUCCH scheduling for the UE about to be released, we could risk interference between UEs in the
            // PUCCH.
            false})) {
      logger.warning("UCI discarded. Cause: Event queue is full");
    }
  }
}

void ue_event_manager::handle_srs_indication(const srs_indication& ind)
{
  srsran_sanity_check(cell_exists(ind.cell_index), "Invalid cell index");

  for (unsigned i = 0, e = ind.srss.size(); i != e; ++i) {
    const srs_indication::srs_indication_pdu& srs_pdu = ind.srss[i];

#ifdef JBPF_ENABLED
    ue* u = ue_db.find_by_rnti(srs_pdu.rnti);
    if (u ) {
      hook_mac_sched_srs_indication(const_cast<void*>(static_cast<const void*>(&srs_pdu)),
        0, u->get_pcell().get_cell_cfg().pci, (uint16_t)srs_pdu.rnti, sizeof(srs_indication::srs_indication_pdu));
    }
#endif

    if (not cell_specific_events[ind.cell_index].try_push(cell_event_t{
            srs_pdu.ue_index,
            [this, channel_matrix = srs_pdu.channel_matrix, time_advance_offset = srs_pdu.time_advance_offset](
                ue_cell& ue_cc) {
              // Indicate the channel matrix.
              ue_cc.handle_srs_channel_matrix(channel_matrix);

              // Handle time aligment measurement if present.
              if (time_advance_offset.has_value()) {
                // Assume some SINR for the TA feedback using the channel matrix topology and near zero noise variance.
                float frobenius_norm = channel_matrix.frobenius_norm();
                float noise_var      = near_zero;
                float sinr_dB        = convert_power_to_dB(frobenius_norm * frobenius_norm / noise_var);

                // Notify UL TA update.
                ue_db[ue_cc.ue_index].handle_ul_n_ta_update_indication(
                    ue_cc.cell_index, sinr_dB, time_advance_offset.value());
              }
            },
            "SRS",
            false})) {
      logger.warning("SRS indication discarded. Cause: Event queue is full");
    }
  }
}

void ue_event_manager::handle_dl_mac_ce_indication(const dl_mac_ce_indication& ce)
{
  auto handle_mac_ce_impl = [this, ce]() {
    if (not ue_db.contains(ce.ue_index)) {
      log_invalid_ue_index(ce.ue_index, "DL MAC CE");
      return;
    }
    auto& u = ue_db[ce.ue_index];

#ifdef JBPF_ENABLED
    hook_mac_sched_dl_mac_ce_indication(const_cast<void*>(static_cast<const void*>(&ce)),
      0, u.get_pcell().get_cell_cfg().pci, (uint16_t)u.crnti, sizeof(dl_mac_ce_indication));
#endif

    u.handle_dl_mac_ce_indication(ce);

    // Notify SRB fallback scheduler upon receiving ConRes CE indication.
    if (ce.ce_lcid == lcid_dl_sch_t::UE_CON_RES_ID) {
      du_cells[ue_db[ce.ue_index].get_pcell().cell_index].fallback_sched->handle_conres_indication(ce.ue_index);
    }

    // Log event.
    du_cells[u.get_pcell().cell_index].ev_logger->enqueue(ce);
  };

  if (not common_events.try_push(common_event_t{ce.ue_index, handle_mac_ce_impl})) {
    logger.warning("ue={} lcid={}: DL MAC CE discarded. Cause: Event queue is full", ce.ue_index, ce.ce_lcid);
  }
}

void ue_event_manager::handle_dl_buffer_state_indication(const dl_buffer_state_indication_message& bs)
{
#ifdef JBPF_ENABLED
  auto& u = ue_db[bs.ue_index];
  hook_mac_sched_dl_buffer_state_indication(const_cast<void*>(static_cast<const void*>(&bs)),
    0, u.get_pcell().get_cell_cfg().pci, (uint16_t)u.crnti, sizeof(dl_buffer_state_indication_message));
#endif

  dl_bo_mng->handle_dl_buffer_state_indication(bs);
}

static void handle_discarded_pusch(const cell_slot_resource_allocator& prev_slot_result, ue_repository& ue_db)
{
  for (const ul_sched_info& grant : prev_slot_result.result.ul.puschs) {
    ue* u = ue_db.find_by_rnti(grant.pusch_cfg.rnti);
    if (u == nullptr) {
      // UE has been removed.
      continue;
    }

    // - The lower layers will not attempt to decode the PUSCH and will not send any CRC indication.
    std::optional<ul_harq_process_handle> h_ul = u->get_pcell().harqs.ul_harq(to_harq_id(grant.pusch_cfg.harq_id));
    if (h_ul.has_value()) {
      // Note: We don't use this cancellation to update the UL OLLA, as we shouldn't take lates into account in link
      // adaptation.
      if (h_ul->nof_retxs() == 0) {
        // Given that the PUSCH grant was discarded before it reached the PHY, the "new_data" flag was not handled
        // and the UL softbuffer was not reset. To avoid mixing different TBs in the softbuffer, it is important to
        // reset the UL HARQ process.
        h_ul->reset();
      } else {
        // To avoid a long UL HARQ timeout window (due to lack of CRC indication), it is important to force a NACK
        // in the UL HARQ process.
        h_ul->ul_crc_info(false);
      }
    }

    // - The lower layers will not attempt to decode any UCI in the PUSCH and will not send any UCI indication.
    if (grant.uci.has_value() and grant.uci->harq.has_value() and grant.uci->harq->harq_ack_nof_bits > 0) {
      // To avoid a long DL HARQ timeout window (due to lack of UCI indication), it is important to NACK the
      // DL HARQ processes with UCI falling in this slot.
      // Note: We don't use this cancellation to update the DL OLLA, as we shouldn't take lates into account in link
      // adaptation.
      u->get_pcell().harqs.uci_sched_failed(prev_slot_result.slot);
    }
  }
}

static void handle_discarded_pucch(const cell_slot_resource_allocator& prev_slot_result, ue_repository& ue_db)
{
  for (const auto& pucch : prev_slot_result.result.ul.pucchs) {
    ue* u = ue_db.find_by_rnti(pucch.crnti);
    if (u == nullptr) {
      // UE has been removed.
      continue;
    }
    bool has_harq_ack = false;
    switch (pucch.format) {
      case pucch_format::FORMAT_1:
        has_harq_ack = pucch.format_1.harq_ack_nof_bits > 0;
        break;
      case pucch_format::FORMAT_2:
        has_harq_ack = pucch.format_2.harq_ack_nof_bits > 0;
        break;
      default:
        break;
    }

    // - The lower layers will not attempt to decode the PUCCH and will not send any UCI indication.
    if (has_harq_ack) {
      // Note: To avoid a long DL HARQ timeout window (due to lack of UCI indication), it is important to force a NACK
      // in the DL HARQ processes with UCI falling in this slot.
      // Note: We don't use this cancellation to update the DL OLLA, as we shouldn't take lates into account in link
      // adaptation.
      u->get_pcell().harqs.uci_sched_failed(prev_slot_result.slot);
    }
  }
}

void ue_event_manager::handle_error_indication(slot_point                            sl_tx,
                                               du_cell_index_t                       cell_index,
                                               scheduler_slot_handler::error_outcome event)
{
  auto handle_error_impl = [this, sl_tx, cell_index, event]() {
    // Handle Error Indication.

    const cell_slot_resource_allocator* prev_slot_result = du_cells[cell_index].res_grid->get_history(sl_tx);
    if (prev_slot_result == nullptr) {
      logger.warning("cell={}, slot={}: Discarding error indication. Cause: Scheduler results associated with the slot "
                     "of the error indication have already been erased",
                     cell_index,
                     sl_tx);
      return;
    }

    // In case DL PDCCHs were skipped, there will be the following consequences:
    // - The UE will not decode the PDSCH and will not send the respective UCI.
    // - The UE won't update the HARQ NDI, if new HARQ TB.
    // - The UCI indication coming later from the lower layers will likely contain a HARQ-ACK=DTX.
    // In case UL PDCCHs were skipped, there will be the following consequences:
    // - The UE will not decode the PUSCH.
    // - The UE won't update the HARQ NDI, if new HARQ TB.
    // - The CRC indication coming from the lower layers will likely be CRC=KO.
    // - Any UCI in the respective PUSCH will be likely reported as HARQ-ACK=DTX.
    // In neither of the cases, the HARQs will timeout, because we did not lose the UCI/CRC indications in the
    // lower layers. We do not need to cancel associated PUSCH grant (in UL PDCCH case) because it is important
    // that the PUSCH "new_data" flag reaches the lower layers, telling them whether the UL HARQ buffer needs to
    // be reset or not. Cancelling HARQ retransmissions is dangerous as it increases the chances of NDI
    // ambiguity.

    // In case of PDSCH grants being discarded, there will be the following consequences:
    // - If the PDCCH was not discarded,the UE will fail to decode the PDSCH and will send an HARQ-ACK=NACK. The
    // scheduler will retransmit the respective DL HARQ. No actions required.

    // In case of PUCCH and PUSCH grants being discarded.
    if (event.pusch_and_pucch_discarded) {
      handle_discarded_pusch(*prev_slot_result, ue_db);

      handle_discarded_pucch(*prev_slot_result, ue_db);
    }

    // Log event.
    du_cells[cell_index].ev_logger->enqueue(scheduler_event_logger::error_indication_event{sl_tx, event});

    // Report metrics.
    du_cells[cell_index].metrics->handle_error_indication();
  };

  if (not common_events.try_push(common_event_t{INVALID_DU_UE_INDEX, handle_error_impl})) {
    logger.warning("Discarding error indication for slot={}. Cause: Event queue is full", sl_tx);
  }
}

void ue_event_manager::process_common(slot_point sl, du_cell_index_t cell_index)
{
  bool new_slot_detected = last_sl != sl;
  if (not new_slot_detected) {
    // This slot has already been processed.
    return;
  }
  last_sl = sl;

  // Process events for UEs whose PCell matches cell_index argument.
  common_event_t ev{MAX_NOF_DU_UES, []() {}};
  while (common_events.try_pop(ev)) {
    if (ev.ue_index == MAX_NOF_DU_UES) {
      // The UE is being created.
      ev.callback();
    } else {
      if (not ue_db.contains(ev.ue_index)) {
        // Can't find UE. Log error.
        log_invalid_ue_index(ev.ue_index);
        continue;
      }
      ev.callback();
    }
  }

  // Process pending DL Buffer Occupancy reports.
  dl_bo_mng->slot_indication(sl);
}

void ue_event_manager::process_cell_specific(du_cell_index_t cell_index)
{
  // Pop and process pending cell-specific events.
  auto&        cell_events = cell_specific_events[cell_index];
  cell_event_t ev{INVALID_DU_UE_INDEX, [](ue_cell&) {}, "invalid", true};
  while (cell_events.try_pop(ev)) {
    if (not ue_db.contains(ev.ue_index)) {
      log_invalid_ue_index(ev.ue_index, ev.event_name, ev.warn_if_ignored);
      continue;
    }
    ue&      ue    = ue_db[ev.ue_index];
    ue_cell* ue_cc = ue.find_cell(cell_index);
    if (ue_cc == nullptr) {
      log_invalid_cc(ev.ue_index, cell_index);
      continue;
    }
    ev.callback(*ue_cc);
  }
}

void ue_event_manager::run(slot_point sl, du_cell_index_t cell_index)
{
  srsran_sanity_check(cell_exists(cell_index), "Invalid cell index {}", cell_index);

  // Process common events.
  process_common(sl, cell_index);

  // Process carrier specific events.
  process_cell_specific(cell_index);
}

void ue_event_manager::add_cell(const cell_creation_event& cell_ev)
{
  const du_cell_index_t cell_index = cell_ev.cell_res_grid.cell_index();
  srsran_assert(not cell_exists(cell_index), "Overwriting cell configurations not supported");

  du_cells[cell_index].cfg            = &cell_ev.cell_res_grid.cfg;
  du_cells[cell_index].res_grid       = &cell_ev.cell_res_grid;
  du_cells[cell_index].cell_harqs     = &cell_ev.cell_harqs;
  du_cells[cell_index].fallback_sched = &cell_ev.fallback_sched;
  du_cells[cell_index].uci_sched      = &cell_ev.uci_sched;
  du_cells[cell_index].slice_sched    = &cell_ev.slice_sched;
  du_cells[cell_index].srs_sched      = &cell_ev.srs_sched;
  du_cells[cell_index].metrics        = &cell_ev.metrics;
  du_cells[cell_index].ev_logger      = &cell_ev.ev_logger;

  while (cell_specific_events.size() <= cell_index) {
    cell_specific_events.emplace_back(CELL_EVENT_LIST_SIZE);
  }
}

bool ue_event_manager::cell_exists(du_cell_index_t cell_index) const
{
  return cell_index < MAX_NOF_DU_CELLS and du_cells[cell_index].cfg != nullptr;
}

void ue_event_manager::log_invalid_ue_index(du_ue_index_t ue_index, const char* event_name, bool warn_if_ignored) const
{
  srslog::log_channel& log_channel = warn_if_ignored ? logger.warning : logger.info;
  log_channel("{} for ue={} discarded. Cause: UE with provided Id does not exist", event_name, ue_index);
}

void ue_event_manager::log_invalid_cc(du_ue_index_t ue_index, du_cell_index_t cell_index) const
{
  logger.warning("Event for ue={} ignored. Cause: Cell {} is not configured.", ue_index, cell_index);
}
