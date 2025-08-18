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

#include "srsran/fapi_adaptor/precoding_matrix_mapper.h"
#include "precoding_matrix_mapper_functions.h"
#include "srsran/srslog/srslog.h"
#include "srsran/support/srsran_assert.h"

using namespace srsran;
using namespace fapi_adaptor;

precoding_matrix_mapper::precoding_matrix_mapper(unsigned sector_id_,
                                                 unsigned nof_ports_,
                                                 const precoding_matrix_mapper_codebook_offset_configuration& config) :
  sector_id(sector_id_),
  logger(srslog::fetch_basic_logger("FAPI")),
  nof_ports(nof_ports_),
  pdsch_omni_offset(config.pdsch_omni_offset),
  ssb_codebook_offsets(config.ssb_codebook_offsets),
  pdsch_codebook_offsets(config.pdsch_codebook_offsets),
  pdcch_codebook_offsets(config.pdcch_codebook_offsets),
  csi_rs_codebook_offsets(config.csi_rs_codebook_offsets)
{
  srsran_assert(!ssb_codebook_offsets.empty(), "Invalid offset configuration");
  srsran_assert(!pdsch_codebook_offsets.empty(), "Invalid offset configuration");
  srsran_assert(!pdcch_codebook_offsets.empty(), "Invalid offset configuration");
  srsran_assert(!csi_rs_codebook_offsets.empty(), "Invalid offset configuration");
}

unsigned precoding_matrix_mapper::map(const mac_csi_rs_precoding_info& precoding_info) const
{
  return csi_rs_codebook_offsets[0] + get_csi_rs_precoding_matrix_index();
}

unsigned precoding_matrix_mapper::map(const mac_ssb_precoding_info& precoding_info) const
{
  return ssb_codebook_offsets[0] + get_ssb_precoding_matrix_index();
}

unsigned precoding_matrix_mapper::map(const mac_pdcch_precoding_info& precoding_info) const
{
  return pdcch_codebook_offsets[0] + get_pdcch_precoding_matrix_index();
}

/// Returns the precoding matrix index for the PDSCH codebook using the given offset, precoding information and number
/// of ports.
static unsigned get_pdsch_precoding_matrix_index(unsigned              offset,
                                                 const csi_report_pmi& precoding_info,
                                                 unsigned              nof_ports,
                                                 unsigned              nof_layers,
                                                 unsigned              sector_id,
                                                 srslog::basic_logger& logger)
{
  if (nof_ports == 1U) {
    logger.debug("Sector#{}: One port PDSCH precoding matrix, nof_layers={}", sector_id, nof_layers);

    return offset + get_pdsch_one_port_precoding_matrix_index();
  }

  if (nof_ports == 2U) {
    srsran_assert(std::holds_alternative<csi_report_pmi::two_antenna_port>(precoding_info.type),
                  "Expected PMI information");
    unsigned pmi = std::get<csi_report_pmi::two_antenna_port>(precoding_info.type).pmi;

    logger.debug("Sector#{}: Two ports PDSCH precoding matrix, pmi={}, nof_layers={}", sector_id, pmi, nof_layers);

    return offset + get_pdsch_two_port_precoding_matrix_index(pmi);
  }

  if (nof_ports == 4U) {
    srsran_assert(std::holds_alternative<csi_report_pmi::typeI_single_panel_4ports_mode1>(precoding_info.type),
                  "Invalid PMI information");
    const auto& report = std::get<csi_report_pmi::typeI_single_panel_4ports_mode1>(precoding_info.type);

    logger.debug("Sector#{}: Four ports PDSCH precoding matrix, i11={}, i13={}, i2={}, nof_layers={}",
                 sector_id,
                 report.i_1_1,
                 (report.i_1_3) ? report.i_1_3.value() : -1,
                 report.i_2,
                 nof_layers);

    return offset + get_pdsch_four_port_precoding_matrix_index(
                        report.i_1_1, (report.i_1_3) ? report.i_1_3.value() : 0U, report.i_2);
  }

  return 0;
}

unsigned precoding_matrix_mapper::map(const mac_pdsch_precoding_info& precoding_info, unsigned nof_layers) const
{
  srsran_assert(nof_layers > 0, "Invalid number of layers={}", nof_layers);

  if (!precoding_info.report) {
    logger.debug("Sector#{}: Omnidirectional PDSCH precoding matrix, nof_layers={}", sector_id, nof_layers);

    return pdsch_omni_offset + get_pdsch_omnidirectional_precoding_matrix_index();
  }

  unsigned layer_index = nof_layers - 1U;
  srsran_assert(layer_index < pdsch_codebook_offsets.size(), "Invalid layer index value {}", layer_index);

  return get_pdsch_precoding_matrix_index(
      pdsch_codebook_offsets[layer_index], *precoding_info.report, nof_ports, nof_layers, sector_id, logger);
}
