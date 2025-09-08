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

#include "srsran/ran/srs/srs_resource_configuration.h"

namespace srsran {

/// \brief Collects Sounding Reference Signal information.
///
/// Contains the derived parameters that are used for the SRS sequence generation and mapping, as per TS38.211 Sections
/// 6.4.1.4.2 and 6.4.1.4.3.
///
/// The parameters describe the sequence and its allocation for a certain transmission port and OFDM symbol.
struct srs_information {
  /// Sequence length, parameter \f$M_{sc,b}^{SRS}\f$.
  unsigned sequence_length;
  /// Sequence group, parameter \f$u\f$.
  unsigned sequence_group;
  /// Sequence number, parameter \f$v\f$.
  unsigned sequence_number;
  /// Sequence cyclic shift, parameter \f$n_{SRS}^{cs,i}\f$.
  unsigned n_cs;
  /// Sequence maximum cyclic shift, parameter \f$n_{SRS}^{cs,max}\f$.
  unsigned n_cs_max;
  /// First subcarrier used for mapping the sequence within the BWP, parameter \f$k_{0}^{(p_i)}\f$.
  unsigned mapping_initial_subcarrier;
  /// Comb size, parameter \f$K_{TC}\f$.
  unsigned comb_size;

  /// Gets the sequence cyclic shift, parameter \f$\alpha\f$.
  float get_alpha() const { return static_cast<float>(2 * M_PI * n_cs) / static_cast<float>(n_cs_max); }
};

/// \brief Get Sounding Reference Signal information.
///
/// Simplified SRS information. It does not implement sequence, group or frequency hopping.
///
/// \param resource       SRS resource configuration.
/// \param i_antenna_port Transmit antenna 0-based port index.
/// \return The SRS information.
/// \remark An assertion is triggered if one or more resource parameters are invalid or not supported.
srs_information get_srs_information(const srs_resource_configuration& resource, unsigned i_antenna_port);

} // namespace srsran
