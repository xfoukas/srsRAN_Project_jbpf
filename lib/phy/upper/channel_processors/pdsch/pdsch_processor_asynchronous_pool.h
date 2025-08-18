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
#include "srsran/adt/span.h"
#include "srsran/phy/upper/channel_processors/pdsch/formatters.h"
#include "srsran/phy/upper/channel_processors/pdsch/pdsch_processor.h"
#include "srsran/srslog/logger.h"
#include "srsran/srslog/srslog.h"
#include <memory>
#include <vector>

namespace srsran {

namespace detail {

/// Free PDSCH processor identifier list type.
using pdsch_processor_free_list =
    concurrent_queue<unsigned, concurrent_queue_policy::lockfree_mpmc, concurrent_queue_wait_policy::sleep>;

/// PDSCH processor wrapper. It appends its identifier into the free list when the processing is finished.
class pdsch_processor_wrapper : private pdsch_processor, private pdsch_processor_notifier
{
public:
  /// Creates a PDSCH processor wrapper from another PDSCH processor.
  explicit pdsch_processor_wrapper(unsigned                         index_,
                                   pdsch_processor_free_list&       free_list_,
                                   std::unique_ptr<pdsch_processor> processor_) :
    index(index_), free_list(free_list_), processor(std::move(processor_))
  {
    srsran_assert(processor, "Invalid PDSCH processor.");
  }

  /// Creates a PDSCH processor wrapper from another PDSCH processor wrapper.
  pdsch_processor_wrapper(pdsch_processor_wrapper&& other) noexcept :
    index(other.index), free_list(other.free_list), notifier(other.notifier), processor(std::move(other.processor))
  {
    other.notifier = nullptr;
  }

  // See pdsch_processor interface for documentation.
  void process(resource_grid_writer&                                                            grid,
               pdsch_processor_notifier&                                                        notifier_,
               static_vector<shared_transport_block, pdsch_processor::MAX_NOF_TRANSPORT_BLOCKS> data,
               const pdsch_processor::pdu_t&                                                    pdu) override
  {
    // Save original notifier.
    notifier = &notifier_;

    // Process.
    processor->process(grid, *this, std::move(data), pdu);
  }

private:
  // See pdsch_processor_notifier for documentation.
  void on_finish_processing() override
  {
    srsran_assert(notifier != nullptr, "Invalid notifier.");

    // Notify the completion of the processing.
    notifier->on_finish_processing();

    // Return the PDSCH processor identifier to the free list.
    free_list.push_blocking(index);
  }

  /// Processor identifier within the pool.
  unsigned index;
  /// List of free processors.
  pdsch_processor_free_list& free_list;
  /// Current PDSCH processor notifier.
  pdsch_processor_notifier* notifier = nullptr;
  /// Wrapped PDSCH processor.
  std::unique_ptr<pdsch_processor> processor;
};
} // namespace detail

/// \brief Asynchronous PDSCH processor pool.
///
/// It contains PDSCH processors that are asynchronously executed. The processing of a PDSCH transmission is dropped if
/// there are no free PDSCH processors available.
class pdsch_processor_asynchronous_pool : public pdsch_processor
{
public:
  explicit pdsch_processor_asynchronous_pool(span<std::unique_ptr<pdsch_processor>> processors_, bool blocking_) :
    logger(srslog::fetch_basic_logger("PHY")), free_list(processors_.size()), blocking(blocking_)
  {
    unsigned index = 0;
    for (std::unique_ptr<pdsch_processor>& processor : processors_) {
      free_list.push_blocking(index);
      processors.emplace_back(index++, free_list, std::move(processor));
    }
  }

  void process(resource_grid_writer&                                           grid,
               pdsch_processor_notifier&                                       notifier,
               static_vector<shared_transport_block, MAX_NOF_TRANSPORT_BLOCKS> data,
               const pdu_t&                                                    pdu) override
  {
    // Try to get a worker.
    unsigned index;
    bool     popped = false;
    do {
      popped = free_list.try_pop(index);
    } while (blocking && !popped);

    // If no worker is available.
    if (!popped) {
      logger.warning(
          pdu.slot.sfn(), pdu.slot.slot_index(), "Insufficient number of PDSCH processors. Dropping PDSCH {:s}.", pdu);
      notifier.on_finish_processing();
      return;
    }

    // Process PDSCH.
    processors[index].process(grid, notifier, std::move(data), pdu);
  }

private:
  srslog::basic_logger&                        logger;
  std::vector<detail::pdsch_processor_wrapper> processors;
  detail::pdsch_processor_free_list            free_list;
  bool                                         blocking;
};

} // namespace srsran
