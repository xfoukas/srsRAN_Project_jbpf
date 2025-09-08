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

#include "srsran/ofh/ethernet/ethernet_frame_builder.h"

namespace srsran {
namespace ether {
namespace testing {

/// Spy VLAN frame builder implementation.
class vlan_frame_builder_spy : public frame_builder
{
  bool              build_vlan_frame_method_called = false;
  vlan_frame_params params;

public:
  explicit vlan_frame_builder_spy(const vlan_frame_params& eth_params) : params(eth_params) {}

  // See interface for documentation.
  units::bytes get_header_size() const override { return units::bytes(18); }

  // See interface for documentation.
  void build_frame(span<uint8_t> buffer) override { build_vlan_frame_method_called = true; }

  /// Returns true if the build VLAN frame has been called, otherwise false.
  bool has_build_vlan_frame_method_been_called() const { return build_vlan_frame_method_called; }

  /// Returns the ethernet parameters processed by this builder.
  const vlan_frame_params& get_vlan_frame_params() const { return params; }
};

} // namespace testing
} // namespace ether
} // namespace srsran
