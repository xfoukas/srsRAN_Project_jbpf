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

#include "../support/uplink_context_repository.h"
#include "srsran/srslog/logger.h"

namespace srsran {
namespace ofh {

struct uplane_message_decoder_results;

/// \brief Open Fronthaul User-Plane uplink received symbol data flow writer.
///
/// Writes IQ data received in an Open Fronthaul message to the corresponding resource grid.
class uplane_rx_symbol_data_flow_writer
{
public:
  uplane_rx_symbol_data_flow_writer(span<const unsigned>                       ul_eaxc_,
                                    unsigned                                   sector_id_,
                                    srslog::basic_logger&                      logger_,
                                    std::shared_ptr<uplink_context_repository> ul_context_repo_) :
    ul_eaxc(ul_eaxc_.begin(), ul_eaxc_.end()),
    sector_id(sector_id_),
    logger(logger_),
    ul_context_repo(std::move(ul_context_repo_))
  {
    srsran_assert(!ul_eaxc.empty(), "Invalid number of uplink eAxCs");
    srsran_assert(ul_context_repo, "Invalid uplink context repository");
  }

  /// Writes the given decoder results in the corresponding resource grid using the given eAxC.
  void write_to_resource_grid(unsigned eaxc, const uplane_message_decoder_results& results);

private:
  const static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC> ul_eaxc;
  const unsigned                                        sector_id;
  srslog::basic_logger&                                 logger;
  std::shared_ptr<uplink_context_repository>            ul_context_repo;
};

} // namespace ofh
} // namespace srsran
