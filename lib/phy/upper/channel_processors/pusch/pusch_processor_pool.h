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

#include "srsran/adt/mpmc_queue.h"
#include "srsran/phy/upper/channel_processors/pusch/formatters.h"
#include "srsran/phy/upper/channel_processors/pusch/pusch_processor.h"
#include "srsran/srslog/logger.h"
#include "srsran/support/memory_pool/bounded_object_pool.h"

namespace srsran {

namespace detail {

/// Free PUSCH processor identifier list type.
using pusch_processor_free_list =
    concurrent_queue<unsigned, concurrent_queue_policy::lockfree_mpmc, concurrent_queue_wait_policy::sleep>;

/// PUSCH processor wrapper. It appends its identifier into the free list when the processing is finished.
class pusch_processor_wrapper : public pusch_processor, private pusch_processor_result_notifier
{
public:
  /// Creates a pusch processor wrapper from another pusch processor.
  explicit pusch_processor_wrapper(unsigned                         index_,
                                   pusch_processor_free_list&       free_list_,
                                   std::unique_ptr<pusch_processor> processor_) :
    index(index_), free_list(free_list_), notifier(nullptr), processor(std::move(processor_))
  {
    srsran_assert(processor, "Invalid pusch processor.");
  }

  /// Creates a PUSCH processor wrapper from another PUSCH processor wrapper.
  pusch_processor_wrapper(pusch_processor_wrapper&& other) :
    index(other.index), free_list(other.free_list), notifier(other.notifier), processor(std::move(other.processor))
  {
    other.notifier = nullptr;
  }

  // See pusch_processor interface for documentation.
  void process(span<uint8_t>                    data,
               unique_rx_buffer                 rm_buffer,
               pusch_processor_result_notifier& notifier_,
               const resource_grid_reader&      grid,
               const pdu_t&                     pdu) override
  {
    // Save original notifier.
    notifier = &notifier_;

    // Process.
    processor->process(data, std::move(rm_buffer), *this, grid, pdu);
  }

private:
  // See pusch_processor_result_notifier for documentation.
  void on_uci(const pusch_processor_result_control& uci) override
  {
    srsran_assert(notifier != nullptr, "Invalid notifier.");
    notifier->on_uci(uci);
  }

  // See pusch_processor_result_notifier for documentation.
  void on_sch(const pusch_processor_result_data& sch) override
  {
    srsran_assert(notifier != nullptr, "Invalid notifier.");

    // Notify the completion of the processing.
    notifier->on_sch(sch);

    // Return the pusch processor identifier to the free list.
    free_list.push_blocking(index);
  }

  /// Processor identifier within the pool.
  unsigned index;
  /// List of free processors.
  pusch_processor_free_list& free_list;
  /// Current pusch processor notifier.
  pusch_processor_result_notifier* notifier = nullptr;
  /// Wrapped pusch processor.
  std::unique_ptr<pusch_processor> processor;
};
} // namespace detail

/// \brief PUSCH processor pool
///
/// It contains PUSCH processors that are asynchronously executed. The processing of a PUSCH transmission is dropped if
/// there are no free PUSCH processors available.
///
class pusch_processor_pool : public pusch_processor
{
public:
  /// Creates a PUSCH processor pool from a list of processors. Ownership is transferred to the pool.
  pusch_processor_pool(std::vector<std::unique_ptr<pusch_processor>> processors_,
                       std::vector<std::unique_ptr<pusch_processor>> uci_processors_,
                       bool                                          blocking_) :
    logger(srslog::fetch_basic_logger("PHY")),
    uci_processors(uci_processors_),
    free_list(processors_.size()),
    blocking(blocking_)
  {
    unsigned index = 0;
    for (std::unique_ptr<pusch_processor>& processor : processors_) {
      free_list.push_blocking(index);
      processors.emplace_back(detail::pusch_processor_wrapper(index++, free_list, std::move(processor)));
    }
  }

  // See interface for documentation.
  void process(span<uint8_t>                    data,
               unique_rx_buffer                 rm_buffer,
               pusch_processor_result_notifier& notifier,
               const resource_grid_reader&      grid,
               const pdu_t&                     pdu) override
  {
    // Try to get an available worker.
    unsigned index;
    bool     popped = false;
    do {
      popped = free_list.try_pop(index);
    } while (blocking && !popped);

    // A PUSCH processor is selected.
    if (popped) {
      processors[index].process(data, std::move(rm_buffer), notifier, grid, pdu);
      return;
    }

    // No PUSCH processor is available and no UCI is present. Drop PUSCH reception.
    if ((pdu.uci.nof_harq_ack == 0) && (pdu.uci.nof_csi_part1 == 0)) {
      logger.warning(
          pdu.slot.sfn(), pdu.slot.slot_index(), "PUSCH processing queue is full. Dropping PUSCH {:s}.", pdu);

      // Report data-related discarded result if shared channel data is present.
      if (pdu.codeword.has_value()) {
        notifier.on_sch({.data = {.tb_crc_ok = false, .nof_codeblocks_total = 0, .ldpc_decoder_stats = {}}, .csi = {}});
      }

      return;
    }

    // Process UCI synchronously if UCI is present.
    auto uci_processor = uci_processors.get();
    if (uci_processor) {
      logger.warning(
          pdu.slot.sfn(), pdu.slot.slot_index(), "PUSCH processing queue is full. Processing UCI {:s}.", pdu);
      uci_processor->process(data, std::move(rm_buffer), notifier, grid, pdu);
    } else {
      logger.warning(
          pdu.slot.sfn(), pdu.slot.slot_index(), "PUSCH processing queue is full. Failed processing UCI {:s}.", pdu);

      // Report data-related discarded result if shared channel data is present.
      if (pdu.codeword.has_value()) {
        notifier.on_sch({.data = {.tb_crc_ok = false, .nof_codeblocks_total = 0, .ldpc_decoder_stats = {}}, .csi = {}});
      }

      pusch_processor_result_data discarded_results;
      notifier.on_uci({.harq_ack  = {.payload = uci_payload_type(pdu.uci.nof_harq_ack), .status = uci_status::invalid},
                       .csi_part1 = {},
                       .csi_part2 = {},
                       .csi       = {}});
    }
  }

private:
  /// Physical layer logger.
  srslog::basic_logger& logger;
  /// Actual PUSCH processor pool.
  std::vector<detail::pusch_processor_wrapper> processors;
  /// Synchronous PUSCH processor. It only processes UCI.
  bounded_unique_object_pool<pusch_processor> uci_processors;
  /// List containing the indices of free PUSCH processors.
  detail::pusch_processor_free_list free_list;
  /// Set to true for blocking upon the the selection of a PUSCH processor.
  bool blocking;
};

} // namespace srsran
