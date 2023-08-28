/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "ue_link_adaptation_controller.h"
#include "../support/mcs_calculator.h"
#include "../support/sch_pdu_builder.h"

using namespace srsran;

ue_link_adaptation_controller::ue_link_adaptation_controller(const cell_configuration&       cell_cfg_,
                                                             const ue_channel_state_manager& ue_channel_state) :
  cell_cfg(cell_cfg_), ue_ch_st(ue_channel_state)
{
}

void ue_link_adaptation_controller::handle_dl_ack_info(mac_harq_ack_report_status ack_value,
                                                       sch_mcs_index              used_mcs,
                                                       pdsch_mcs_table            mcs_table)
{
  static constexpr unsigned MAX_CQI = 15;

  if (ack_value == mac_harq_ack_report_status::dtx or not dl_olla.has_value()) {
    return;
  }

  const sch_mcs_index                 max_mcs = map_cqi_to_mcs(MAX_CQI, mcs_table).value();
  const interval<sch_mcs_index, true> mcs_bounds{cell_cfg.expert_cfg.ue.dl_mcs.start(),
                                                 std::min(cell_cfg.expert_cfg.ue.dl_mcs.stop(), max_mcs)};

  dl_olla->update(ack_value == mac_harq_ack_report_status::ack, used_mcs, mcs_bounds);
}

void ue_link_adaptation_controller::handle_ul_crc_info(bool crc, sch_mcs_index used_mcs, pusch_mcs_table mcs_table)
{
  if (not ul_olla.has_value()) {
    return;
  }

  const interval<sch_mcs_index, true> mcs_bounds{
      cell_cfg.expert_cfg.ue.dl_mcs.start(), std::min(cell_cfg.expert_cfg.ue.dl_mcs.stop(), get_max_mcs_ul(mcs_table))};
  ul_olla->update(crc, used_mcs, mcs_bounds);
}

cqi_value ue_link_adaptation_controller::get_effective_cqi() const
{
  if (not dl_olla.has_value()) {
    // For the case of no DL outer loop link adaptation.
    return ue_ch_st.get_wideband_cqi();
  }

  float cqi = ue_ch_st.get_wideband_cqi().to_uint();
  cqi += dl_olla.value().offset_db();
  return cqi_value{std::min(static_cast<uint8_t>(std::max(cqi, 0.0f)), cqi_value::max())};
}

float ue_link_adaptation_controller::get_effective_snr() const
{
  return ue_ch_st.get_pusch_snr() + (ul_olla.has_value() ? ul_olla.value().offset_db() : 0.0f);
}

optional<sch_mcs_index> ue_link_adaptation_controller::calculate_dl_mcs(const pdsch_config_params& pdsch_cfg) const
{
  if (cell_cfg.expert_cfg.ue.dl_mcs.length() == 0) {
    // Fixed MCS.
    return cell_cfg.expert_cfg.ue.dl_mcs.start();
  }

  // Derive MCS using the combination of CQI + outer loop link adaptation.
  optional<sch_mcs_index> mcs = map_cqi_to_mcs(get_effective_cqi().to_uint(), pdsch_cfg.mcs_table);

  if (not mcs.has_value()) {
    return nullopt;
  }

  mcs = std::min(std::max(mcs.value(), cell_cfg.expert_cfg.ue.dl_mcs.start()), cell_cfg.expert_cfg.ue.dl_mcs.stop());

  return mcs;
}

sch_mcs_index ue_link_adaptation_controller::calculate_ul_mcs(const pusch_config_params& pusch_cfg) const
{
  if (cell_cfg.expert_cfg.ue.ul_mcs.length() == 0) {
    // Fixed MCS.
    return cell_cfg.expert_cfg.ue.ul_mcs.start();
  }

  // Derive MCS using the combination of estimated UL SNR + outer loop link adaptation.
  sch_mcs_index mcs = map_snr_to_mcs_ul(get_effective_snr(), pusch_cfg.mcs_table);
  mcs = std::min(std::max(mcs, cell_cfg.expert_cfg.ue.ul_mcs.start()), cell_cfg.expert_cfg.ue.ul_mcs.stop());

  return mcs;
}
