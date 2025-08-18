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

#include "rx_buffer_codeblock_pool.h"
#include "rx_buffer_impl.h"
#include "srsran/phy/upper/rx_buffer_pool.h"
#include "srsran/phy/upper/unique_rx_buffer.h"
#include "srsran/ran/slot_point.h"
#include "srsran/srslog/logger.h"
#include "srsran/srslog/srslog.h"
#include "srsran/support/srsran_assert.h"
#include <algorithm>
#include <vector>

namespace srsran {

/// Implements a PUSCH rate matcher buffer pool.
class rx_buffer_pool_impl : public rx_buffer_pool_controller, private rx_buffer_pool
{
private:
  /// No expiration time value.
  const slot_point null_expiration = slot_point();

  /// Set to true when the buffer pool stopped.
  std::atomic<bool> stopped = {false};
  /// Codeblock buffer pool.
  rx_buffer_codeblock_pool codeblock_pool;
  /// Actual buffer pool.
  std::vector<rx_buffer_impl> buffers;
  /// \brief List of buffer identifiers.
  ///
  /// Maps buffer identifiers to buffers. Each position indicates the buffer identifier assign to each of the buffers.
  /// Buffers with \c trx_buffer_identifier::invalid() identifier are not reserved.
  std::vector<trx_buffer_identifier> identifiers;
  /// Tracks expiration times.
  std::vector<slot_point> expirations;

  /// Indicates the lifetime of a buffer reservation as a number of slots.
  unsigned expire_timeout_slots;
  /// Logger.
  srslog::basic_logger& logger;

  // See interface for documentation.
  unique_rx_buffer
  reserve(const slot_point& slot, trx_buffer_identifier id, unsigned nof_codeblocks, bool new_data) override;

  // See interface for documentation.
  void run_slot(const slot_point& slot) override;

public:
  /// \brief Creates a generic receiver buffer pool.
  /// \param[in] config Provides the pool required parameters.
  rx_buffer_pool_impl(const rx_buffer_pool_config& config) :
    codeblock_pool(config.nof_codeblocks, config.max_codeblock_size, config.external_soft_bits),
    buffers(config.nof_buffers, rx_buffer_impl(codeblock_pool)),
    identifiers(config.nof_buffers, trx_buffer_identifier::invalid()),
    expirations(config.nof_buffers, null_expiration),
    expire_timeout_slots(config.expire_timeout_slots),
    logger(srslog::fetch_basic_logger("PHY", true))
  {
  }

  // See rx_buffer_pool_controller interface for documentation.
  ~rx_buffer_pool_impl() override
  {
    srsran_assert(std::none_of(buffers.begin(), buffers.end(), [](const auto& buffer) { return buffer.is_locked(); }),
                  "A buffer is still locked.");
  }

  // See rx_buffer_pool_controller interface for documentation.
  rx_buffer_pool& get_pool() override;

  // See rx_buffer_pool_controller interface for documentation.
  void stop() override;
};

} // namespace srsran
