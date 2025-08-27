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

#include "ngap_ue_logger.h"
#include "srsran/ngap/ngap.h"
#include "srsran/ngap/ngap_types.h"
#include "srsran/support/timers.h"
#include <unordered_map>

namespace srsran {
namespace srs_cu_cp {

struct ngap_ue_ids {
  ue_index_t        ue_index  = ue_index_t::invalid;
  const ran_ue_id_t ran_ue_id = ran_ue_id_t::invalid;
  amf_ue_id_t       amf_ue_id = amf_ue_id_t::invalid;
};

struct ngap_ue_context {
  ngap_ue_ids             ue_ids;
  ngap_cu_cp_ue_notifier* ue = nullptr;
  guami_t                 serving_guami;
  uint64_t                aggregate_maximum_bit_rate_dl = 0;
  uint64_t                aggregate_maximum_bit_rate_ul = 0;
  unique_timer            request_pdu_session_timer     = {};
  bool                    release_requested             = false;
  bool                    release_scheduled             = false;
  byte_buffer    last_pdu_session_resource_modify_request; // To check if a received modify request is a duplicate
  ngap_ue_logger logger;

  ngap_ue_context(ue_index_t              ue_index_,
                  ran_ue_id_t             ran_ue_id_,
                  ngap_cu_cp_ue_notifier& ue_notifier_,
                  timer_manager&          timers_,
                  task_executor&          task_exec_) :
    ue_ids({ue_index_, ran_ue_id_}), ue(&ue_notifier_), logger("NGAP", {ue_index_, ran_ue_id_})
  {
    request_pdu_session_timer = timers_.create_unique_timer(task_exec_);
  }

  [[nodiscard]] ngap_cu_cp_ue_notifier* get_cu_cp_ue() const { return ue; }
};

class ngap_ue_context_list
{
public:
  ngap_ue_context_list(srslog::basic_logger& logger_) : logger(logger_) {}

  /// \brief Checks whether a UE with the given RAN UE ID exists.
  /// \param[in] ran_ue_id The RAN UE ID used to find the UE.
  /// \return True when a UE for the given RAN UE ID exists, false otherwise.
  bool contains(ran_ue_id_t ran_ue_id) const { return ues.find(ran_ue_id) != ues.end(); }

  /// \brief Checks whether a UE with the given UE index exists.
  /// \param[in] ue_index The UE index used to find the UE.
  /// \return True when a UE for the given UE index exists, false otherwise.
  bool contains(ue_index_t ue_index) const
  {
    if (ue_index_to_ran_ue_id.find(ue_index) == ue_index_to_ran_ue_id.end()) {
      return false;
    }
    if (ues.find(ue_index_to_ran_ue_id.at(ue_index)) == ues.end()) {
      return false;
    }
    return true;
  }

  /// \brief Checks whether a UE with the given AMF UE ID exists.
  /// \param[in] amf_ue_id The AMF UE ID used to find the UE.
  /// \return True when a UE for the given AMF UE ID exists, false otherwise.
  bool contains(amf_ue_id_t amf_ue_id) const
  {
    if (amf_ue_id_to_ran_ue_id.find(amf_ue_id) == amf_ue_id_to_ran_ue_id.end()) {
      return false;
    }
    if (ues.find(amf_ue_id_to_ran_ue_id.at(amf_ue_id)) == ues.end()) {
      return false;
    }
    return true;
  }

  ngap_ue_context& operator[](ran_ue_id_t ran_ue_id)
  {
    srsran_assert(ues.find(ran_ue_id) != ues.end(), "ran_ue={}: NGAP UE context not found", fmt::underlying(ran_ue_id));
    return ues.at(ran_ue_id);
  }

  ngap_ue_context& operator[](ue_index_t ue_index)
  {
    srsran_assert(
        ue_index_to_ran_ue_id.find(ue_index) != ue_index_to_ran_ue_id.end(), "ue={}: RAN-UE-ID not found", ue_index);
    srsran_assert(ues.find(ue_index_to_ran_ue_id.at(ue_index)) != ues.end(),
                  "ran_ue={}: NGAP UE context not found",
                  fmt::underlying(ue_index_to_ran_ue_id.at(ue_index)));
    return ues.at(ue_index_to_ran_ue_id.at(ue_index));
  }

  ngap_ue_context& operator[](amf_ue_id_t amf_ue_id)
  {
    srsran_assert(amf_ue_id_to_ran_ue_id.find(amf_ue_id) != amf_ue_id_to_ran_ue_id.end(),
                  "amf_ue={}: RAN-UE-ID not found",
                  fmt::underlying(amf_ue_id));
    srsran_assert(ues.find(amf_ue_id_to_ran_ue_id.at(amf_ue_id)) != ues.end(),
                  "ran_ue={}: NGAP UE context not found",
                  fmt::underlying(amf_ue_id_to_ran_ue_id.at(amf_ue_id)));
    return ues.at(amf_ue_id_to_ran_ue_id.at(amf_ue_id));
  }

  ngap_ue_context* find(ran_ue_id_t ran_ue_id)
  {
    auto it = ues.find(ran_ue_id);
    if (it == ues.end()) {
      return nullptr;
    }
    return &it->second;
  }
  const ngap_ue_context* find(ran_ue_id_t ran_ue_id) const
  {
    auto it = ues.find(ran_ue_id);
    if (it == ues.end()) {
      return nullptr;
    }
    return &it->second;
  }

  ngap_ue_context& add_ue(ue_index_t              ue_index,
                          ran_ue_id_t             ran_ue_id,
                          ngap_cu_cp_ue_notifier& ue_notifier,
                          timer_manager&          timers,
                          task_executor&          task_exec)
  {
    srsran_assert(ue_index != ue_index_t::invalid, "Invalid ue_index={}", fmt::underlying(ue_index));
    srsran_assert(ran_ue_id != ran_ue_id_t::invalid, "Invalid ran_ue={}", fmt::underlying(ran_ue_id));

    logger.debug("ue={} ran_ue={}: NGAP UE context created", fmt::underlying(ue_index), fmt::underlying(ran_ue_id));
    ues.emplace(std::piecewise_construct,
                std::forward_as_tuple(ran_ue_id),
                std::forward_as_tuple(ue_index, ran_ue_id, ue_notifier, timers, task_exec));
    ue_index_to_ran_ue_id.emplace(ue_index, ran_ue_id);
    return ues.at(ran_ue_id);
  }

  void update_amf_ue_id(ran_ue_id_t ran_ue_id, amf_ue_id_t amf_ue_id)
  {
    srsran_assert(amf_ue_id != amf_ue_id_t::invalid, "Invalid amf_ue={}", fmt::underlying(amf_ue_id));
    srsran_assert(ran_ue_id != ran_ue_id_t::invalid, "Invalid ran_ue={}", fmt::underlying(ran_ue_id));
    srsran_assert(ues.find(ran_ue_id) != ues.end(), "ran_ue={}: NGAP UE context not found", fmt::underlying(ran_ue_id));

    auto& ue = ues.at(ran_ue_id);

    if (ue.ue_ids.amf_ue_id == amf_ue_id) {
      // If the AMF-UE-ID is already set, we don't want to change it.
      return;
    } else if (ue.ue_ids.amf_ue_id == amf_ue_id_t::invalid) {
      // If it was not set before, we add it.
      ue.logger.log_debug("Setting AMF-UE-NGAP-ID={}", fmt::underlying(amf_ue_id));
      ue.ue_ids.amf_ue_id = amf_ue_id;
      amf_ue_id_to_ran_ue_id.emplace(amf_ue_id, ran_ue_id);
    } else if (ue.ue_ids.amf_ue_id != amf_ue_id) {
      // If it was set before, we update it.
      amf_ue_id_t old_amf_ue_id = ue.ue_ids.amf_ue_id;
      ue.logger.log_info("Updating AMF-UE-NGAP-ID={}", fmt::underlying(amf_ue_id));
      ue.ue_ids.amf_ue_id = amf_ue_id;
      amf_ue_id_to_ran_ue_id.emplace(amf_ue_id, ran_ue_id);
      amf_ue_id_to_ran_ue_id.erase(old_amf_ue_id);
    }

    ue.logger.set_prefix({ue.ue_ids.ue_index, ran_ue_id, amf_ue_id});
  }

  void update_ue_index(ue_index_t new_ue_index, ue_index_t old_ue_index, ngap_cu_cp_ue_notifier& new_ue_notifier)
  {
    srsran_assert(new_ue_index != ue_index_t::invalid, "Invalid new_ue_index={}", new_ue_index);
    srsran_assert(old_ue_index != ue_index_t::invalid, "Invalid old_ue_index={}", old_ue_index);
    srsran_assert(ue_index_to_ran_ue_id.find(old_ue_index) != ue_index_to_ran_ue_id.end(),
                  "ue={}: RAN-UE-ID not found",
                  old_ue_index);

    ran_ue_id_t ran_ue_id = ue_index_to_ran_ue_id.at(old_ue_index);

    srsran_assert(ues.find(ran_ue_id) != ues.end(), "ran_ue={}: NGAP UE context not found", fmt::underlying(ran_ue_id));

    // Update UE context.
    ues.at(ran_ue_id).ue_ids.ue_index = new_ue_index;
    ues.at(ran_ue_id).ue              = &new_ue_notifier;

    // Update lookups.
    ue_index_to_ran_ue_id.emplace(new_ue_index, ran_ue_id);
    ue_index_to_ran_ue_id.erase(old_ue_index);

    ues.at(ran_ue_id).logger.set_prefix(
        {ues.at(ran_ue_id).ue_ids.ue_index, ran_ue_id, ues.at(ran_ue_id).ue_ids.amf_ue_id});

    ues.at(ran_ue_id).logger.log_debug("Updated UE index from ue_index={}", old_ue_index);
  }

  void remove_ue_context(ue_index_t ue_index)
  {
    srsran_assert(ue_index != ue_index_t::invalid, "Invalid ue_index={}", ue_index);

    if (ue_index_to_ran_ue_id.find(ue_index) == ue_index_to_ran_ue_id.end()) {
      logger.warning("ue={}: RAN-UE-ID not found", ue_index);
      return;
    }

    // Remove UE from lookup.
    ran_ue_id_t ran_ue_id = ue_index_to_ran_ue_id.at(ue_index);
    ue_index_to_ran_ue_id.erase(ue_index);

    if (ues.find(ran_ue_id) == ues.end()) {
      logger.warning("ran_ue={}: NGAP UE context not found", fmt::underlying(ran_ue_id));
      return;
    }

    ues.at(ran_ue_id).logger.log_debug("Removing NGAP UE context");

    if (ues.at(ran_ue_id).ue_ids.amf_ue_id != amf_ue_id_t::invalid) {
      amf_ue_id_to_ran_ue_id.erase(ues.at(ran_ue_id).ue_ids.amf_ue_id);
    }
    ues.erase(ran_ue_id);
  }

  size_t size() const { return ues.size(); }

  /// \brief Get the next available RAN-UE-ID.
  ran_ue_id_t allocate_ran_ue_id()
  {
    // Return invalid when no RAN-UE-ID is available.
    if (ue_index_to_ran_ue_id.size() == MAX_NOF_RAN_UES) {
      return ran_ue_id_t::invalid;
    }

    // Check if the next_ran_ue_id is available.
    if (ues.find(next_ran_ue_id) == ues.end()) {
      ran_ue_id_t ret = next_ran_ue_id;
      // Increase the next CU UE F1AP ID.
      increase_next_ran_ue_id();
      return ret;
    }

    // Iterate over all ids starting with the next_ran_ue_id to find the available id.
    while (true) {
      // Iterate over ue_index_to_ran_ue_id.
      auto it = std::find_if(ue_index_to_ran_ue_id.begin(), ue_index_to_ran_ue_id.end(), [this](auto& u) {
        return u.second == next_ran_ue_id;
      });

      // Return the ID if it is not already used.
      if (it == ue_index_to_ran_ue_id.end()) {
        ran_ue_id_t ret = next_ran_ue_id;
        // Increase the next CU-UE-F1AP-ID.
        increase_next_ran_ue_id();
        return ret;
      }

      // Increase the next CU-UE-F1AP-ID and try again.
      increase_next_ran_ue_id();
    }

    return ran_ue_id_t::invalid;
  }

protected:
  ran_ue_id_t next_ran_ue_id = ran_ue_id_t::min;

private:
  srslog::basic_logger& logger;

  inline void increase_next_ran_ue_id()
  {
    if (next_ran_ue_id == ran_ue_id_t::max) {
      // Reset RAN-UE-ID counter.
      next_ran_ue_id = ran_ue_id_t::min;
    } else {
      // Increase RAN-UE-ID counter.
      next_ran_ue_id = uint_to_ran_ue_id(ran_ue_id_to_uint(next_ran_ue_id) + 1);
    }
  }

  // Note: Given that UEs will self-remove from the map, we don't want to destructor to clear the lookups beforehand.
  std::unordered_map<ue_index_t, ran_ue_id_t>      ue_index_to_ran_ue_id;  // indexed by ue_index
  std::unordered_map<amf_ue_id_t, ran_ue_id_t>     amf_ue_id_to_ran_ue_id; // indexed by amf_ue_id_t
  std::unordered_map<ran_ue_id_t, ngap_ue_context> ues;                    // indexed by ran_ue_id_t
};

} // namespace srs_cu_cp
} // namespace srsran
