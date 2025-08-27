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

#include "cell_meas_manager_test_helpers.h"
#include "lib/cu_cp/cell_meas_manager/cell_meas_manager_helpers.h"
#include "srsran/cu_cp/cu_cp_configuration_helpers.h"

using namespace srsran;
using namespace srs_cu_cp;

cell_meas_manager_test::cell_meas_manager_test() :
  cu_cp_cfg([this]() {
    cu_cp_configuration cucfg     = config_helpers::make_default_cu_cp_config();
    cucfg.services.timers         = &timers;
    cucfg.services.cu_cp_executor = &ctrl_worker;
    return cucfg;
  }())
{
  cu_cp_logger.set_level(srslog::basic_levels::debug);
  test_logger.set_level(srslog::basic_levels::debug);
  test_logger.set_hex_dump_max_size(32);
  srslog::init();
}

cell_meas_manager_test::~cell_meas_manager_test()
{
  // flush logger after each test
  srslog::flush();
}

void cell_meas_manager_test::create_empty_manager()
{
  cell_meas_manager_cfg cfg = {};
  manager                   = std::make_unique<cell_meas_manager>(cfg, mobility_manager, ue_mng);
  ASSERT_NE(manager, nullptr);
}

void cell_meas_manager_test::create_default_manager()
{
  cell_meas_manager_cfg cfg;

  // Add 2 cells - one being the neighbor of the other one
  gnb_id_t         gnb_id{0x19b, 32};
  nr_cell_identity nci1 = nr_cell_identity::create(gnb_id, 0).value();
  nr_cell_identity nci2 = nr_cell_identity::create(gnb_id, 1).value();

  cell_meas_config cell_cfg;
  cell_cfg.serving_cell_cfg.gnb_id_bit_length = gnb_id.bit_length;
  cell_cfg.serving_cell_cfg.nci               = nci1;
  cell_cfg.serving_cell_cfg.pci               = 1;
  cell_cfg.periodic_report_cfg_id             = uint_to_report_cfg_id(1);

  neighbor_cell_meas_config ncell_meas_cfg;
  ncell_meas_cfg.nci = nci2;
  ncell_meas_cfg.report_cfg_ids.push_back(uint_to_report_cfg_id(2));
  cell_cfg.ncells.push_back(ncell_meas_cfg);

  cell_cfg.serving_cell_cfg.band.emplace()      = nr_band::n78;
  cell_cfg.serving_cell_cfg.ssb_arfcn.emplace() = 632628;
  cell_cfg.serving_cell_cfg.ssb_scs.emplace()   = subcarrier_spacing::kHz30;
  {
    rrc_ssb_mtc ssb_mtc;
    ssb_mtc.dur                                 = 1;
    ssb_mtc.periodicity_and_offset.periodicity  = rrc_periodicity_and_offset::periodicity_t::sf5;
    ssb_mtc.periodicity_and_offset.offset       = 0;
    cell_cfg.serving_cell_cfg.ssb_mtc.emplace() = ssb_mtc;
  }
  cfg.cells.emplace(cell_cfg.serving_cell_cfg.nci, cell_cfg);

  // Reuse config to setup config for next cell.
  cell_cfg.serving_cell_cfg.nci = nci2;

  cell_cfg.ncells.clear();
  ncell_meas_cfg.nci = nci1;
  ncell_meas_cfg.report_cfg_ids.clear();
  ncell_meas_cfg.report_cfg_ids.push_back(uint_to_report_cfg_id(2));
  cell_cfg.ncells.push_back(ncell_meas_cfg);

  cell_cfg.serving_cell_cfg.band.emplace()      = nr_band::n78;
  cell_cfg.serving_cell_cfg.ssb_arfcn.emplace() = 632128;
  cell_cfg.serving_cell_cfg.ssb_scs.emplace()   = subcarrier_spacing::kHz30;
  {
    rrc_ssb_mtc ssb_mtc;
    ssb_mtc.dur                                 = 1;
    ssb_mtc.periodicity_and_offset.periodicity  = rrc_periodicity_and_offset::periodicity_t::sf5;
    ssb_mtc.periodicity_and_offset.offset       = 0;
    cell_cfg.serving_cell_cfg.ssb_mtc.emplace() = ssb_mtc;
  }
  cfg.cells.emplace(cell_cfg.serving_cell_cfg.nci, cell_cfg);

  // Add periodic event.
  rrc_report_cfg_nr         periodic_report_cfg;
  rrc_periodical_report_cfg periodical_cfg;

  periodical_cfg.rs_type                = srs_cu_cp::rrc_nr_rs_type::ssb;
  periodical_cfg.report_interv          = 1024;
  periodical_cfg.report_amount          = -1;
  periodical_cfg.report_quant_cell.rsrp = true;
  periodical_cfg.report_quant_cell.rsrq = true;
  periodical_cfg.report_quant_cell.sinr = true;
  periodical_cfg.max_report_cells       = 4;

  periodic_report_cfg = periodical_cfg;
  cfg.report_config_ids.emplace(uint_to_report_cfg_id(1), periodic_report_cfg);

  // Add A3 event.
  rrc_report_cfg_nr     a3_report_cfg;
  rrc_event_trigger_cfg event_trigger_cfg = {};

  rrc_event_id event_a3;
  event_a3.id = rrc_event_id::event_id_t::a3;
  event_a3.meas_trigger_quant_thres_or_offset.emplace();
  event_a3.meas_trigger_quant_thres_or_offset.value().rsrp.emplace() = 6;
  event_a3.hysteresis                                                = 0;
  event_a3.time_to_trigger                                           = 100;
  event_a3.use_allowed_cell_list                                     = false;

  event_trigger_cfg.event_id = event_a3;

  event_trigger_cfg.rs_type                = srs_cu_cp::rrc_nr_rs_type::ssb;
  event_trigger_cfg.report_interv          = 1024;
  event_trigger_cfg.report_amount          = -1;
  event_trigger_cfg.report_quant_cell.rsrp = true;
  event_trigger_cfg.report_quant_cell.rsrq = true;
  event_trigger_cfg.report_quant_cell.sinr = true;
  event_trigger_cfg.max_report_cells       = 4;

  rrc_meas_report_quant report_quant_rs_idxes;
  report_quant_rs_idxes.rsrp              = true;
  report_quant_rs_idxes.rsrq              = true;
  report_quant_rs_idxes.sinr              = true;
  event_trigger_cfg.report_quant_rs_idxes = report_quant_rs_idxes;

  a3_report_cfg = event_trigger_cfg;
  cfg.report_config_ids.emplace(uint_to_report_cfg_id(2), a3_report_cfg);

  manager = std::make_unique<cell_meas_manager>(cfg, mobility_manager, ue_mng);
  ASSERT_NE(manager, nullptr);
}

void cell_meas_manager_test::create_manager_with_incomplete_cells_and_periodic_report_at_target_cell()
{
  cell_meas_manager_cfg cfg;

  // Add 2 cells - one being the neighbor of the other one
  gnb_id_t         gnb_id{0x19b, 32};
  nr_cell_identity nci1 = nr_cell_identity::create(gnb_id, 0).value();
  nr_cell_identity nci2 = nr_cell_identity::create(gnb_id, 1).value();

  cell_meas_config cell_cfg;
  cell_cfg.serving_cell_cfg.gnb_id_bit_length = gnb_id.bit_length;
  cell_cfg.serving_cell_cfg.nci               = nci1;
  cell_cfg.serving_cell_cfg.pci               = 1;

  neighbor_cell_meas_config ncell_meas_cfg;
  ncell_meas_cfg.nci = nci2;
  ncell_meas_cfg.report_cfg_ids.push_back(uint_to_report_cfg_id(2));
  cell_cfg.ncells.push_back(ncell_meas_cfg);
  cfg.cells.emplace(cell_cfg.serving_cell_cfg.nci, cell_cfg);

  // Reuse config to setup config for next cell.
  cell_cfg.serving_cell_cfg.nci   = nci2;
  cell_cfg.periodic_report_cfg_id = uint_to_report_cfg_id(1);

  cell_cfg.ncells.clear();
  ncell_meas_cfg.nci = nci1;
  ncell_meas_cfg.report_cfg_ids.clear();
  ncell_meas_cfg.report_cfg_ids.push_back(uint_to_report_cfg_id(2));
  cell_cfg.ncells.push_back(ncell_meas_cfg);
  cfg.cells.emplace(cell_cfg.serving_cell_cfg.nci, cell_cfg);

  // Add periodic event.
  rrc_report_cfg_nr         periodic_report_cfg;
  rrc_periodical_report_cfg periodical_cfg;

  periodical_cfg.rs_type                = srs_cu_cp::rrc_nr_rs_type::ssb;
  periodical_cfg.report_interv          = 1024;
  periodical_cfg.report_amount          = -1;
  periodical_cfg.report_quant_cell.rsrp = true;
  periodical_cfg.report_quant_cell.rsrq = true;
  periodical_cfg.report_quant_cell.sinr = true;
  periodical_cfg.max_report_cells       = 4;

  periodic_report_cfg = periodical_cfg;
  cfg.report_config_ids.emplace(uint_to_report_cfg_id(1), periodic_report_cfg);

  // Add A3 event.
  rrc_report_cfg_nr     a3_report_cfg;
  rrc_event_trigger_cfg event_trigger_cfg = {};

  rrc_event_id event_a3;
  event_a3.id = rrc_event_id::event_id_t::a3;
  event_a3.meas_trigger_quant_thres_or_offset.emplace();
  event_a3.meas_trigger_quant_thres_or_offset.value().rsrp.emplace() = 6;
  event_a3.hysteresis                                                = 0;
  event_a3.time_to_trigger                                           = 100;
  event_a3.use_allowed_cell_list                                     = false;

  event_trigger_cfg.event_id = event_a3;

  event_trigger_cfg.rs_type                = srs_cu_cp::rrc_nr_rs_type::ssb;
  event_trigger_cfg.report_interv          = 1024;
  event_trigger_cfg.report_amount          = -1;
  event_trigger_cfg.report_quant_cell.rsrp = true;
  event_trigger_cfg.report_quant_cell.rsrq = true;
  event_trigger_cfg.report_quant_cell.sinr = true;
  event_trigger_cfg.max_report_cells       = 4;

  rrc_meas_report_quant report_quant_rs_idxes;
  report_quant_rs_idxes.rsrp              = true;
  report_quant_rs_idxes.rsrq              = true;
  report_quant_rs_idxes.sinr              = true;
  event_trigger_cfg.report_quant_rs_idxes = report_quant_rs_idxes;

  a3_report_cfg = event_trigger_cfg;
  cfg.report_config_ids.emplace(uint_to_report_cfg_id(2), a3_report_cfg);

  manager = std::make_unique<cell_meas_manager>(cfg, mobility_manager, ue_mng);
  ASSERT_NE(manager, nullptr);
}

void cell_meas_manager_test::create_manager_without_ncells_and_periodic_report()
{
  cell_meas_manager_cfg cfg;

  // Add serving cell
  gnb_id_t         gnb_id{411, 32};
  cell_meas_config cell_cfg;
  cell_cfg.serving_cell_cfg.gnb_id_bit_length = gnb_id.bit_length;
  cell_cfg.serving_cell_cfg.nci               = nr_cell_identity::create(gnb_id, 0).value();

  cell_cfg.serving_cell_cfg.band.emplace()      = nr_band::n78;
  cell_cfg.serving_cell_cfg.ssb_arfcn.emplace() = 632628;
  cell_cfg.serving_cell_cfg.ssb_scs.emplace()   = subcarrier_spacing::kHz30;
  {
    rrc_ssb_mtc ssb_mtc;
    ssb_mtc.dur                                 = 1;
    ssb_mtc.periodicity_and_offset.periodicity  = rrc_periodicity_and_offset::periodicity_t::sf5;
    ssb_mtc.periodicity_and_offset.offset       = 0;
    cell_cfg.serving_cell_cfg.ssb_mtc.emplace() = ssb_mtc;
  }
  cfg.cells.emplace(cell_cfg.serving_cell_cfg.nci, cell_cfg);

  // Add A3 event.
  rrc_report_cfg_nr     a3_report_cfg;
  rrc_event_trigger_cfg event_trigger_cfg = {};

  rrc_event_id event_a3;
  event_a3.id = rrc_event_id::event_id_t::a3;
  event_a3.meas_trigger_quant_thres_or_offset.emplace();
  event_a3.meas_trigger_quant_thres_or_offset.value().rsrp.emplace() = 6;
  event_a3.hysteresis                                                = 0;
  event_a3.time_to_trigger                                           = 100;
  event_a3.use_allowed_cell_list                                     = false;

  event_trigger_cfg.event_id = event_a3;

  event_trigger_cfg.rs_type                = srs_cu_cp::rrc_nr_rs_type::ssb;
  event_trigger_cfg.report_interv          = 1024;
  event_trigger_cfg.report_amount          = -1;
  event_trigger_cfg.report_quant_cell.rsrp = true;
  event_trigger_cfg.report_quant_cell.rsrq = true;
  event_trigger_cfg.report_quant_cell.sinr = true;
  event_trigger_cfg.max_report_cells       = 4;

  rrc_meas_report_quant report_quant_rs_idxes;
  report_quant_rs_idxes.rsrp              = true;
  report_quant_rs_idxes.rsrq              = true;
  report_quant_rs_idxes.sinr              = true;
  event_trigger_cfg.report_quant_rs_idxes = report_quant_rs_idxes;

  a3_report_cfg = event_trigger_cfg = {};
  cfg.report_config_ids.emplace(uint_to_report_cfg_id(1), a3_report_cfg);

  manager = std::make_unique<cell_meas_manager>(cfg, mobility_manager, ue_mng);
  ASSERT_NE(manager, nullptr);
}

void cell_meas_manager_test::check_default_meas_cfg(const std::optional<rrc_meas_cfg>& meas_cfg,
                                                    meas_obj_id_t                      meas_obj_id)
{
  ASSERT_TRUE(meas_cfg.has_value());
  ASSERT_EQ(meas_cfg.value().meas_obj_to_add_mod_list.size(), 2);
  ASSERT_EQ((unsigned)meas_cfg.value().meas_obj_to_add_mod_list.begin()->meas_obj_id, meas_obj_id_to_uint(meas_obj_id));
  // TODO: Add checks for more values
}

void cell_meas_manager_test::verify_meas_cfg(const std::optional<rrc_meas_cfg>& meas_cfg)
{
  // Performs sanity check on the config without making any assumptions.
  ASSERT_TRUE(meas_cfg.has_value());

  std::vector<meas_id_t> used_meas_ids;

  // Make sure sure all elements referenced in MeasIds exist.
  for (const auto& meas_item : meas_cfg.value().meas_id_to_add_mod_list) {
    // Check ID is not already used.
    ASSERT_EQ(std::find(used_meas_ids.begin(), used_meas_ids.end(), meas_item.meas_id), used_meas_ids.end());
    used_meas_ids.push_back(meas_item.meas_id);

    // Report config must be valid/present in meas_cfg.
    auto report_it = std::find_if(
        begin(meas_cfg.value().report_cfg_to_add_mod_list),
        end(meas_cfg.value().report_cfg_to_add_mod_list),
        [meas_item](const rrc_report_cfg_to_add_mod& x) { return x.report_cfg_id == meas_item.report_cfg_id; });
    ASSERT_NE(report_it, meas_cfg.value().report_cfg_to_add_mod_list.end());

    auto meas_obj_it =
        std::find_if(begin(meas_cfg.value().meas_obj_to_add_mod_list),
                     end(meas_cfg.value().meas_obj_to_add_mod_list),
                     [meas_item](const rrc_meas_obj_to_add_mod& x) { return x.meas_obj_id == meas_item.meas_obj_id; });
    ASSERT_NE(meas_obj_it, meas_cfg.value().meas_obj_to_add_mod_list.end());

    // check meas object is unique
    for (const auto& meas_obj : meas_cfg.value().meas_obj_to_add_mod_list) {
      for (const auto& meas_obj_2 : meas_cfg.value().meas_obj_to_add_mod_list) {
        if (meas_obj.meas_obj_id == meas_obj_2.meas_obj_id) {
          continue;
        }
        if (meas_obj.meas_obj_nr.has_value() && meas_obj_2.meas_obj_nr.has_value()) {
          ASSERT_FALSE(is_duplicate(meas_obj.meas_obj_nr.value(), meas_obj_2.meas_obj_nr.value()));
        }
      }
    }
  }
}

void cell_meas_manager_test::verify_empty_meas_cfg(const std::optional<rrc_meas_cfg>& meas_cfg)
{
  ASSERT_FALSE(meas_cfg.has_value());
}
