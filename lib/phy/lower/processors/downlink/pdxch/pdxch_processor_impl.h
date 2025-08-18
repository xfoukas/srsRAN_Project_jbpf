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

#include "../../resource_grid_request_pool.h"
#include "srsran/gateways/baseband/buffer/baseband_gateway_buffer_dynamic.h"
#include "srsran/phy/lower/modulation/ofdm_modulator.h"
#include "srsran/phy/lower/processors/downlink/pdxch/pdxch_processor.h"
#include "srsran/phy/lower/processors/downlink/pdxch/pdxch_processor_baseband.h"
#include "srsran/phy/lower/processors/downlink/pdxch/pdxch_processor_notifier.h"
#include "srsran/phy/lower/processors/downlink/pdxch/pdxch_processor_request_handler.h"
#include "srsran/phy/lower/processors/lower_phy_center_freq_controller.h"
#include "srsran/phy/support/resource_grid_context.h"

namespace srsran {

/// Implements PDxCH baseband processor.
class pdxch_processor_impl : public pdxch_processor,
                             private pdxch_processor_baseband,
                             private pdxch_processor_request_handler,
                             private lower_phy_center_freq_controller
{
public:
  struct configuration {
    cyclic_prefix cp;
    unsigned      nof_tx_ports;
    unsigned      request_queue_size;
  };

  pdxch_processor_impl(std::unique_ptr<ofdm_symbol_modulator> modulator_, const configuration& config) :
    nof_symbols_per_slot(get_nsymb_per_slot(config.cp)),
    nof_tx_ports(config.nof_tx_ports),
    modulator(std::move(modulator_))
  {
    srsran_assert(modulator, "Invalid modulator.");
  }

  // See pdxch_processor interface for documentation.
  void connect(pdxch_processor_notifier& notifier) override;

  // See pdxch_processor interface for documentation.
  void stop() override { stopped = true; }

  // See pdxch_processor interface for documentation.
  pdxch_processor_request_handler& get_request_handler() override;

  // See pdxch_processor interface for documentation.
  pdxch_processor_baseband& get_baseband() override;

  // See pdxch_processor interface for documentation.
  lower_phy_center_freq_controller& get_center_freq_control() override;

private:
  // See pdxch_processor_baseband interface for documentation.
  bool process_symbol(baseband_gateway_buffer_writer& samples, const symbol_context& context) override;

  // See pdxch_processor_request_handler interface for documentation.
  void handle_request(const shared_resource_grid& grid, const resource_grid_context& context) override;

  // See lower_phy_center_freq_controller interface for documentation.
  bool set_carrier_center_frequency(double carrier_center_frequency_Hz) override;

  std::atomic<bool>                      stopped = false;
  unsigned                               nof_symbols_per_slot;
  unsigned                               nof_tx_ports;
  pdxch_processor_notifier*              notifier = nullptr;
  std::unique_ptr<ofdm_symbol_modulator> modulator;
  slot_point                             current_slot;
  shared_resource_grid                   current_grid;
  resource_grid_request_pool             requests;
};

} // namespace srsran
