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

#include <optional>

namespace srsran {

/// PUSCH demodulator interface to notify the demodulation statistics of a codeword.
class pusch_demodulator_notifier
{
public:
  /// Collects PUSCH demodulation statistics.
  struct demodulation_stats {
    /// Estimated Signal-to-Interference-plus-Noise Ratio (SINR) at the output of the equalizer.
    std::optional<float> sinr_dB;
    /// Measured EVM.
    std::optional<float> evm;
  };

  /// Default destructor.
  virtual ~pusch_demodulator_notifier() = default;

  /// \brief Notifies intermediate PUSCH demodulator statistics.
  /// \param i_symbol OFDM symbol index within the slot.
  /// \param stats    OFDM symbol statistics.
  virtual void on_provisional_stats(unsigned i_symbol, const demodulation_stats& stats) = 0;

  /// Notifies the end of PUSCH processing and the final demodulator statistics.
  virtual void on_end_stats(const demodulation_stats& stats) = 0;
};

} // namespace srsran
