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

#include "srsran/fapi/messages/ul_tti_request.h"
#include "srsran/phy/upper/uplink_pdu_slot_repository.h"

namespace srsran {
namespace fapi_adaptor {

class uci_part2_correspondence_repository;

/// Helper function that converts a PUSCH FAPI PDU into a PUSCH uplink slot PDU.
void convert_pusch_fapi_to_phy(uplink_pdu_slot_repository::pusch_pdu& pdu,
                               const fapi::ul_pusch_pdu&              fapi_pdu,
                               uint16_t                               sfn,
                               uint16_t                               slot,
                               uint16_t                               num_rx_ant,
                               uci_part2_correspondence_repository&   part2_repo);

} // namespace fapi_adaptor
} // namespace srsran
