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

#include "srsran/phy/upper/channel_processors/pdcch/pdcch_processor.h"
#include "srsran/support/memory_pool/bounded_object_pool.h"

namespace srsran {

/// \brief Concurrent PDCCH processor pool.
///
/// Wraps a concurrent pool of PDCCH processors. It assigns a PDCCH processor to each thread that processes PDCCH.
class pdcch_processor_pool : public pdcch_processor
{
public:
  /// PDCCH processor pool type.
  using pdcch_processor_pool_type = bounded_unique_object_pool<pdcch_processor>;

  /// Creates a PDCCH processor pool from a list of processors. Ownership is transferred to the pool.
  explicit pdcch_processor_pool(std::shared_ptr<pdcch_processor_pool_type> processors_) :
    logger(srslog::fetch_basic_logger("PHY")), processors(std::move(processors_))
  {
    srsran_assert(processors, "Invalid processor pool.");
  }

  // See interface for documentation.
  void process(resource_grid_writer& grid, const pdu_t& pdu) override
  {
    auto processor = processors->get();
    if (!processor) {
      logger.error(pdu.slot.sfn(), pdu.slot.slot_index(), "Failed to retrieve PDCCH processor.");
      return;
    }
    processor->process(grid, pdu);
  }

private:
  srslog::basic_logger&                      logger;
  std::shared_ptr<pdcch_processor_pool_type> processors;
};

} // namespace srsran
