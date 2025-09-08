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

#include "srsran/phy/support/resource_grid_reader.h"
#include "srsran/phy/support/resource_grid_writer.h"
#include "srsran/phy/support/support_factories.h"
#include "srsran/srsvec/zero.h"
#include "srsran/support/srsran_test.h"
#include <random>

using namespace srsran;

static std::mt19937 rgen(0);

// Gets the tolerance from an expected value.
static float get_tolerance(cf_t expected_value)
{
  // The tolerance is calculated from the complex number based in brain float (BF16) precision.
  return std::max(std::abs(expected_value) / 256.0F, 1e-5F);
}

// Creates a resource grid.
static std::unique_ptr<resource_grid> create_resource_grid(unsigned nof_ports, unsigned nof_symbols, unsigned nof_subc)
{
  std::shared_ptr<channel_precoder_factory> precoding_factory = create_channel_precoder_factory("auto");
  TESTASSERT(precoding_factory != nullptr, "Invalid channel precoder factory.");
  std::shared_ptr<resource_grid_factory> rg_factory = create_resource_grid_factory();
  TESTASSERT(rg_factory != nullptr, "Invalid resource grid factory.");

  return rg_factory->create(nof_ports, nof_symbols, nof_subc);
}

void test_all_zero(unsigned nof_ports, unsigned nof_symbols, unsigned nof_subc)
{
  // Create grid
  std::unique_ptr<resource_grid> grid = create_resource_grid(nof_ports, nof_symbols, nof_subc);

  // Set all grid to zero
  grid->set_all_zero();

  // Verify all RE are zero.
  for (unsigned port = 0; port != nof_ports; ++port) {
    // Verify the grid for the port is empty.
    TESTASSERT_EQ(true, grid->get_reader().is_empty(port));

    for (unsigned symbol = 0; symbol != nof_symbols; ++symbol) {
      // Get resource grid data for the given symbol
      std::vector<cf_t> rg_data(nof_subc);
      grid->get_reader().get(rg_data, port, symbol, 0);

      for (unsigned subc = 0; subc != nof_subc; ++subc) {
        cf_t re = rg_data[subc];
        TESTASSERT(iszero(re.real()), "Real is not zero.");
        TESTASSERT(iszero(re.imag()), "Imaginary is not zero.");
      }
    }
  }
}

void test_mask_bitset(unsigned nof_ports, unsigned nof_symbols, unsigned nof_subc, unsigned nof_elements)
{
  // Create grid and zero.
  std::unique_ptr<resource_grid> grid = create_resource_grid(nof_ports, nof_symbols, nof_subc);
  grid->set_all_zero();

  std::uniform_int_distribution<unsigned> port_dist(0, nof_ports - 1);
  std::uniform_int_distribution<unsigned> symbol_dist(0, nof_symbols - 1);
  std::uniform_int_distribution<unsigned> subc_dist(0, nof_subc - 1);
  std::uniform_real_distribution<float>   value_dist(-1.0, +1.0);

  // Put elements in grid.
  unsigned                     symbol_idx = symbol_dist(rgen);
  std::vector<cf_t>            symbols_gold(nof_elements);
  bounded_bitset<MAX_RB * NRE> mask(nof_subc);

  // Fill mask and generate symbols.
  unsigned port_gold = port_dist(rgen);
  for (unsigned i = 0; i != nof_elements; ++i) {
    unsigned subc = 0;

    // Select a subcarrier that has not been set yet.
    do {
      subc = subc_dist(rgen);
    } while (mask.test(subc));

    // Create random allocation
    mask.set(subc);
    symbols_gold[i] = {value_dist(rgen), value_dist(rgen)};
  }

  // Put elements.
  span<const cf_t> symbol_buffer_put = grid->get_writer().put(port_gold, symbol_idx, 0, mask, symbols_gold);

  // Make sure all symbols are used.
  TESTASSERT(symbol_buffer_put.empty());

  // Assert grid entries.
  unsigned count = 0;
  for (unsigned port = 0; port != nof_ports; ++port) {
    // Verify the grid for the port is NOT empty.
    TESTASSERT_EQ(port != port_gold, grid->get_reader().is_empty(port));

    for (unsigned symbol = 0; symbol != nof_symbols; ++symbol) {
      // Get resource grid data for the given symbol.
      std::vector<cf_t> rg_data(nof_subc);
      grid->get_reader().get(rg_data, port, symbol, 0);

      for (unsigned subc = 0; subc != nof_subc; ++subc) {
        cf_t gold  = {0.0, 0.0};
        cf_t value = rg_data[subc];

        if (port == port_gold && symbol == symbol_idx && mask.test(subc)) {
          gold = symbols_gold[count];
          count++;
        }

        float error = std::abs(gold - value);
        TESTASSERT(error < get_tolerance(gold), "{} != {}", gold, value);
      }
    }
  }

  // Get elements using the same mask.
  std::vector<cf_t> symbols(nof_elements);
  span<cf_t>        symbol_buffer_get = grid->get_reader().get(symbols, port_gold, symbol_idx, 0, mask);

  // Make sure all symbols are used.
  TESTASSERT(symbol_buffer_get.empty(), "Symbol buffer - not empty.");

  // Assert that symbols are equal.
  for (unsigned i = 0; i != nof_elements; ++i) {
    cf_t gold  = symbols_gold[i];
    cf_t value = symbols[i];

    float error = std::abs(gold - value);
    TESTASSERT(error < get_tolerance(gold), "{} != {}", gold, value);
  }
}

void test_consecutive(unsigned nof_ports, unsigned nof_symbols, unsigned nof_subc, unsigned nof_elements)
{
  // Create grid and zero
  std::unique_ptr<resource_grid> grid = create_resource_grid(nof_ports, nof_symbols, nof_subc);
  grid->set_all_zero();

  std::uniform_int_distribution<unsigned> port_dist(0, nof_ports - 1);
  std::uniform_int_distribution<unsigned> symbol_dist(0, nof_symbols - 1);
  std::uniform_int_distribution<unsigned> subc_dist(0, nof_subc - 1 - nof_elements);
  std::uniform_real_distribution<float>   value_dist(-1.0, +1.0);

  // Select port
  unsigned port_gold = port_dist(rgen);

  // Put elements in grid
  unsigned          symbol_idx = symbol_dist(rgen);
  std::vector<cf_t> symbols_gold(nof_elements);

  // Select initial subcarrier
  unsigned k_init = subc_dist(rgen);

  // Create random data
  for (unsigned i = 0; i != nof_elements; ++i) {
    symbols_gold[i] = {value_dist(rgen), value_dist(rgen)};
  }

  // Put element
  grid->get_writer().put(port_gold, symbol_idx, k_init, symbols_gold);

  // Assert grid
  unsigned count = 0;
  for (unsigned port = 0; port != nof_ports; ++port) {
    // Verify the grid for the port is NOT empty.
    TESTASSERT_EQ(port != port_gold, grid->get_reader().is_empty(port));

    for (unsigned symbol = 0; symbol != nof_symbols; ++symbol) {
      // Get resource grid data for the given symbol
      std::vector<cf_t> rg_data(nof_subc);
      grid->get_reader().get(rg_data, port, symbol, 0);

      for (unsigned subc = 0; subc != nof_subc; ++subc) {
        cf_t gold  = {0.0, 0.0};
        cf_t value = rg_data[subc];

        if (port == port_gold && symbol == symbol_idx && (subc >= k_init && subc < k_init + nof_elements)) {
          gold = symbols_gold[count];
          count++;
        }

        float error = std::abs(gold - value);
        TESTASSERT(error < get_tolerance(gold), "{} != {}", gold, value);
      }
    }
  }

  // Get elements
  std::vector<cf_t> symbols(nof_elements);
  grid->get_reader().get(symbols, port_gold, symbol_idx, k_init);

  // Assert symbols
  for (unsigned i = 0; i != nof_elements; ++i) {
    cf_t gold  = symbols_gold[i];
    cf_t value = symbols[i];

    float error = std::abs(gold - value);
    TESTASSERT(error < get_tolerance(gold), "{} != {}", gold, value);
  }

  // Test view contents
  span<const cbf16_t> view = grid->get_reader().get_view(port_gold, symbol_idx).subspan(k_init, nof_subc - k_init);
  for (unsigned i = 0; i != nof_elements; ++i) {
    cf_t gold  = symbols_gold[i];
    cf_t value = to_cf(view[i]);

    float error = std::abs(gold - value);
    TESTASSERT(error < get_tolerance(gold), "{} != {}", gold, value);
  }
}

int main()
{
  // Iterates over the possible number of ports
  for (unsigned nof_ports : {1, 2, 4}) {
    // Iterate over the posisble number of symbols per slot
    for (unsigned nof_symbols : {14}) {
      // Iterate over a symbolic number of subcarriers
      for (unsigned nof_subc : {6 * 12, 15 * 12}) {
        test_all_zero(nof_ports, nof_symbols, nof_subc);
        // Test symbolic number of elements
        for (unsigned nof_elements : {1, 2, 4, 8, 16, 32}) {
          test_mask_bitset(nof_ports, nof_symbols, nof_subc, nof_elements);
          test_consecutive(nof_ports, nof_symbols, nof_subc, nof_elements);
        }
      }
    }
  }

  return 0;
}
