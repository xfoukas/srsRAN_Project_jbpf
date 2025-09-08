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

#include "srsran/gateways/baseband/baseband_gateway_base.h"
#include "srsran/gateways/baseband/baseband_gateway_transmitter_metadata.h"

namespace srsran {

class baseband_gateway_buffer_reader;

/// Baseband gateway - transmission interface.
class baseband_gateway_transmitter : public baseband_gateway_base
{
public:
  /// \brief Transmits a set of baseband samples at the time instant provided in the metadata.
  /// \param[in] data     Buffer of baseband samples to transmit.
  /// \param[in] metadata Additional parameters for transmission.
  /// \remark The data buffers must have the same number of channels as the stream.
  virtual void transmit(const baseband_gateway_buffer_reader&        data,
                        const baseband_gateway_transmitter_metadata& metadata) = 0;
};

} // namespace srsran
