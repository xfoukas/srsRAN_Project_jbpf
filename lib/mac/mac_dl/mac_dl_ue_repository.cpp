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

#include "mac_dl_ue_repository.h"
#include "srsran/ran/pdsch/pdsch_constants.h"
#include "srsran/support/timers.h"

using namespace srsran;

mac_dl_ue_context::mac_dl_ue_context(const mac_ue_create_request& req) : ue_index(req.ue_index)
{
  // Store DL logical channel notifiers.
  addmod_logical_channels(req.bearers);

  // Store UL-CCCH
  if (req.ul_ccch_msg != nullptr) {
    // If the Msg3 contained an UL-CCCH message, store it for Contention Resolution.
    srsran_assert(req.ul_ccch_msg->length() >= UE_CON_RES_ID_LEN,
                  "Invalid UL-CCCH message length ({} < 6)",
                  req.ul_ccch_msg->length());
    std::copy(req.ul_ccch_msg->begin(), req.ul_ccch_msg->begin() + UE_CON_RES_ID_LEN, msg3_subpdu.begin());
  }
}

void mac_dl_ue_context::addmod_logical_channels(span<const mac_logical_channel_config> dl_logical_channels)
{
  for (const mac_logical_channel_config& lc : dl_logical_channels) {
    dl_bearers.insert(lc.lcid, lc.dl_bearer);
  }
}

void mac_dl_ue_context::remove_logical_channels(const bounded_bitset<MAX_NOF_RB_LCIDS>& lcids_to_remove)
{
  lcids_to_remove.for_each(0, lcids_to_remove.size(), [this](size_t pos) { dl_bearers.erase(uint_to_lcid(pos)); });
}

// ///////////////////////

mac_dl_ue_repository::mac_dl_ue_repository(du_rnti_table& rnti_table_) : rnti_table(rnti_table_) {}

bool mac_dl_ue_repository::add_ue(mac_dl_ue_context ue_to_add)
{
  const du_ue_index_t ue_index = ue_to_add.get_ue_index();

  // Register the UE in the repository.
  if (ue_db.contains(ue_index)) {
    return false;
  }
  ue_db.emplace(ue_index, std::move(ue_to_add));
  return true;
}

bool mac_dl_ue_repository::remove_ue(du_ue_index_t ue_index)
{
  // Erase UE from the repository.
  return ue_db.erase(ue_index);
}

bool mac_dl_ue_repository::addmod_bearers(du_ue_index_t                          ue_index,
                                          span<const mac_logical_channel_config> dl_logical_channels)
{
  if (not ue_db.contains(ue_index)) {
    return false;
  }
  auto& u = ue_db[ue_index];

  u.addmod_logical_channels(dl_logical_channels);
  return true;
}

bool mac_dl_ue_repository::remove_bearers(du_ue_index_t ue_index, const bounded_bitset<MAX_NOF_RB_LCIDS>& lcids_to_rem)
{
  if (not ue_db.contains(ue_index)) {
    return false;
  }
  auto& u = ue_db[ue_index];

  u.remove_logical_channels(lcids_to_rem);
  return true;
}

ue_con_res_id_t mac_dl_ue_repository::get_con_res_id(rnti_t rnti)
{
  const du_ue_index_t ue_index = rnti_table[rnti];
  if (not ue_db.contains(ue_index)) {
    return {};
  }
  return ue_db[ue_index].get_con_res_id();
}
