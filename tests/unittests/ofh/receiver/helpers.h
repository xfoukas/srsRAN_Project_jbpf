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

#include "../../phy/support/resource_grid_test_doubles.h"
#include "srsran/phy/support/prach_buffer.h"
#include "srsran/ran/resource_block.h"

namespace srsran {
namespace ofh {
namespace testing {

/// PRACH buffer dummy implementation.
class prach_buffer_dummy : public prach_buffer
{
  unsigned                    nof_symbols;
  static_vector<cbf16_t, 839> buffer;
  mutable bool                symbol_out_of_bounds;

public:
  prach_buffer_dummy(unsigned nof_symbols_, bool long_format = true) :
    nof_symbols((nof_symbols_ == 0) ? 1 : nof_symbols_), buffer(long_format ? 839 : 139), symbol_out_of_bounds(false)
  {
  }

  unsigned get_max_nof_ports() const override { return 1; }

  unsigned get_max_nof_td_occasions() const override { return 1; }

  unsigned get_max_nof_fd_occasions() const override { return 1; }

  unsigned get_max_nof_symbols() const override { return nof_symbols; }

  unsigned get_sequence_length() const override { return buffer.size(); }

  span<cbf16_t> get_symbol(unsigned i_port, unsigned i_td_occasion, unsigned i_fd_occasion, unsigned i_symbol) override
  {
    return buffer;
  }

  span<const cbf16_t>
  get_symbol(unsigned i_port, unsigned i_td_occasion, unsigned i_fd_occasion, unsigned i_symbol) const override
  {
    if (i_symbol >= nof_symbols) {
      symbol_out_of_bounds = true;
    }
    return buffer;
  }

  bool correct_symbols_requested() const { return !symbol_out_of_bounds; }
};

/// Spy implementation of the resource grid writer that returns if the functions were called.
class resource_grid_writer_bool_spy : public resource_grid_writer
{
  static inline const cbf16_t init_value{-1.0, +1.0};

  bool                 grid_written     = false;
  unsigned             nof_prbs_written = 0;
  std::vector<cbf16_t> grid_data;

public:
  resource_grid_writer_bool_spy() = default;
  explicit resource_grid_writer_bool_spy(unsigned nof_prbs) : grid_data(nof_prbs * NOF_SUBCARRIERS_PER_RB)
  {
    for (auto& sample : grid_data) {
      sample = init_value;
    }
  }

  unsigned get_nof_ports() const override { return 1; }
  unsigned get_nof_subc() const override { return grid_data.size(); }
  unsigned get_nof_symbols() const override { return MAX_NSYMB_PER_SLOT; }

  span<const cf_t> put(unsigned                                               port,
                       unsigned                                               l,
                       unsigned                                               k_init,
                       const bounded_bitset<NOF_SUBCARRIERS_PER_RB * MAX_RB>& mask,
                       span<const cf_t>                                       symbols) override
  {
    grid_written = true;
    nof_prbs_written += symbols.size() / NOF_SUBCARRIERS_PER_RB;
    return {};
  }

  span<const cbf16_t> put(unsigned                                               port,
                          unsigned                                               l,
                          unsigned                                               k_init,
                          const bounded_bitset<NOF_SUBCARRIERS_PER_RB * MAX_RB>& mask,
                          span<const cbf16_t>                                    symbols) override
  {
    grid_written = true;
    nof_prbs_written += symbols.size() / NOF_SUBCARRIERS_PER_RB;
    return {};
  }

  void put(unsigned port, unsigned l, unsigned k_init, span<const cf_t> symbols) override
  {
    grid_written = true;
    nof_prbs_written += symbols.size() / NOF_SUBCARRIERS_PER_RB;
  }

  void put(unsigned port, unsigned l, unsigned k_init, unsigned stride, span<const cbf16_t> symbols) override
  {
    grid_written = true;
    nof_prbs_written += divide_ceil(symbols.size() * stride, NOF_SUBCARRIERS_PER_RB);
  }

  span<cbf16_t> get_view(unsigned port, unsigned l) override
  {
    grid_written = true;
    return grid_data;
  }

  /// Returns true if the gris has been written, otherise false.
  bool has_grid_been_written() const { return grid_written; }

  /// Returns the number of PRBs written.
  unsigned get_nof_prbs_written() const
  {
    if (!nof_prbs_written && grid_written) {
      // Check how many REs are storing a value different from the initial one.
      unsigned written_re_count = 0;
      for (auto sample : grid_data) {
        if (sample != init_value) {
          ++written_re_count;
        }
      }
      return written_re_count / NOF_SUBCARRIERS_PER_RB;
    }
    return nof_prbs_written;
  }
};

} // namespace testing
} // namespace ofh
} // namespace srsran
