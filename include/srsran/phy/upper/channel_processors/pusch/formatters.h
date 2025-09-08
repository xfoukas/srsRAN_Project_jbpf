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

#include "srsran/phy/upper/channel_processors/pusch/pusch_decoder_result.h"
#include "srsran/phy/upper/channel_processors/pusch/pusch_processor.h"
#include "srsran/phy/upper/channel_processors/pusch/pusch_processor_result_notifier.h"
#include "srsran/phy/upper/channel_state_information_formatters.h"
#include "srsran/ran/pusch/pusch_context_formatter.h"
#include "srsran/ran/uci/uci_formatters.h"
#include <fmt/std.h>

namespace srsran {
namespace detail {

class uci_bad_payload
{
public:
  uci_bad_payload(std::size_t size) : bad_payload(size, 2U) {}

  span<const uint8_t> get() const { return bad_payload; }

private:
  static_vector<uint8_t, uci_constants::MAX_NOF_PAYLOAD_BITS> bad_payload;
};

} // namespace detail
} // namespace srsran

namespace fmt {

/// \brief Custom formatter for \c pusch_processor::codeword_description.
template <>
struct formatter<srsran::pusch_processor::codeword_description> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::pusch_processor::codeword_description& codeword, FormatContext& ctx) const
  {
    helper.format_always(ctx, "rv={}", codeword.rv);
    helper.format_if_verbose(ctx, "bg={}", fmt::underlying(codeword.ldpc_base_graph));
    helper.format_if_verbose(ctx, "new_data={}", codeword.new_data);

    return ctx.out();
  }
};

/// \brief Custom formatter for \c pusch_processor::uci_description.
template <>
struct formatter<srsran::pusch_processor::uci_description> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::pusch_processor::uci_description& uci_desc, FormatContext& ctx) const
  {
    // Number of ACK, CSI Part 1 and CSI Part 2 bits.
    helper.format_if_verbose(ctx, "oack={}", uci_desc.nof_harq_ack);
    helper.format_if_verbose(ctx, "ocsi1={}", uci_desc.nof_csi_part1);
    helper.format_if_verbose(ctx, "part2={}", uci_desc.csi_part2_size);

    // Scaling and offset parameters.
    helper.format_if_verbose(ctx, "alpha={}", uci_desc.alpha_scaling);

    helper.format_if_verbose(ctx,
                             "betas=[{}, {}, {}]",
                             uci_desc.beta_offset_harq_ack,
                             uci_desc.beta_offset_csi_part1,
                             uci_desc.beta_offset_csi_part2);

    return ctx.out();
  }
};

/// \brief Custom formatter for \c pusch_processor::pdu_t.
template <>
struct formatter<srsran::pusch_processor::pdu_t> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::pusch_processor::pdu_t& pdu, FormatContext& ctx) const
  {
    if (pdu.context.has_value()) {
      helper.format_always(ctx, *pdu.context);
    } else {
      helper.format_always(ctx, "rnti=0x{:04x}", pdu.rnti);
    }
    helper.format_if_verbose(ctx, "bwp=[{}, {})", pdu.bwp_start_rb, pdu.bwp_start_rb + pdu.bwp_size_rb);
    helper.format_always(ctx, "prb={}", pdu.freq_alloc);
    helper.format_always(ctx, "symb=[{}, {})", pdu.start_symbol_index, pdu.start_symbol_index + pdu.nof_symbols);

    // UCI description.
    helper.format_always(ctx, pdu.uci);

    // Verbose parameters.
    helper.format_always(ctx, "mod={}", to_string(pdu.mcs_descr.modulation));
    helper.format_if_verbose(ctx, "tcr={}", pdu.mcs_descr.get_normalised_target_code_rate());

    // PUSCH data codeword if available.
    if (pdu.codeword.has_value()) {
      helper.format_always(ctx, *pdu.codeword);
    }

    helper.format_if_verbose(ctx, "n_id={}", pdu.n_id);
    helper.format_if_verbose(ctx, "dmrs_mask={}", pdu.dmrs_symbol_mask);
    helper.format_if_verbose(ctx, "tbs_lbrm={}bytes", pdu.tbs_lbrm);
    helper.format_if_verbose(ctx, "slot={}", pdu.slot);
    helper.format_if_verbose(ctx, "cp={}", pdu.cp.to_string());
    helper.format_if_verbose(ctx, "nof_layers={}", pdu.nof_tx_layers);
    helper.format_if_verbose(ctx, "ports={}", srsran::span<const uint8_t>(pdu.rx_ports));
    helper.format_if_verbose(ctx, "dc_position={}", pdu.dc_position);

    if (std::holds_alternative<srsran::pusch_processor::dmrs_configuration>(pdu.dmrs)) {
      const auto& dmrs_config = std::get<srsran::pusch_processor::dmrs_configuration>(pdu.dmrs);
      helper.format_if_verbose(ctx, "n_scr_id={}", dmrs_config.scrambling_id);
      helper.format_if_verbose(ctx, "n_scid={}", dmrs_config.n_scid);
      helper.format_if_verbose(ctx, "n_cdm_g_wd={}", dmrs_config.nof_cdm_groups_without_data);
      helper.format_if_verbose(ctx, "dmrs_type={}", (dmrs_config.dmrs == srsran::dmrs_type::TYPE1) ? 1 : 2);
    } else {
      const auto& dmrs_config = std::get<srsran::pusch_processor::dmrs_transform_precoding_configuration>(pdu.dmrs);
      helper.format_if_verbose(ctx, "n_rs_id={}", dmrs_config.n_rs_id);
    }

    return ctx.out();
  }
};

/// \brief Custom formatter for \c pusch_decoder_result.
template <>
struct formatter<srsran::pusch_decoder_result> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::pusch_decoder_result& result, FormatContext& ctx) const
  {
    helper.format_always(ctx, "crc={}", result.tb_crc_ok ? "OK" : "KO");
    helper.format_always(ctx, "iter={:.1f}", result.ldpc_decoder_stats.get_mean());

    helper.format_if_verbose(ctx, "max_iter={}", result.ldpc_decoder_stats.get_max());
    helper.format_if_verbose(ctx, "min_iter={}", result.ldpc_decoder_stats.get_min());
    helper.format_if_verbose(ctx, "nof_cb={}", result.nof_codeblocks_total);

    return ctx.out();
  }
};

/// \brief Custom formatter for \c pusch_processor_result_data.
template <>
struct formatter<srsran::pusch_processor_result_data> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::pusch_processor_result_data& result, FormatContext& ctx) const
  {
    helper.format_always(ctx, result.data);
    return ctx.out();
  }
};

/// \brief Custom formatter for \c pusch_processor_result_control.
template <>
struct formatter<srsran::pusch_processor_result_control> {
  /// Helper used to parse formatting options and format fields.
  srsran::delimited_formatter helper;

  /// Default constructor.
  formatter() = default;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return helper.parse(ctx);
  }

  template <typename FormatContext>
  auto format(const srsran::pusch_processor_result_control& result, FormatContext& ctx) const
  {
    if ((!result.harq_ack.payload.empty())) {
      if (result.harq_ack.status == srsran::uci_status::valid) {
        helper.format_always(ctx, "ack={:br}", result.harq_ack.payload);
      } else {
        helper.format_always(ctx, "ack={:#}", srsran::detail::uci_bad_payload(result.harq_ack.payload.size()).get());
      }
    }
    if ((!result.csi_part1.payload.empty())) {
      if (result.csi_part1.status == srsran::uci_status::valid) {
        helper.format_always(ctx, "csi1={:br}", result.csi_part1.payload);
      } else {
        helper.format_always(ctx, "csi1={:#}", srsran::detail::uci_bad_payload(result.csi_part1.payload.size()).get());
      }
    }
    if ((!result.csi_part2.payload.empty())) {
      if (result.csi_part2.status == srsran::uci_status::valid) {
        helper.format_always(ctx, "csi2={:br}", result.csi_part2.payload);
      } else {
        helper.format_always(ctx, "csi2={:#}", srsran::detail::uci_bad_payload(result.csi_part2.payload.size()).get());
      }
    }
    return ctx.out();
  }
};

} // namespace fmt
