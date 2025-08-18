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

#include "o_du_low_unit_factory.h"
#include "apps/services/worker_manager/worker_manager.h"
#include "du_low_config.h"
#include "du_low_config_translator.h"
#include "du_low_hal_factory.h"
#include "srsran/du/du_low/o_du_low_factory.h"
#include "srsran/ran/slot_pdu_capacity_constants.h"

using namespace srsran;

static void generate_dl_processor_config(downlink_processor_factory_sw_config& out_cfg,
                                         const du_low_unit_config&             unit_cfg,
                                         task_executor&                        pdsch_codeblock_executor,
                                         std::shared_ptr<hal::hw_accelerator_pdsch_enc_factory> hw_encoder_factory)
{
  out_cfg.ldpc_encoder_type   = "auto";
  out_cfg.crc_calculator_type = "auto";

  const du_low_unit_expert_threads_config& upper_phy_threads_cfg = unit_cfg.expert_execution_cfg.threads;

  // Check hardware acceleration usage.
  bool hwacc_pdsch_processor = hw_encoder_factory != nullptr;
  if (hwacc_pdsch_processor) {
    out_cfg.hw_encoder_factory = hw_encoder_factory;
  }

  // The flexible PDSCH processor implementation will be used by default.
  if ((upper_phy_threads_cfg.pdsch_processor_type == "auto") ||
      (upper_phy_threads_cfg.pdsch_processor_type == "flexible")) {
    // The worker pool in charge of processing PDSCH CBs is shared with the rest of the DL processors.
    unsigned nof_pdsch_codeblock_threads = upper_phy_threads_cfg.nof_dl_threads;

    // Setup parameters for synchronous operation:
    // - the batch size must be the maximum to avoid more than one batch; and
    // - the maximum number of simultaneous PDSCH equals to the number of DL processing threads.
    unsigned cb_batch_length            = pdsch_processor_flexible_configuration::synchronous_cb_batch_length;
    unsigned max_nof_simultaneous_pdsch = upper_phy_threads_cfg.nof_dl_threads;

    // Override default parameters if the CB batch length is set for asynchronous concurrent operation.
    if (upper_phy_threads_cfg.pdsch_cb_batch_length != du_low_unit_expert_threads_config::synchronous_cb_batch_length) {
      // For asynchronous operation:
      // - Use the given CB batch length;
      // - The number of simultaneous PDSCH is equal to the maximum number of PDSCH per slot times the maximum allowed
      //   processing time.
      cb_batch_length            = upper_phy_threads_cfg.pdsch_cb_batch_length;
      max_nof_simultaneous_pdsch = (MAX_UE_PDUS_PER_SLOT + 1) * unit_cfg.expert_phy_cfg.max_processing_delay_slots;
    }

    // Emplace configuration parameters.
    out_cfg.pdsch_processor.emplace<pdsch_processor_flexible_configuration>(
        pdsch_processor_flexible_configuration{.nof_pdsch_codeblock_threads   = nof_pdsch_codeblock_threads,
                                               .cb_batch_length               = cb_batch_length,
                                               .max_nof_simultaneous_pdsch    = max_nof_simultaneous_pdsch,
                                               .pdsch_codeblock_task_executor = pdsch_codeblock_executor

        });
  } else if (upper_phy_threads_cfg.pdsch_processor_type == "generic") {
    out_cfg.pdsch_processor.emplace<pdsch_processor_generic_configuration>();
  } else {
    srsran_assert(false,
                  "Invalid {}PDSCH processor type {}.",
                  hwacc_pdsch_processor ? "hardware-accelerated " : "",
                  upper_phy_threads_cfg.pdsch_processor_type);
  }
  out_cfg.nof_concurrent_threads = upper_phy_threads_cfg.nof_dl_threads;
}

o_du_low_unit_factory::o_du_low_unit_factory(const std::optional<du_low_unit_hal_config>& hal_config,
                                             unsigned                                     nof_cells) :
  hal_dependencies(make_du_low_hal_dependencies(hal_config, nof_cells))
{
}

o_du_low_unit o_du_low_unit_factory::create(const o_du_low_unit_config&       params,
                                            const o_du_low_unit_dependencies& dependencies)
{
  srs_du::o_du_low_config o_du_low_cfg;

  // Copy FAPI configuration.
  o_du_low_cfg.fapi_cfg = params.fapi_cfg;

  // Configure the metrics.
  o_du_low_cfg.enable_metrics = params.du_low_unit_cfg.metrics_cfg.enable_du_low;

  generate_o_du_low_config(o_du_low_cfg, params.du_low_unit_cfg, params.cells);

  // Fill the workers information.
  for (unsigned i = 0, e = o_du_low_cfg.du_low_cfg.cells.size(); i != e; ++i) {
    srs_du::du_low_cell_executor_mapper& cell_exec_map = dependencies.workers[i];

    srs_du::du_low_cell_config& cell = o_du_low_cfg.du_low_cfg.cells[i];

    generate_dl_processor_config(cell.dl_proc_cfg,
                                 params.du_low_unit_cfg,
                                 cell_exec_map.pdsch_codeblock_executor(),
                                 hal_dependencies.hw_encoder_factory);

    upper_phy_config& upper          = cell.upper_phy_cfg;
    upper.rg_gateway                 = &dependencies.rg_gateway;
    upper.rx_symbol_request_notifier = &dependencies.rx_symbol_request_notifier;
    upper.pucch_executor             = &cell_exec_map.pucch_executor();
    upper.pusch_executor             = &cell_exec_map.pusch_executor();
    upper.pusch_decoder_executor =
        (upper.nof_pusch_decoder_threads > 1) ? &cell_exec_map.pusch_decoder_executor() : nullptr;
    upper.prach_executor = &cell_exec_map.prach_executor();
    upper.srs_executor   = &cell_exec_map.srs_executor();
    if (hal_dependencies.hw_decoder_factory) {
      upper.hw_decoder_factory = hal_dependencies.hw_decoder_factory;
    }
    upper.pdcch_executor   = &cell_exec_map.pdcch_executor();
    upper.pdsch_executor   = &cell_exec_map.pdsch_executor();
    upper.ssb_executor     = &cell_exec_map.ssb_executor();
    upper.csi_rs_executor  = &cell_exec_map.csi_rs_executor();
    upper.prs_executor     = &cell_exec_map.prs_executor();
    upper.dl_grid_executor = &cell_exec_map.dl_grid_pool_executor();
  }

  o_du_low_unit unit;
  unit.o_du_lo = srs_du::make_o_du_low(o_du_low_cfg);
  report_error_if_not(unit.o_du_lo, "Invalid O-DU low");

  return unit;
}
