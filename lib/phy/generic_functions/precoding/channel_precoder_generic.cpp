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

#include "channel_precoder_generic.h"

using namespace srsran;

void channel_precoder_generic::apply_precoding_port(span<cbf16_t>             port_re,
                                                    const re_buffer_reader<>& input_re,
                                                    span<const cf_t>          port_weights) const
{
  unsigned nof_re     = input_re.get_nof_re();
  unsigned nof_layers = input_re.get_nof_slices();

  std::array<span<const cf_t>, precoding_constants::MAX_NOF_LAYERS> layer_re_view_list;
  for (unsigned i_layer = 0; i_layer != nof_layers; ++i_layer) {
    layer_re_view_list[i_layer] = input_re.get_slice(i_layer);
  }

  for (unsigned i_re = 0; i_re != nof_re; ++i_re) {
    // Set the port RE to the contribution of the first layer.
    cf_t sum = layer_re_view_list[0][i_re] * port_weights[0];

    for (unsigned i_layer = 1; i_layer != nof_layers; ++i_layer) {
      // Accumulate the contributions of all other layers.
      sum += layer_re_view_list[i_layer][i_re] * port_weights[i_layer];
    }
    port_re[i_re] = sum;
  }
}

void channel_precoder_generic::apply_layer_map_and_precoding(re_buffer_writer<cbf16_t>&     output,
                                                             span<const ci8_t>              input,
                                                             const precoding_weight_matrix& precoding) const
{
  unsigned nof_re     = output.get_nof_re();
  unsigned nof_layers = precoding.get_nof_layers();
  unsigned nof_ports  = precoding.get_nof_ports();

  for (unsigned i_re = 0; i_re != nof_re; ++i_re) {
    for (unsigned i_port = 0; i_port != nof_ports; ++i_port) {
      span<const cf_t> port_weights = precoding.get_port_coefficients(i_port);
      span<cbf16_t>    port_re      = output.get_slice(i_port);

      cf_t sum = to_cf(input[nof_layers * i_re]) * port_weights[0];
      for (unsigned i_layer = 1; i_layer != nof_layers; ++i_layer) {
        sum += to_cf(input[nof_layers * i_re + i_layer]) * port_weights[i_layer];
      }
      port_re[i_re] = sum;
    }
  }
}
