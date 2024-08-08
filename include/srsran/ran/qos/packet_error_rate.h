/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include <cmath>
#include <cstdint>

namespace srsran {

/// \brief Representation of the Packet Error Rate (PER) as defined in TS 38.473 and TS 23.501, 5.7.3.5.
///
/// The Packet Error Rate (PER) defines an upper bound for the rate of PDUs (e.g. IP packets) that have been processed
/// by the sender of a link layer protocol (e.g. RLC in RAN of a 3GPP access) but that are not successfully delivered
/// by the corresponding receiver to the upper layer (e.g. PDCP in RAN of a 3GPP access). Thus, the PER defines an
/// upper bound for a rate of non-congestion related packet losses.
struct packet_error_rate_t {
  uint8_t scalar   = 0;
  uint8_t exponent = 0;

  constexpr packet_error_rate_t() = default;
  constexpr packet_error_rate_t(uint8_t scalar_, uint8_t exponent_) : scalar(scalar_), exponent(exponent_) {}

  static packet_error_rate_t make(double per_)
  {
    srsran_assert(per_ > 0 and per_ < 1.0, "Invalid PER value {}", per_);
    double exp_val = std::floor(std::log10(per_));
    double scalar  = std::round(per_ / std::pow(10, exp_val));
    return packet_error_rate_t{static_cast<uint8_t>(scalar), static_cast<uint8_t>(-exp_val)};
  }

  double to_double() const { return static_cast<double>(scalar) * std::pow(10, -static_cast<int>(exponent)); }
};

} // namespace srsran