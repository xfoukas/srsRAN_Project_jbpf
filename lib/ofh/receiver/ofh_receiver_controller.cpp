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

#include "ofh_receiver_controller.h"
#include "srsran/ofh/ethernet/ethernet_controller.h"

using namespace srsran;
using namespace ofh;

void receiver_controller::start()
{
  msg_receiver.get_ethernet_receiver().get_operation_controller().start(msg_receiver);
}

void receiver_controller::stop()
{
  msg_receiver.get_ethernet_receiver().get_operation_controller().stop();
}
