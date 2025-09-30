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

#include "inter_slice_scheduler.h"
#include "../policy/scheduler_policy_factory.h"
#include "../support/pusch/pusch_td_resource_indices.h"
#include "srsran/srslog/srslog.h"

#ifdef JBPF_ENABLED
DEFINE_JBPF_CTRL_HOOK(mac_sched_slice_mgmt)
#endif


using namespace srsran;

inter_slice_scheduler::inter_slice_scheduler(const cell_configuration& cell_cfg_, ue_repository& ues_) :
  cell_cfg(cell_cfg_),
  logger(srslog::fetch_basic_logger("SCHED")),
  ues(ues_),
  valid_pusch_td_list_per_slot(get_fairly_distributed_pusch_td_resource_indices(cell_cfg))
{
  // Create a number of slices equal to the number of configured RRM Policy members + 1 (default SRB slice) + 1 (default
  // DRB slice).
  unsigned capacity = cell_cfg.rrm_policy_members.size() + 2;
  dl_prio_queue.reserve(capacity);
  // We use a larger capacity for the UL priority queue to account for the maximum number of PUSCH TD resources.
  ul_prio_queue.reserve(capacity * pusch_constants::MAX_NOF_PUSCH_TD_RES_ALLOCS);
  slices.reserve(capacity);

  // NOTE: We assume nof. CRBs in a cell for both DL and UL are same.
  const unsigned cell_max_rbs = cell_cfg.dl_cfg_common.init_dl_bwp.generic_params.crbs.length();

  // Create RAN slice instances.
  // Default SRB slice.
  // NOTE: We set \c min_prb for default SRB slice to maximum nof. PRBs of a UE carrier to give maximum priority to this
  // slice.
  slices.emplace_back(SRB_RAN_SLICE_ID,
                      cell_cfg,
                      slice_rrm_policy_config{.min_prb  = cell_max_rbs,
                                              .max_prb  = cell_max_rbs,
                                              .priority = slice_rrm_policy_config::max_priority},
                      create_scheduler_strategy(cell_cfg.expert_cfg.ue, cell_cfg.cell_index));
  // Default DRB slice.
  slices.emplace_back(DEFAULT_DRB_RAN_SLICE_ID,
                      cell_cfg,
                      slice_rrm_policy_config{.max_prb = cell_max_rbs},
                      create_scheduler_strategy(cell_cfg.expert_cfg.ue, cell_cfg.cell_index));
  // NOTE: RAN slice IDs 0 and 1 are reserved for default SRB and default DRB slice respectively.
  ran_slice_id_t id_count{2};
  // Configured RRM policy members.
  for (const slice_rrm_policy_config& rrm : cell_cfg.rrm_policy_members) {
    // Adjust maximum PRBs per slice based on the number of PRBs in a cell.
    auto rrm_adjusted     = rrm;
    rrm_adjusted.max_prb  = std::min(rrm.max_prb, cell_max_rbs);
    rrm_adjusted.priority = std::min(rrm.priority, slice_rrm_policy_config::max_priority);
    // Set policy scheduler based on slice configuration.
    scheduler_ue_expert_config slice_scheduler_ue_expert_cfg{cell_cfg.expert_cfg.ue};
    slice_scheduler_ue_expert_cfg.strategy_cfg = rrm.policy_sched_cfg;
    // Create custom RAN slice.
    slices.emplace_back(id_count,
                        cell_cfg,
                        rrm_adjusted,
                        create_scheduler_strategy(slice_scheduler_ue_expert_cfg, cell_cfg.cell_index));
    ++id_count;
  }

#ifdef JBPF_ENABLED
  map_slice_allocation_to_jbpf();
  // set candidate as current config
  jbpf_slice_allocation_candidiate = jbpf_slice_allocation;


  printf("jbpf_slice_allocation_candidiate: num_slices %d \n", jbpf_slice_allocation_candidiate.num_slices);
  int i=0;
  for (i=0; i<jbpf_slice_allocation_candidiate.num_slices; i++){

    printf("  [%d] pci %d plmn_id %d sst %d sd %d min_prb %d max_prb %d priority %d \n",
      i, 
      jbpf_slice_allocation_candidiate.slices[i].pci,
      jbpf_slice_allocation_candidiate.slices[i].plmn_id,
      jbpf_slice_allocation_candidiate.slices[i].nssai.sst,
      jbpf_slice_allocation_candidiate.slices[i].nssai.sd,
      jbpf_slice_allocation_candidiate.slices[i].min_prb_policy_ratio,
      jbpf_slice_allocation_candidiate.slices[i].max_prb_policy_ratio,
      jbpf_slice_allocation_candidiate.slices[i].priority);
  }

#endif
}


#ifdef JBPF_ENABLED

void inter_slice_scheduler::map_slice_allocation_to_jbpf(void) {

  // init the jbpf_slice_allocation
  jbpf_slice_allocation.num_slices = 0;

  for (auto& slice : slices) {

    // dont encode the default slices
    if (slice.inst.id == SRB_RAN_SLICE_ID or slice.inst.id == DEFAULT_DRB_RAN_SLICE_ID)
    {
      continue;
    }

    jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].pci = static_cast<uint16_t>(slice.inst.cell_cfg->pci);
    jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].plmn_id =  slice.inst.cfg.rrc_member.plmn_id.to_bcd();
    jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].nssai.sst = slice.inst.cfg.rrc_member.s_nssai.sst.value();
    jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].nssai.sd = slice.inst.cfg.rrc_member.s_nssai.sd.value();
    jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].min_prb_policy_ratio = slice.inst.cfg.min_prb_policy_ratio;
    jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].max_prb_policy_ratio = slice.inst.cfg.max_prb_policy_ratio;
    jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].priority = slice.inst.cfg.priority;
    jbpf_slice_allocation.num_slices++;
  }
}

void inter_slice_scheduler::map_slice_allocation_from_jbpf(void) {

  // the slice order will not be changed so no re-ordering is required.
  jbpf_slice_allocation.num_slices = 0;
  for (auto& slice : slices) {
    
    // dont encode the default slices
    if (slice.inst.id == SRB_RAN_SLICE_ID or slice.inst.id == DEFAULT_DRB_RAN_SLICE_ID)
    {
      continue;
    }    
    slice.inst.cfg.min_prb_policy_ratio = jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].min_prb_policy_ratio;
    slice.inst.cfg.max_prb_policy_ratio = jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].max_prb_policy_ratio;
    slice.inst.cfg.priority = jbpf_slice_allocation.slices[jbpf_slice_allocation.num_slices].priority;

    // update the min_prb and max_prb
    slice.inst.cfg.min_prb = (slice.inst.cfg.nof_cell_crbs * slice.inst.cfg.min_prb_policy_ratio) / 100;
    slice.inst.cfg.max_prb = (slice.inst.cfg.nof_cell_crbs * slice.inst.cfg.max_prb_policy_ratio) / 100;

    jbpf_slice_allocation.num_slices++;

  }
}

bool inter_slice_scheduler::validate_map_slice_allocation_candidate(void) {

  // validation includes checking ...
  // - number of slices has not changed
  // - per slice
  //     - plmn, pci, nssai has not changed 
  // - check correct ranges for min_prb_policy_ratio, max_prb_policy_ratio and priority

#define MIN_PRB_RADIO_RANGE_LOW   (0)
#define MIN_PRB_RADIO_RANGE_HIGH  (100)
#define MAX_PRB_RADIO_RANGE_LOW   (1)
#define MAX_PRB_RADIO_RANGE_HIGH  (100)
#define PRIORITY_RANGE_LOW        (0)
#define PRIORITY_RANGE_HIGH       (254)

  if (jbpf_slice_allocation_candidiate.num_slices != jbpf_slice_allocation.num_slices) {
    printf("Slave allocation candidiate failure : Number of slices has changed from %d to %d\n", jbpf_slice_allocation_candidiate.num_slices, jbpf_slice_allocation.num_slices);
    return false;
  }
  int i=0;
  for (i=0; i<jbpf_slice_allocation_candidiate.num_slices; i++){
    if (jbpf_slice_allocation_candidiate.slices[i].pci != jbpf_slice_allocation.slices[i].pci) {
      printf("Slave allocation candidiate failure : PCI changed for slice idx %d\n", i);
      return false;
    }
    if (jbpf_slice_allocation_candidiate.slices[i].plmn_id != jbpf_slice_allocation.slices[i].plmn_id) {
      printf("Slave allocation candidiate failure : plmn_id changed for slice idx %d\n", i);
      return false;
    }
    if (!((jbpf_slice_allocation_candidiate.slices[i].min_prb_policy_ratio >= MIN_PRB_RADIO_RANGE_LOW) && 
        (jbpf_slice_allocation_candidiate.slices[i].min_prb_policy_ratio <= MIN_PRB_RADIO_RANGE_HIGH))) {
        printf("min_prb_policy_ratio %d not in range [%d - %d] \n", jbpf_slice_allocation_candidiate.slices[i].min_prb_policy_ratio,
            MIN_PRB_RADIO_RANGE_LOW, MIN_PRB_RADIO_RANGE_HIGH);
      return false;
    }
    if (!((jbpf_slice_allocation_candidiate.slices[i].max_prb_policy_ratio >= MAX_PRB_RADIO_RANGE_LOW) && 
        (jbpf_slice_allocation_candidiate.slices[i].max_prb_policy_ratio <= MAX_PRB_RADIO_RANGE_HIGH))) {
        printf("max_prb_policy_ratio %d not in range [%d - %d] \n", jbpf_slice_allocation_candidiate.slices[i].max_prb_policy_ratio,
            MAX_PRB_RADIO_RANGE_LOW, MAX_PRB_RADIO_RANGE_HIGH);
      return false;
    }
    if (!((jbpf_slice_allocation_candidiate.slices[i].priority >= PRIORITY_RANGE_LOW) && 
        (jbpf_slice_allocation_candidiate.slices[i].priority <= PRIORITY_RANGE_HIGH))) {
        printf("priority %d not in range [%d - %d] \n", jbpf_slice_allocation_candidiate.slices[i].priority,
            PRIORITY_RANGE_LOW, PRIORITY_RANGE_HIGH);
      return false;
    }
  }
  return true;
}

#endif


void inter_slice_scheduler::slot_indication(slot_point slot_tx, const cell_resource_allocator& res_grid)
{
  // Update the context of each slice.
  if (not current_slot.valid()) {
    // First call.
    current_slot = slot_tx - 1;
  }
  while (current_slot != slot_tx) {
    ++current_slot;
    for (auto& slice : slices) {
      slice.inst.slot_indication(current_slot);
    }
  }

  // Clear the priority queues.
  dl_prio_queue.clear();
  ul_prio_queue.clear();

  if (not cell_cfg.is_dl_enabled(slot_tx)) {
    // If PDCCH is inactive in this slot, we don't generate any slice candidates.
    return;
  }

#ifdef JBPF_ENABLED
  {
    uint32_t ret = ctrl_hook_mac_sched_slice_mgmt(
      slot_tx.sfn(), slot_tx.slot_index(), &jbpf_slice_allocation_candidiate);
    if (ret) {
      printf("Allocation changed by codelet \n");
      if (validate_map_slice_allocation_candidate()) {
        // promote candidiate
        jbpf_slice_allocation = jbpf_slice_allocation_candidiate;
        // update slices with new allocation
        map_slice_allocation_from_jbpf();
        printf("slice allocation successfuly updated \n");
      } else {
        printf("Invalid slice allocation attempted\n");
        // revert to current jbpf_slice_allocation
        jbpf_slice_allocation_candidiate = jbpf_slice_allocation;
      }
    }
  }
#endif

  s_nssai_t internet_slice_nssai{.sst = slice_service_type{1}, .sd = slice_differentiator::create(162).value()};
  s_nssai_t internet2_slice_nssai{.sst = slice_service_type{1}, .sd = slice_differentiator::create(163).value()};

  if ((slot_tx.sfn()==0) && (slot_tx.slot_index()==0))
  {
    printf("########################################################\n");
    printf("########################################################\n");
    printf("########################################################\n");
    printf("########################################################\n");
    for (auto& slice : slices) {

      if ((slice.inst.cfg.rrc_member.s_nssai == internet_slice_nssai) || (slice.inst.cfg.rrc_member.s_nssai == internet2_slice_nssai))
      {
        printf("sfn/slot=%d/%d, BEFORE slice [sst=%d st=%d] to max_prb=%d \n",
          slot_tx.sfn(), slot_tx.slot_index(),
          slice.inst.cfg.rrc_member.s_nssai.sst.value(), 
          slice.inst.cfg.rrc_member.s_nssai.sd.value(),
          slice.inst.cfg.max_prb);
        slice.inst.cfg.max_prb = (slice.inst.cfg.max_prb == 163) ? 109 : 163;
        printf("sfn/slot=%d/%d, Changed slice [sst=%d st=%d] to max_prb=%d \n",
          slot_tx.sfn(), slot_tx.slot_index(),
          slice.inst.cfg.rrc_member.s_nssai.sst.value(), 
          slice.inst.cfg.rrc_member.s_nssai.sd.value(),
          slice.inst.cfg.max_prb);
      }
    }
  }

  // Recompute the priority queues.
  unsigned max_rbs = 0;
  for (const auto& slice : slices) {

#if 0
    if (slot_tx.slot_index()==0)
    {
    printf("sfn/slot=%d/%d, slice : inst(id=%ld cell_cfg=[cell_index=%d cell_group_index=%d pci=%d nof_dl_prbs=%d nof_ul_prbs=%d nof_slots_per_frame=%d]" 
           "cfg=[min_prb_policy_ratio=%d max_prb_policy_ratio=%d min_prb=%d max_prb=%d, prio=%d member=[plmn=%d s_nssai=[sst=%d st=%d]]] \n",
      slot_tx.sfn(), slot_tx.slot_index(),
      static_cast<std::size_t>(slice.inst.id),
      static_cast<uint16_t>(slice.inst.cell_cfg->cell_index),
      static_cast<uint16_t>(slice.inst.cell_cfg->cell_group_index),
      static_cast<uint16_t>(slice.inst.cell_cfg->pci),
      static_cast<uint16_t>(slice.inst.cell_cfg->nof_dl_prbs),
      static_cast<uint16_t>(slice.inst.cell_cfg->nof_ul_prbs),
      static_cast<uint16_t>(slice.inst.cell_cfg->nof_slots_per_frame),
      slice.inst.cfg.min_prb_policy_ratio,
      slice.inst.cfg.max_prb_policy_ratio,
      slice.inst.cfg.min_prb,
      slice.inst.cfg.max_prb,
      slice.inst.cfg.priority,
      slice.inst.cfg.rrc_member.plmn_id.to_bcd(),
      slice.inst.cfg.rrc_member.s_nssai.sst.value(), 
      slice.inst.cfg.rrc_member.s_nssai.sd.value());
    }
#endif


//       /// \remark See O-RAN.WG3.E2SM-RC-R003-v3.00 Section 8.4.3.6
// struct rrm_policy_member {
//   plmn_identity plmn_id = plmn_identity::test_value();
//   /// Single Network Slice Selection Assistance Information (S-NSSAI).
//   s_nssai_t s_nssai;

//   bool operator==(const rrm_policy_member& other) const
//   {
//     return plmn_id == other.plmn_id and s_nssai == other.s_nssai;
//   }
// };



//       struct slice_rrm_policy_config {
//   /// Maximum RAN scheduling policy.
//   constexpr static unsigned max_priority = 255;

//   /// RRM Policy identifier.
//   rrm_policy_member rrc_member;
//   /// Sets the minimum number of PRBs to be allocated to this group.
//   unsigned min_prb = 0;
//   /// Sets the maximum number of PRBs to be allocated to this group.
//   unsigned max_prb = MAX_NOF_PRBS;
//   /// RAN slice scheduling priority. Values: {0, ..., 255}.
//   unsigned priority = 0;
//   /// Policy scheduler configuration for the slice.
//   policy_scheduler_expert_config policy_sched_cfg = time_qos_scheduler_expert_config{};
// };

  // const du_cell_index_t                        cell_index;
  // const du_cell_group_index_t                  cell_group_index;
  // const pci_t                                  pci;
  // const subcarrier_spacing                     scs_common;
  // const unsigned                               nof_dl_prbs;
  // const unsigned                               nof_ul_prbs;
  // const unsigned                               nof_slots_per_frame;







// std::cout << "Slice ID: " << static_cast<std::size_t>(id) << std::endl;


//       struct ran_slice_sched_context {
//     ran_slice_instance                inst;
//     std::unique_ptr<scheduler_policy> policy;

//     ran_slice_sched_context(ran_slice_id_t                    id,
//                             const cell_configuration&         cell_cfg,
//                             const slice_rrm_policy_config&    cfg,
//                             std::unique_ptr<scheduler_policy> policy_) :
//       inst(id, cell_cfg, cfg), policy(std::move(policy_))
//     {
//     }

//     /// Determines the slice candidate priority.
//     priority_type get_prio(bool is_dl, slot_point pdcch_slot, slot_point pxsch_slot, bool slice_resched) const;
//   };











    const bool pdsch_enabled =
        cell_cfg.expert_cfg.ue.enable_csi_rs_pdsch_multiplexing or res_grid[0].result.dl.csi_rs.empty();
    if (pdsch_enabled and slice.inst.pdsch_rb_count < slice.inst.cfg.max_prb) {
      max_rbs = slice.inst.pdsch_rb_count <= slice.inst.cfg.min_prb and slice.inst.cfg.min_prb > 0
                    ? slice.inst.cfg.min_prb
                    : slice.inst.cfg.max_prb;                    
      // printf("pdsch: max_rbs=%d slice.inst.pdsch_rb_count=%d slice.inst.cfg.min_prb=%d  slice.inst.cfg.max_prb=%d \n",
      //   max_rbs, slice.inst.pdsch_rb_count, slice.inst.cfg.min_prb, slice.inst.cfg.max_prb);

      dl_prio_queue.push(slice_candidate_context{
          slice.inst.id, slice.get_prio(true, slot_tx, slot_tx, false), {slice.inst.pdsch_rb_count, max_rbs}, slot_tx});
    }

    // TODO: Revisit when PUSCH time domain resource list is also defined in UE dedicated configuration.
    span<const pusch_time_domain_resource_allocation> pusch_time_domain_list =
        cell_cfg.ul_cfg_common.init_ul_bwp.pusch_cfg_common.value().pusch_td_alloc_list;
    for (const unsigned pusch_td_res_idx :
         valid_pusch_td_list_per_slot[(slot_tx).to_uint() % valid_pusch_td_list_per_slot.size()]) {
      unsigned pusch_delay = pusch_time_domain_list[pusch_td_res_idx].k2 + cell_cfg.ntn_cs_koffset;
      const cell_slot_resource_allocator& pusch_alloc = res_grid[pusch_delay];
      slot_point                          pusch_slot  = slot_tx + pusch_delay;

      unsigned pusch_rb_count = slice.inst.nof_pusch_rbs_allocated(pusch_slot);
      if (pusch_rb_count >= slice.inst.cfg.max_prb) {
        // Slice reached max RBs.
        continue;
      }

      const crb_bitmap pusch_used_crbs =
          pusch_alloc.ul_res_grid.used_crbs(cell_cfg.ul_cfg_common.init_ul_bwp.generic_params.scs,
                                            cell_cfg.ul_cfg_common.init_ul_bwp.generic_params.crbs,
                                            pusch_time_domain_list[pusch_td_res_idx].symbols);
      if (pusch_used_crbs.all()) {
        // No more RBs left to allocated so skip adding slice candidate.
        continue;
      }

      max_rbs = pusch_rb_count <= slice.inst.cfg.min_prb and slice.inst.cfg.min_prb > 0 ? slice.inst.cfg.min_prb
                                                                                        : slice.inst.cfg.max_prb;
      ul_prio_queue.push(slice_candidate_context{
          slice.inst.id, slice.get_prio(false, slot_tx, pusch_slot, false), {pusch_rb_count, max_rbs}, pusch_slot});
    }
  }
}

void inter_slice_scheduler::add_ue(du_ue_index_t ue_idx)
{
  ue* u = fetch_ue_to_update(ue_idx);
  if (u == nullptr) {
    return;
  }

  // Add UE and new bearers.
  add_impl(*u);
}

void inter_slice_scheduler::reconf_ue(du_ue_index_t ue_idx)
{
  // When the UE is reconfigured, it enters fallback mode and only leaves it when config_applied is called.
  // For this reason, we remove the UE from its slices.
  rem_ue(ue_idx);
}

void inter_slice_scheduler::rem_ue(du_ue_index_t ue_idx)
{
  // Remove all logical channels of UE.
  // Note: We take the conservative approach of traversing all slices, because the current UE config might not match
  // the UE repositories inside each slice instance (e.g. in case of fallback or during reconfig).
  for (auto& slice : slices) {
    if (slice.inst.get_ues().contains(ue_idx)) {
      get_policy(slice.inst.id).rem_ue(ue_idx);
    }
    slice.inst.get_ues().rem_ue(ue_idx);
  }
}

void inter_slice_scheduler::config_applied(du_ue_index_t ue_idx)
{
  ue* u = fetch_ue_to_update(ue_idx);
  if (u == nullptr) {
    logger.error("Config applied to a UE that is inactive or in fallback");
    return;
  }

  // Add UE to slices.
  add_impl(*u);
}

void inter_slice_scheduler::add_impl(ue& u)
{
  const ue_configuration& ue_cfg = *u.ue_cfg_dedicated();
  for (logical_channel_config_ptr lc_cfg : *ue_cfg.logical_channels()) {
    ran_slice_instance& sl_inst = get_slice(*lc_cfg);
    if (lc_cfg->lcid != LCID_SRB0 and not sl_inst.get_ues().contains(u.ue_index)) {
      get_policy(sl_inst.id).add_ue(u.ue_index);
    }
    sl_inst.get_ues().add_logical_channel(u, lc_cfg->lcid, lc_cfg->lc_group);
  }
}

ue* inter_slice_scheduler::fetch_ue_to_update(du_ue_index_t ue_idx)
{
  if (not ues.contains(ue_idx)) {
    // UE should be added to the repository at this stage.
    logger.warning("ue={}: Not configuring UE to slice scheduler. Cause: No UE context found", fmt::underlying(ue_idx));
    return nullptr;
  }

  auto&                   u      = ues[ue_idx];
  const ue_configuration& ue_cfg = *u.ue_cfg_dedicated();

  // If UE does not have complete configuration, we won't be added to any slice.
  if (not ue_cfg.is_ue_cfg_complete()) {
    return nullptr;
  }

  // If UE is in fallback mode, we do not add it to the slice scheduler.
  const ue_cell* ue_cc = u.find_cell(cell_cfg.cell_index);
  if (ue_cc == nullptr) {
    logger.warning("ue={}: Not adding UE to slice scheduler. Cause: No UE context found in cell {}",
                   fmt::underlying(ue_cfg.ue_index),
                   fmt::underlying(cell_cfg.cell_index));
    return nullptr;
  }
  if (ue_cc->is_in_fallback_mode()) {
    // Do not include yet the UE in the slice scheduler.
    return nullptr;
  }

  return &u;
}

ran_slice_instance& inter_slice_scheduler::get_slice(const logical_channel_config& lc_cfg)
{
  // Return default SRB slice if LCID belongs to a SRB.
  if (lc_cfg.lcid < LCID_MIN_DRB) {
    return slices[SRB_RAN_SLICE_ID.value()].inst;
  }
  auto it = std::find_if(slices.begin(), slices.end(), [&lc_cfg](const ran_slice_sched_context& slice) {
    return slice.inst.cfg.rrc_member == lc_cfg.rrm_policy;
  });
  if (it == slices.end() or lc_cfg.rrm_policy == rrm_policy_member{}) {
    // Slice with the provided RRM policy member was not found. Return default DRB slice.
    return slices[DEFAULT_DRB_RAN_SLICE_ID.value()].inst;
  }
  return it->inst;
}

template <bool IsDownlink>
std::optional<std::conditional_t<IsDownlink, dl_ran_slice_candidate, ul_ran_slice_candidate>>
inter_slice_scheduler::get_next_candidate()
{
  using candidate_type = std::conditional_t<IsDownlink, dl_ran_slice_candidate, ul_ran_slice_candidate>;

  slice_prio_queue& prio_queue = IsDownlink ? dl_prio_queue : ul_prio_queue;
  while (not prio_queue.empty()) {
    ran_slice_sched_context& chosen_slice = slices[prio_queue.top().id.value()];
    interval<unsigned>       rb_lims      = prio_queue.top().rb_lims;
    slot_point               pxsch_slot   = prio_queue.top().slot_tx;
    prio_queue.pop();

    unsigned rb_count =
        IsDownlink
            ? chosen_slice.inst.pdsch_rb_count
            : chosen_slice.inst
                  .pusch_rb_count_per_slot[pxsch_slot.to_uint() % chosen_slice.inst.pusch_rb_count_per_slot.size()];
    if (not rb_lims.contains(rb_count)) {
      // The slice has been scheduled in this slot with a number of RBs that is not within the limits for this
      // candidate. This could happen, for instance, if the scheduler could not schedule all RBs of a candidate
      // bounded between {RBLimsMin, RBLimsMax}. In this case, the second candidate for the same slice with bounds
      // {RBLimsMax, maxRB} is skipped.
      continue;
    }

    const slice_rrm_policy_config& cfg = chosen_slice.inst.cfg;
    if (cfg.min_prb > 0 and cfg.min_prb != cfg.max_prb and rb_lims.stop() >= cfg.min_prb and
        rb_lims.stop() != cfg.max_prb) {
      // For the special case when minRB ratio>0, the first candidate for this slice was bounded between {RBLimsMin,
      // RBLimsMax}. We re-add the slice as a candidate, this time, with RB bounds {RBLimsMax, maxRB}.
      priority_type prio    = chosen_slice.get_prio(IsDownlink, current_slot, pxsch_slot, true);
      unsigned      min_rbs = rb_count > 0 ? rb_count : cfg.min_prb;
      prio_queue.push(slice_candidate_context{chosen_slice.inst.id, prio, {min_rbs, cfg.max_prb}, pxsch_slot});

      if (prio_queue.top().id == chosen_slice.inst.id and pxsch_slot == prio_queue.top().slot_tx) {
        // The candidate just inserted the queue is the same as the current candidate. We merge both as one single
        // candidate.
        const auto new_rb_max = std::max(rb_lims.stop(), prio_queue.top().rb_lims.stop());
        prio_queue.pop();
        return candidate_type{chosen_slice.inst, pxsch_slot, new_rb_max};
      }
    }

    // Return the candidate.
    return candidate_type{chosen_slice.inst, pxsch_slot, rb_lims.stop()};
  }
  return std::nullopt;
}

std::optional<dl_ran_slice_candidate> inter_slice_scheduler::get_next_dl_candidate()
{
  return get_next_candidate<true>();
}

std::optional<ul_ran_slice_candidate> inter_slice_scheduler::get_next_ul_candidate()
{
  return get_next_candidate<false>();
}

inter_slice_scheduler::priority_type inter_slice_scheduler::ran_slice_sched_context::get_prio(bool       is_dl,
                                                                                              slot_point pdcch_slot,
                                                                                              slot_point pxsch_slot,
                                                                                              bool slice_resched) const
{
  // Note: The positive integer representing the priority of a slice consists of a concatenation of several priority
  // values:
  // 1. slice slot distance (7 most significant bits). It prioritizes slices whose PXSCH slot is closer to the PDCCH
  // slot. This is to avoid scheduling PDSCHs/PUSCHs out-of-order for the same UE.
  // 2. slice min/max RB priority (1 bit). It prioritizes slices that have not yet achieved their minRB ratio.
  // 3. slice traffic priority (8 bits). It differentiates slices based on their configured priority value.
  // 4. delay priority (8 bits), which attributes the highest priority to slices that have not been scheduled for a
  // long time.
  // 5. round-robin based on slot indication count (7 least significant bits).
  // 6. one bit set to 1 to differentiate from "skip" priority.

  static constexpr priority_type slice_dist_bitsize       = 7U;
  static constexpr priority_type slice_dist_bitmask       = (1U << slice_dist_bitsize) - 1U;
  static constexpr priority_type slice_minrb_prio_bitsize = 1U;
  static constexpr priority_type slice_prio_bitsize       = 8U;
  static constexpr priority_type slice_prio_bitmask       = (1U << slice_prio_bitsize) - 1U;
  static constexpr priority_type delay_prio_bitsize       = 8U;
  static constexpr priority_type delay_prio_bitmask       = (1U << delay_prio_bitsize) - 1U;
  static constexpr priority_type rr_bitsize               = 7U;
  static constexpr priority_type rr_bitmask               = (1U << rr_bitsize) - 1U;

  unsigned rb_count = is_dl ? inst.pdsch_rb_count : inst.nof_pusch_rbs_allocated(pxsch_slot);
  if (not inst.active() or rb_count >= inst.cfg.max_prb) {
    // If the slice is not in a state to be scheduled in this slot, return skip priority level.
    return skip_prio;
  }

  // 1. (most significant bits) Prioritize closer slots over slots further away into the future.
  // Note: This is relevant for UL-heavy cases, to avoid scheduling PUSCHs out-of-order for the same UE.
  const priority_type slot_dist =
      slice_dist_bitmask - std::min(static_cast<unsigned>(pxsch_slot - pdcch_slot), slice_dist_bitmask);
  priority_type prio = slot_dist;

  // 2. In case minRB > 0 and minimum RB ratio agreement is not yet reached, we give it a higher priority.
  const priority_type slice_minrb_prio =
      not slice_resched and inst.cfg.min_prb > 0 and rb_count < inst.cfg.min_prb ? 1U : 0U;
  prio = (prio << slice_minrb_prio_bitsize) + slice_minrb_prio;

  // 3. Prioritize slices with higher priority.
  prio = (prio << slice_prio_bitsize) + std::min(inst.cfg.priority, slice_prio_bitmask);

  // 4. Increase priorities of slices that have not been scheduled for a long time.
  const priority_type delay_prio =
      std::min(is_dl ? inst.nof_slots_since_last_pdsch(pxsch_slot) : inst.nof_slots_since_last_pusch(pxsch_slot),
               delay_prio_bitmask);
  prio = (prio << delay_prio_bitsize) + delay_prio;

  // 5. Round-robin across slices with the same slice and delay priorities.
  float               rbs_per_slot = is_dl ? inst.average_pdsch_rbs_per_slot() : inst.average_pusch_rbs_per_slot();
  const priority_type rr_prio      = rr_bitmask - std::min((unsigned)std::round(rbs_per_slot), rr_bitmask);
  prio                             = (prio << rr_bitsize) + rr_prio;

  // Add one bit to differentiate from skip priority.
  return (prio << 1U) + 1U;
}
