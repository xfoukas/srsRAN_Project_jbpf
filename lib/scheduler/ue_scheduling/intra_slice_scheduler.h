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

#include "../logging/scheduler_metrics_handler.h"
#include "../policy/scheduler_policy.h"
#include "../slicing/ran_slice_candidate.h"
#include "ue_cell_grid_allocator.h"

namespace srsran {

class pdcch_resource_allocator;
class scheduler_policy;
class ue_repository;

class intra_slice_scheduler
{
public:
  intra_slice_scheduler(const scheduler_ue_expert_config& expert_cfg_,
                        ue_repository&                    ues,
                        pdcch_resource_allocator&         pdcch_alloc,
                        uci_allocator&                    uci_alloc,
                        cell_resource_allocator&          cell_alloc,
                        cell_metrics_handler&             cell_metrics_,
                        cell_harq_manager&                cell_harqs_,
                        srslog::basic_logger&             logger_);

  /// Reset context in preparation for new slot.
  void slot_indication(slot_point sl_tx);

  /// Called once all the UE grants have been allocated for a slot and cell.
  void post_process_results();

  /// Schedule DL grants for a given slice candidate.
  void dl_sched(dl_ran_slice_candidate slice, scheduler_policy& dl_policy);

  /// Schedule UL grants for a given slice candidate.
  void ul_sched(ul_ran_slice_candidate slice, scheduler_policy& ul_policy);

private:
  /// Context for a given slice scheduling.
  struct slice_sched_context {
    // UE index offset of the next UE group to be scheduled in DL.
    du_ue_index_t dl_next_rr_group_offset = to_du_ue_index(0);
    // How many slots have elapsed using the same dl_next_rr_group_offset.
    unsigned dl_rr_count = 0;
    // UE index offset of the next UE group to be scheduled in UL.
    du_ue_index_t ul_next_rr_group_offset = to_du_ue_index(0);
    // How many slots have elapsed using the same ul_next_rr_group_offset.
    unsigned ul_rr_count = 0;
  };

  /// Determines whether a UE can be DL scheduled in a given slot.
  bool can_allocate_pdsch(const slice_ue& u, const ue_cell& ue_cc) const;

  /// Determines whether a UE can be UL scheduled in a given slot.
  bool can_allocate_pusch(const slice_ue& u, const ue_cell& ue_cc) const;

  std::optional<ue_newtx_candidate> create_newtx_dl_candidate(const slice_ue& u) const;

  std::optional<ue_newtx_candidate> create_newtx_ul_candidate(const slice_ue& u) const;

  void prepare_newtx_dl_candidates(const dl_ran_slice_candidate& slice, scheduler_policy& dl_policy);

  void prepare_newtx_ul_candidates(const ul_ran_slice_candidate& slice, scheduler_policy& ul_policy);

  unsigned schedule_dl_retx_candidates(dl_ran_slice_candidate& slice, unsigned max_ue_grants_to_alloc);

  unsigned schedule_ul_retx_candidates(ul_ran_slice_candidate& slice, unsigned max_ue_grants_to_alloc);

  unsigned schedule_dl_newtx_candidates(dl_ran_slice_candidate& slice,
                                        scheduler_policy&       dl_policy,
                                        unsigned                max_ue_grants_to_alloc);

  unsigned schedule_ul_newtx_candidates(ul_ran_slice_candidate& slice,
                                        scheduler_policy&       ul_policy,
                                        unsigned                max_ue_grants_to_alloc);

  unsigned max_pdschs_to_alloc(const dl_ran_slice_candidate& slice);

  unsigned max_puschs_to_alloc(const ul_ran_slice_candidate& slice);

  // Called when bitmap of used VRBs needs to be recalculated.
  void update_used_dl_vrbs(const dl_ran_slice_candidate& slice);
  void update_used_ul_vrbs(const ul_ran_slice_candidate& slice);

  const scheduler_ue_expert_config& expert_cfg;
  const cell_resource_allocator&    cell_alloc;
  cell_metrics_handler&             cell_metrics;
  cell_harq_manager&                cell_harqs;
  uci_allocator&                    uci_alloc;
  srslog::basic_logger&             logger;

  // Derived parameters.
  const unsigned expected_pdschs_per_slot;

  /// Handler of grid allocations.
  ue_cell_grid_allocator ue_alloc;

  // Slot at which PDCCH is scheduled.
  slot_point pdcch_slot;

  // Number of allocation attempts for DL in the given slot.
  unsigned dl_attempts_count = 0;
  unsigned ul_attempts_count = 0;

  // Information related with the scheduling of each slice that needs to be stored and retrieved across different slots.
  slotted_id_vector<ran_slice_id_t, slice_sched_context> slice_ctxt_list;

  // UE candidates for on-going scheduling.
  std::vector<ue_newtx_candidate> newtx_candidates;

  slot_point pdsch_slot;
  slot_point pusch_slot;
  vrb_bitmap used_dl_vrbs;
  bool       enable_pdsch_interleaving;
  vrb_bitmap used_ul_vrbs;

  // Grants being built for the current slice.
  std::vector<ue_cell_grid_allocator::dl_newtx_grant_builder> pending_dl_newtxs;
  std::vector<ue_cell_grid_allocator::ul_newtx_grant_builder> pending_ul_newtxs;
};

} // namespace srsran
