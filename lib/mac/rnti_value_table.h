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

#include "srsran/ran/rnti.h"
#include "srsran/support/srsran_assert.h"
#include <atomic>
#include <limits>
#include <memory>
#include <vector>

namespace srsran {

/// Table used by the MAC layer to convert from RNTI to a value in a thread-safe manner.
template <typename T, T SentinelValue>
class rnti_value_table
{
  static constexpr rnti_t MIN_CRNTI  = rnti_t::MIN_CRNTI;
  static constexpr rnti_t MAX_CRNTI  = rnti_t::MAX_CRNTI;
  static constexpr size_t RNTI_RANGE = to_value(rnti_t::MAX_CRNTI) + 1 - to_value(rnti_t::MIN_CRNTI);
  using array_type                   = std::array<std::atomic<T>, RNTI_RANGE>;

public:
  rnti_value_table() : rnti_to_ue_index_map(std::make_unique<array_type>())
  {
    for (unsigned i = 0, e = rnti_to_ue_index_map->size(); i != e; ++i) {
      (*rnti_to_ue_index_map)[i].store(SentinelValue, std::memory_order_relaxed);
    }
  }

  rnti_value_table(const rnti_value_table&)            = delete;
  rnti_value_table(rnti_value_table&&)                 = delete;
  rnti_value_table& operator=(const rnti_value_table&) = delete;
  rnti_value_table& operator=(rnti_value_table&&)      = delete;

  /// \brief Associates the given RNTI to a value.
  ///
  /// \param crnti RNTI value.
  /// \param value Value to associante with an RNTI.
  /// \return Returns true if the RNTI does not yet exist, otherwise false.
  bool add_ue(rnti_t crnti, T value)
  {
    srsran_assert(is_crnti(crnti), "Invalid c-rnti={}", crnti);
    srsran_assert(value != SentinelValue, "Invalid rnti_value_table value={}", fmt::underlying(value));

    std::atomic<T>& ue_pos      = get(crnti);
    T               prev_ue_idx = ue_pos.exchange(value, std::memory_order_relaxed);
    if (prev_ue_idx == SentinelValue) {
      nof_ues_.fetch_add(1, std::memory_order_relaxed);
      return true;
    }

    return false;
  }

  /// Removes the given RNTI from the table.
  void rem_ue(rnti_t crnti)
  {
    srsran_assert(is_crnti(crnti), "Invalid c-rnti={}", crnti);

    std::atomic<T>& ue_pos      = get(crnti);
    T               prev_ue_idx = ue_pos.exchange(SentinelValue, std::memory_order_relaxed);
    if (prev_ue_idx != SentinelValue) {
      nof_ues_.fetch_sub(1, std::memory_order_relaxed);
      srsran_assert(nof_ues_.load(std::memory_order_relaxed) <= RNTI_RANGE, "Invalid rnti_table state");
    }
  }

  /// Get an estimate of the current number of UEs present in the table.
  size_t nof_ues() const { return nof_ues_.load(std::memory_order_relaxed); }

  /// Checks whether the passed RNTI is registered as a UE.
  bool has_rnti(rnti_t crnti) const { return this->operator[](crnti) != SentinelValue; }

  T operator[](rnti_t crnti) const
  {
    srsran_sanity_check(is_crnti(crnti), "Invalid c-rnti={}", crnti);

    const std::atomic<T>& ue_pos = get(crnti);
    return ue_pos.load(std::memory_order_relaxed);
  }

private:
  std::atomic<T>& get(rnti_t rnti) { return (*rnti_to_ue_index_map)[to_value(rnti) - to_value(rnti_t::MIN_CRNTI)]; }
  const std::atomic<T>& get(rnti_t rnti) const
  {
    return (*rnti_to_ue_index_map)[to_value(rnti) - to_value(rnti_t::MIN_CRNTI)];
  }

  /// Table of RNTI -> UE index with size 65535.
  std::unique_ptr<array_type> rnti_to_ue_index_map;
  /// Number of tracked UEs in the table.
  std::atomic<size_t> nof_ues_{0};
};

} // namespace srsran
