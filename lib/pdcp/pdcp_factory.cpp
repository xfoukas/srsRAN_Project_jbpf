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

#include "srsran/pdcp/pdcp_factory.h"
#include "pdcp_entity_impl.h"

/// Notice this would be the only place were we include concrete class implementation files.

using namespace srsran;

std::unique_ptr<pdcp_entity> srsran::create_pdcp_entity(pdcp_entity_creation_message& msg)
{
  return std::make_unique<pdcp_entity_impl>(msg.ue_index,
                                            msg.rb_id,
                                            msg.config,
                                            *msg.tx_lower,
                                            *msg.tx_upper_cn,
                                            *msg.rx_upper_dn,
                                            *msg.rx_upper_cn,
                                            msg.ue_dl_timer_factory,
                                            msg.ue_ul_timer_factory,
                                            msg.ue_ctrl_timer_factory,
                                            *msg.ue_dl_executor,
                                            *msg.ue_ul_executor,
                                            *msg.ue_ctrl_executor,
                                            *msg.crypto_executor,
                                            msg.max_nof_crypto_workers);
}
