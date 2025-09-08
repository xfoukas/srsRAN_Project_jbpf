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

#include "pdsch_encoder_hw_impl.h"
#include "srsran/phy/upper/channel_coding/crc_calculator.h"
#include "srsran/support/units.h"
#include <climits>

using namespace srsran;
using namespace srsran::ldpc;

void pdsch_encoder_hw_impl::encode(span<uint8_t>        codeword,
                                   span<const uint8_t>  transport_block,
                                   const configuration& config)
{
  // CB mode will be forced if TB mode is requested for a TB requiring a larger buffer than the maximum supported size.
  cb_mode         = encoder->is_cb_mode_supported();
  max_buffer_size = encoder->get_max_supported_buff_size();
  unsigned max_required_buff_size =
      std::max(static_cast<unsigned>(transport_block.size_bytes()),
               static_cast<unsigned>(units::bits(codeword.size()).round_up_to_bytes().value()));
  if (!cb_mode && max_required_buff_size > max_buffer_size) {
    cb_mode = true;
  }

  // Reserve a hardware-queue for the current encoding operation.
  encoder->reserve_queue();

  // Set the TB encoding parameters (common to all CBs) as required by the hardware-accelerated PDSCH encoder.
  hal::hw_pdsch_encoder_configuration hw_cfg = {};

  // Segmentation is only required in CB mode.
  unsigned nof_ops = 1;

  segmenter_config segmenter_cfg;
  segmenter_cfg.base_graph     = config.base_graph;
  segmenter_cfg.rv             = config.rv;
  segmenter_cfg.mod            = config.mod;
  segmenter_cfg.Nref           = config.Nref;
  segmenter_cfg.nof_layers     = config.nof_layers;
  segmenter_cfg.nof_ch_symbols = config.nof_ch_symbols;

  // Initialize the segmenter.
  segment_buffer = &segmenter->new_transmission(transport_block, segmenter_cfg);

  units::bits cb_size = segment_buffer->get_segment_length();
  if (cb_mode) {
    // Prepare codeblock data.
    cb_data.resize(cb_size.value());
  }
  set_hw_enc_tb_configuration(hw_cfg, transport_block, segmenter_cfg);
  if (cb_mode) {
    nof_ops = hw_cfg.nof_segments;
  }

  // Validate that all CBs have been succesfully enqueued and dequeued.
  unsigned last_enqueued_cb_id = 0, last_dequeued_cb_id = 0, offset = 0;
  bool     all_enqueued = false, all_dequeued = false;
  while (!all_enqueued || !all_dequeued) {
    // Enqueue each CB in the TB
    bool enqueued = false;
    for (unsigned cb_id = last_enqueued_cb_id; cb_id != nof_ops; ++cb_id) {
      // Update the pointer to the CB to be currently enqueued.
      last_enqueued_cb_id = cb_id;

      // Get the data to be encoded.
      span<const uint8_t> data;
      if (cb_mode) {
        // Copy codeblock data, including TB and/or CB CRC if applicable, as well as filler and zero padding bits.
        segment_buffer->read_codeblock(cb_data, transport_block, cb_id);

        // Set the encoding parameters of the CB as required by the hardware-accelerated PDSCH encoder.
        set_hw_enc_cb_configuration(hw_cfg, segment_buffer->get_cb_metadata(cb_id), cb_id);

        // Retrieve the current CB to be encoded.
        unsigned segment_length =
            static_cast<unsigned>(segment_buffer->get_segment_length().value()) - hw_cfg.nof_filler_bits;
        unsigned segment_length_bytes = static_cast<unsigned>(ceil(static_cast<float>(segment_length) / 8.0));
        data                          = cb_data.get_buffer().first(segment_length_bytes);
      } else {
        // Set the encoding parameters of the CB as required by the hardware-accelerated PDSCH encoder.
        set_hw_enc_cb_configuration(hw_cfg, {}, cb_id);

        // Encode the full TB at once.
        data = transport_block;
      }
      // Enqueue the hardware-accelerated PDSCH encoding operation.
      enqueued = encoder->enqueue_operation(data, {}, cb_id);

      // Exit the enqueing loop in case the operation couldn't be enqueued.
      if (!enqueued) {
        break;
      }
    }

    // The loop is exited when all CBs in the TB have been enqueued, the queue is full or because enqueuing failed for a
    // given CB.
    if (enqueued) {
      // Update the enqueuing point to ensure that each CB is only enqueued once.
      if (last_enqueued_cb_id == nof_ops - 1) {
        ++last_enqueued_cb_id;
        all_enqueued = true;
      }
    }

    // Reset the dequeuing control variables.
    unsigned num_dequeued = 0;
    bool     dequeued     = false;

    // Dequeue each CB in the TB
    for (unsigned cb_id = last_dequeued_cb_id; cb_id != last_enqueued_cb_id; ++cb_id) {
      // Update the pointer to the CB to be currently dequeued.
      last_dequeued_cb_id = cb_id;

      // In CB mode, select the correct chunk of the output codeword.
      unsigned      rm_length = 0;
      span<uint8_t> codeblock;
      if (cb_mode) {
        rm_length = segment_buffer->get_rm_length(cb_id);
        srsran_assert(offset + rm_length <= codeword.size(), "Wrong codeword length.");

        codeblock = span<uint8_t>(codeword).subspan(offset, rm_length);
        codeblock_packed.resize(units::bits(rm_length).round_up_to_bytes().value());
      } else {
        codeblock = codeword;
        codeblock_packed.resize(units::bits(codeword.size()).round_up_to_bytes().value());
      }

      // Make sure at least one operation is dequeued.
      dequeued = false;
      while (!dequeued) {
        // Dequeue the hardware-accelerated PUSCH decoding operation (updates the softbuffer too).
        dequeued = encoder->dequeue_operation(codeblock, codeblock_packed, cb_id);
        if (!dequeued) {
          if (num_dequeued > 0) {
            break;
          }
        } else {
          ++num_dequeued;
        }
      }

      // Exit the dequeueing loop in case no more operations can be dequeued.
      if (!dequeued) {
        break;
      }

      // Update the dequeuing offset within the TB.
      offset += rm_length;
    }
    // The loop is exited when all enqueued CBs have been dequeued or because dequeuing failed for a given CB.
    if (dequeued) {
      // Dequeueing of a given CB only has to take place once.
      ++last_dequeued_cb_id;
      if (last_dequeued_cb_id == nof_ops) {
        all_dequeued = true;
      }
    }
  }

  // Free the hardware-queue utilized by completed encoding operation.
  encoder->free_queue();
}

void pdsch_encoder_hw_impl::set_hw_enc_tb_configuration(hal::hw_pdsch_encoder_configuration& hw_cfg,
                                                        span<const uint8_t>                  transport_block,
                                                        const segmenter_config&              cfg)
{
  using namespace units::literals;

  // Operation mode.
  hw_cfg.cb_mode = cb_mode;

  // Modulation.
  hw_cfg.modulation = cfg.mod;

  // Redundancy version.
  hw_cfg.rv = cfg.rv;

  // Each transport_block entry is a byte, and TBS can always be expressed as an integer number of bytes (see, e.g.,
  // TS38.214 Section 5.1.3.2).
  units::bits nof_tb_bits     = units::bytes(transport_block.size()).to_bits();
  units::bits nof_tb_crc_bits = segment_buffer->get_tb_crc_bits();
  hw_cfg.nof_tb_bits          = static_cast<unsigned>(nof_tb_bits.value());
  hw_cfg.nof_tb_crc_bits      = static_cast<unsigned>(nof_tb_crc_bits.value());

  // Number of segments.
  unsigned nof_segments = segment_buffer->get_nof_codeblocks();
  hw_cfg.nof_segments   = nof_segments;

  // In CB mode, both TB and CB CRC attachment are implemented by the segmenter (and, thus, disabled in the
  // accelerator). In TB mode, CB CRC attachment is supported (for segmented TBs), but TB CRC attachment is not.
  hw_cfg.attach_cb_crc = false;
  hw_cfg.attach_tb_crc = false;
  if (!cb_mode) {
    hw_cfg.attach_tb_crc = true;
    if (nof_segments > 1) {
      hw_cfg.attach_cb_crc = true;
    }

    // Compute the TB CRC.
    crc_calculator_checksum_t tb_crc     = compute_tb_crc(transport_block, hw_cfg.nof_tb_crc_bits);
    uint8_t                   crc_byte_0 = (static_cast<uint32_t>(tb_crc) >> 16) & 0xff;
    uint8_t                   crc_byte_1 = (static_cast<uint32_t>(tb_crc) >> 8) & 0xff;
    uint8_t                   crc_byte_2 = static_cast<uint32_t>(tb_crc) & 0xff;
    if (hw_cfg.nof_tb_crc_bits > 16) {
      hw_cfg.tb_crc = {crc_byte_0, crc_byte_1, crc_byte_2};
    } else {
      hw_cfg.tb_crc.resize(2);
      hw_cfg.tb_crc = {crc_byte_1, crc_byte_2};
    }
  }

  // Number of segments that will have a short rate-matched length. In TS38.212 Section 5.4.2.1, these correspond to
  // codeblocks whose length E_r is computed by rounding down - floor. For the remaining codewords, the length is
  // rounded up.
  unsigned nof_short_segments = segment_buffer->get_nof_short_segments();
  hw_cfg.nof_short_segments   = nof_short_segments;

  // Codeword length for short rate-matched segments in bits (Ea). The FPGA will insert the CB CRC, thus the CB size
  // needs to be adjusted.
  unsigned           cw_length_a = 0;
  codeblock_metadata common_cfg  = segment_buffer->get_cb_metadata(0);
  cw_length_a                    = segment_buffer->get_rm_length(0);
  hw_cfg.cw_length_a             = cw_length_a;
  // Codeword length for long rate-matched segments in bits (Eb). The FPGA will insert the CB CRC, thus the CB size
  // needs to be adjusted.
  unsigned cw_length_b = cw_length_a;
  if (nof_segments > nof_short_segments) {
    cw_length_b = segment_buffer->get_rm_length(nof_short_segments);
  }
  hw_cfg.cw_length_b = cw_length_b;

  // LDPC lifting size.
  unsigned lifting_size = common_cfg.tb_common.lifting_size;
  hw_cfg.lifting_size   = lifting_size;

  // Base graph index and length of the circular buffer in bits, as described in TS38.212 Section 5.4.2.1.
  unsigned N              = 0;
  hw_cfg.base_graph_index = cfg.base_graph;
  if (cfg.base_graph == ldpc_base_graph_type::BG1) {
    N = lifting_size * 66;
  } else {
    N = lifting_size * 50;
  }
  hw_cfg.Ncb  = N;
  hw_cfg.Nref = cfg.Nref;

  // Number of information bits that is assigned to a segment.
  units::bits cb_info_bits = segment_buffer->get_cb_info_bits(0);
  hw_cfg.nof_segment_bits  = static_cast<unsigned>(cb_info_bits.value());

  if (!cb_mode) {
    // Number of filler bits.
    units::bits nof_filler_bits = segment_buffer->get_nof_filler_bits();
    hw_cfg.nof_filler_bits      = static_cast<unsigned>(nof_filler_bits.value());
  }
}

void pdsch_encoder_hw_impl::set_hw_enc_cb_configuration(hal::hw_pdsch_encoder_configuration& hw_cfg,
                                                        const codeblock_metadata&            cb_cfg,
                                                        unsigned                             cb_index)
{
  if (cb_mode) {
    // Number of filler bits in this segment.
    hw_cfg.nof_filler_bits = cb_cfg.cb_specific.nof_filler_bits;

    // Rate-matched length of this segment in bits.
    hw_cfg.rm_length = cb_cfg.cb_specific.rm_length;
  }

  // Set configuration in the HW accelerated encoder.
  encoder->configure_operation(hw_cfg, cb_index);
}

crc_calculator_checksum_t pdsch_encoder_hw_impl::compute_tb_crc(span<const uint8_t> transport_block,
                                                                unsigned            nof_tb_crc_bits)
{
  crc_calculator& tb_crc_calculator = (nof_tb_crc_bits == 16) ? *crc_set.crc16 : *crc_set.crc24A;
  return tb_crc_calculator.calculate_byte(transport_block);
}
