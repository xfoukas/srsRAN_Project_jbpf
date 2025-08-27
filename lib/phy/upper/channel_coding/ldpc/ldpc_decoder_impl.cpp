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

#include "ldpc_decoder_impl.h"
#include "ldpc_luts_impl.h"
#include "srsran/srsvec/copy.h"
#include "srsran/srsvec/fill.h"
#include "srsran/srsvec/zero.h"
#include "srsran/support/srsran_assert.h"

using namespace srsran;
using namespace srsran::ldpc;

void ldpc_decoder_impl::init(const configuration& cfg)
{
  uint8_t  pos   = get_lifting_size_position(cfg.block_conf.tb_common.lifting_size);
  unsigned skip  = (cfg.block_conf.tb_common.base_graph == ldpc_base_graph_type::BG2) ? NOF_LIFTING_SIZES : 0;
  current_graph  = &graph_array[skip + pos];
  bg_N_full      = current_graph->get_nof_BG_var_nodes_full();
  bg_N_short     = current_graph->get_nof_BG_var_nodes_short();
  bg_M           = current_graph->get_nof_BG_check_nodes();
  bg_K           = current_graph->get_nof_BG_info_nodes();
  bg_N_high_rate = bg_K + 4;
  srsran_assert(bg_K == bg_N_full - bg_M, "Invalid bg_K value '{}'", bg_K);
  lifting_size = static_cast<uint16_t>(cfg.block_conf.tb_common.lifting_size);

  max_iterations = cfg.algorithm_conf.max_iterations;
  srsran_assert(max_iterations > 0, "Max iterations must be different to 0");

  scaling_factor = cfg.algorithm_conf.scaling_factor;
  srsran_assert((scaling_factor > 0) && (scaling_factor < 1), "Scaling factor must be between 0 and 1 exclusively");

  unsigned nof_crc_bits = cfg.block_conf.cb_specific.nof_crc_bits;
  srsran_assert((nof_crc_bits == 16) || (nof_crc_bits == 24), "Invalid number of CRC bits.");

  nof_significant_bits = bg_K * lifting_size - cfg.block_conf.cb_specific.nof_filler_bits;

  specific_init();
}

std::optional<unsigned> ldpc_decoder_impl::decode(bit_buffer&                      output,
                                                  span<const log_likelihood_ratio> input,
                                                  crc_calculator*                  crc,
                                                  const configuration&             cfg)
{
  init(cfg);

  uint16_t message_length   = bg_K * lifting_size;
  uint16_t max_input_length = bg_N_short * lifting_size;
  srsran_assert(output.size() == message_length,
                "The output size {} is not equal to the message length {}.",
                output.size(),
                message_length);
  srsran_assert(input.size() <= max_input_length,
                "The input size {} exceeds the maximum message length {}.",
                input.size(),
                max_input_length);

  // The minimum input length is message_length + two times the lifting size.
  uint16_t min_input_length = message_length + 2 * lifting_size;
  srsran_assert(input.size() >= min_input_length,
                "The input length {} does not reach minimum {}",
                input.size(),
                min_input_length);

  // Find the last soft bit in the buffer and trim the output.
  const log_likelihood_ratio* last =
      std::find_if(input.rbegin(), input.rend(), [](const log_likelihood_ratio& in) { return in != 0; }).base();

  // Determine input length.
  unsigned input_size = std::distance(input.begin(), last);

  // The input meaningful number of bits must contain the message length number of bits.
  if (input_size < message_length) {
    // If the codeblock CRC check is external, set all bits to one (so that the CRC will fail).
    if (crc == nullptr) {
      output.one();
    }
    return std::nullopt;
  }

  // Ensure check-to-variable messages are not initialized.
  std::fill(is_check_to_var_initialized.begin(), is_check_to_var_initialized.end(), false);

  load_soft_bits(input, input_size);

  // The minimum codeblock length is message_length + four times the lifting size
  // (that is, the length of the high-rate region).
  uint16_t min_codeblock_length = message_length + 4 * lifting_size;
  // The decoder works with at least min_codeblock_length bits. Recall that the encoder also shortens
  // the codeblock by 2 * lifting size before returning it as output.
  codeblock_length = std::max(input_size + 2UL * lifting_size, static_cast<size_t>(min_codeblock_length));
  // The decoder works with a codeblock length that is a multiple of the lifting size.
  if (codeblock_length % lifting_size != 0) {
    codeblock_length = (codeblock_length / lifting_size + 1) * lifting_size;
  }

  unsigned nof_layers = codeblock_length / lifting_size - bg_K;

  for (unsigned i_iteration = 0; i_iteration != max_iterations; ++i_iteration) {
    for (unsigned i_layer = 0; i_layer != nof_layers; ++i_layer) {
      update_variable_to_check_messages(i_layer);

      update_check_to_variable_messages(i_layer);

      update_soft_bits(i_layer);
    }

    // If a CRC calculator was passed with the configuration parameters.
    if (crc != nullptr) {
      // Get hard bits.
      bool success = get_hard_bits(output);

      // Early stop. The hard bits must be successful.
      if (success && crc->calculate(output.first(nof_significant_bits)) == 0) {
        return i_iteration + 1;
      }
    }
  }

  // If a CRC calculator was passed with the configuration parameters and we hit this point, the codeblock wasn't
  // decoded correctly.
  if (crc != nullptr) {
    return {};
  }

  // We reach this point only if we don't have a CRC calculator for early stopping: we return whatever message we could
  // reconstruct after max_iterations (note that we don't know whether the message is correct or not).
  get_hard_bits(output);
  return {};
}

void ldpc_decoder_impl::load_soft_bits(span<const log_likelihood_ratio> llrs, unsigned nof_llr)
{
  // Compute the number of data nodes fully occupied by the llrs (the + 2 is due to the shortened nodes at the beginning
  // of the codeblock).
  unsigned nof_full_nodes = llrs.size() / lifting_size + 2;

  // Copy input llrs and organize them by nodes.
  span<const log_likelihood_ratio> llr_view = llrs;
  // Recall that the first 2 * lifting_size bits (2 nodes) are not transmitted.
  span<log_likelihood_ratio> soft_bits_view(soft_bits);
  srsvec::zero(soft_bits_view.first(2 * node_size_byte));
  soft_bits_view = soft_bits_view.last(soft_bits_view.size() - 2 * node_size_byte);
  for (unsigned i_node = 2 * node_size_byte, max_node = nof_full_nodes * node_size_byte; i_node != max_node;
       i_node += node_size_byte) {
    // Copy input LLR in the soft bits.
    if (nof_llr != 0) {
      clamp(
          soft_bits_view.first(lifting_size), llr_view.first(lifting_size), soft_bits_clamp_low, soft_bits_clamp_high);
    } else {
      srsvec::zero(soft_bits_view.first(lifting_size));
    }

    // Advance input LLR.
    llr_view = llr_view.last(llr_view.size() - lifting_size);
    nof_llr  = (nof_llr >= lifting_size) ? (nof_llr - lifting_size) : 0;

    // Zero node tail soft bits.
    srsvec::zero(soft_bits_view.subspan(lifting_size, node_size_byte - lifting_size));

    // Recall that soft bits may have zero padding in SIMD implementations (i.e., when node_size_byte != lifting_size).
    soft_bits_view = soft_bits_view.last(soft_bits_view.size() - node_size_byte);
  }

  // The length of llrs may not be an exact multiple of the lifting size.
  unsigned tail_positions = llr_view.size();
  if (tail_positions != 0) {
    // Copy last LLRs.
    srsvec::copy(soft_bits_view.first(tail_positions), llr_view);
    // Zero the remaining soft bits.
    srsvec::zero(soft_bits_view.subspan(tail_positions, node_size_byte - lifting_size));
  }
}

void ldpc_decoder_impl::update_variable_to_check_messages(unsigned check_node)
{
  // Retrieve list of variable nodes connected to this check node.
  const BG_adjacency_row_t& current_var_indices = current_graph->get_adjacency_row(check_node);

  // Find first NO_EDGE in current_var_indices.
  const auto* this_var_index_end =
      std::find_if(current_var_indices.cbegin(), current_var_indices.cend(), [this](auto& element) {
        return (element == NO_EDGE) || (element >= bg_N_high_rate);
      });

  // Iterate all variable nodes connected to this check node.
  for (BG_adjacency_row_t::const_iterator this_var_index = current_var_indices.cbegin();
       this_var_index != this_var_index_end;
       ++this_var_index) {
    unsigned                         i_node   = *this_var_index;
    span<const log_likelihood_ratio> soft     = get_soft_bits(i_node);
    span<const log_likelihood_ratio> c2v      = get_check_to_var(check_node, i_node);
    span<log_likelihood_ratio>       v2c      = get_var_to_check(i_node, 0);
    span<log_likelihood_ratio>       v2c_copy = get_var_to_check(i_node, lifting_size);
    if (is_check_to_var_initialized[check_node]) {
      compute_var_to_check_msgs(v2c, soft, c2v);
    } else {
      srsvec::copy(v2c, soft);
    }
    srsvec::copy(v2c_copy, v2c);
  }

  // Next, update the messages corresponding to the extension region, if applicable.
  // From layer 4 onwards, each layer is connected to only one consecutive block of lifting_size bits.
  if (check_node >= 4) {
    unsigned                         i_node   = bg_N_high_rate + check_node - 4;
    span<const log_likelihood_ratio> soft     = get_soft_bits(i_node);
    span<const log_likelihood_ratio> c2v      = get_check_to_var(check_node, bg_N_high_rate);
    span<log_likelihood_ratio>       v2c      = get_var_to_check(bg_N_high_rate, 0);
    span<log_likelihood_ratio>       v2c_copy = get_var_to_check(bg_N_high_rate, lifting_size);
    if (is_check_to_var_initialized[check_node]) {
      compute_var_to_check_msgs(v2c, soft, c2v);
    } else {
      srsvec::copy(v2c, soft);
    }
    srsvec::copy(v2c_copy, v2c);
  }
}

void ldpc_decoder_impl::update_soft_bits(unsigned check_node)
{
  const BG_adjacency_row_t& current_var_indices = current_graph->get_adjacency_row(check_node);
  for (BG_adjacency_row_t::const_iterator this_var_index = current_var_indices.cbegin(),
                                          last_var_index = current_var_indices.cend();
       (this_var_index != last_var_index) && (*this_var_index != NO_EDGE);
       ++this_var_index) {
    unsigned                         i_node            = std::min(*this_var_index, bg_N_high_rate);
    span<const log_likelihood_ratio> this_check_to_var = get_check_to_var(check_node, i_node);
    span<const log_likelihood_ratio> this_var_to_check = get_var_to_check(i_node, 0);
    span<log_likelihood_ratio>       this_soft_bits    = get_soft_bits(*this_var_index);
    compute_soft_bits(this_soft_bits, this_var_to_check, this_check_to_var);
  }
}

void ldpc_decoder_impl::update_check_to_variable_messages(unsigned check_node)
{
  // Buffer to store the minimum (in absolute value) variable-to-check message.
  std::array<log_likelihood_ratio, MAX_LIFTING_SIZE> min_var_to_check;
  // Buffer to store the second minimum (in absolute value) variable-to-check message for each base graph check node.
  std::array<log_likelihood_ratio, MAX_LIFTING_SIZE> second_min_var_to_check;
  // Buffer to store the index of the minimum-valued variable-to-check message.
  std::array<uint8_t, MAX_LIFTING_SIZE> min_var_to_check_index;
  // Buffer to store the sign product of all variable-to-check messages.
  std::array<uint8_t, MAX_LIFTING_SIZE> sign_prod_var_to_check;

  // Take views of the above buffers.
  span<log_likelihood_ratio> min_var_to_check_view = span<log_likelihood_ratio>(min_var_to_check).first(node_size_byte);
  span<log_likelihood_ratio> second_min_var_to_check_view =
      span<log_likelihood_ratio>(second_min_var_to_check).first(node_size_byte);
  span<uint8_t> min_var_to_check_index_view = span<uint8_t>(min_var_to_check_index).first(node_size_byte);
  span<uint8_t> sign_prod_var_to_check_view = span<uint8_t>(sign_prod_var_to_check).first(node_size_byte);

  // Reset temporal buffers.
  srsvec::fill(min_var_to_check_view, LLR_MAX);
  srsvec::fill(second_min_var_to_check_view, LLR_MAX);
  srsvec::zero(min_var_to_check_index_view);
  // For the optimized implementations, we store 0 if the sign is positive and 1 if the sign is negative. Therefore, the
  // following is equivalent to setting al signs to +1.
  srsvec::zero(sign_prod_var_to_check_view);

  // Retrieve list of variable nodes connected to this check node.
  const BG_adjacency_row_t& current_var_indices = current_graph->get_adjacency_row(check_node);

  // Find first NO_EDGE in current_var_indices.
  const auto* this_var_index_end = std::find(current_var_indices.begin(), current_var_indices.end(), NO_EDGE);

  // For all variable nodes connected to this check node.
  unsigned var_node = 0;
  for (const auto* this_var_index_itr = current_var_indices.cbegin(); this_var_index_itr != this_var_index_end;
       ++this_var_index_itr, ++var_node) {
    // Rotate the variable node as specified by the base graph.
    unsigned shift          = current_graph->get_lifted_node(check_node, *this_var_index_itr);
    unsigned v2c_base_index = std::min(*this_var_index_itr, bg_N_high_rate);

    span<const log_likelihood_ratio> rotated_node = get_var_to_check(v2c_base_index, shift);

    analyze_var_to_check_msgs(min_var_to_check_view,
                              second_min_var_to_check_view,
                              min_var_to_check_index_view,
                              sign_prod_var_to_check_view,
                              rotated_node,
                              var_node);
  }

  // Scale the message to compensate for approximations.
  scale(min_var_to_check_view, min_var_to_check_view);
  scale(second_min_var_to_check_view, second_min_var_to_check_view);

  // For all variable nodes connected to this check node.
  var_node = 0;
  for (const auto* this_var_index_itr = current_var_indices.cbegin(); this_var_index_itr != this_var_index_end;
       ++this_var_index_itr, ++var_node) {
    unsigned shift          = current_graph->get_lifted_node(check_node, *this_var_index_itr);
    unsigned c2v_base_index = std::min(*this_var_index_itr, bg_N_high_rate);

    span<log_likelihood_ratio>       this_check_to_var = get_check_to_var(check_node, c2v_base_index);
    span<const log_likelihood_ratio> this_var_to_check = get_var_to_check(c2v_base_index, 0);
    span<const log_likelihood_ratio> rotated_node      = get_var_to_check(c2v_base_index, shift);

    compute_check_to_var_msgs(this_check_to_var,
                              this_var_to_check,
                              rotated_node,
                              min_var_to_check_view,
                              second_min_var_to_check_view,
                              min_var_to_check_index_view,
                              sign_prod_var_to_check_view,
                              shift,
                              var_node);
  }
  is_check_to_var_initialized[check_node] = true;
}

bool ldpc_decoder_impl::get_hard_bits(bit_buffer& out) const
{
  if (lifting_size == node_size_byte) {
    span<const log_likelihood_ratio> llrs = span<const log_likelihood_ratio>(soft_bits).first(out.size());
    return hard_decision(out, llrs);
  }

  // Perform hard-decision of the LLRs from the soft_bits array directly into the output without any padding.
  bool valid = true;
  for (unsigned i_node = 0; i_node != bg_K; ++i_node) {
    // View over the LLR.
    span<const log_likelihood_ratio> current_soft =
        span<const log_likelihood_ratio>(soft_bits).subspan(node_size_byte * i_node, lifting_size);

    // Perform hard decision of the node.
    valid &= hard_decision(out, current_soft, lifting_size * i_node);
  }

  return valid;
}
