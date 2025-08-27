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

#include "../common/e2ap_asn1_utils.h"
#include "srsran/asn1/e2ap/e2ap.h"
#include "srsran/asn1/e2sm/e2sm_kpm_ies.h"
#include "srsran/e2/e2.h"
#include "srsran/e2/e2_event_manager.h"
#include "srsran/e2/e2sm/e2sm.h"
#include "srsran/e2/subscription/e2_subscription.h"

namespace srsran {

/// E2 INDICATION procedure will be used to send the measurement data to the RIC in response to the E2 SUBSCRIPTION
/// REQUEST, it will execute periodically until the E2 SUBSCRIPTION DELETE REQUEST is received.
class e2_indication_procedure
{
public:
  e2_indication_procedure(e2_message_notifier&    notif_,
                          e2_event_manager&       ev_mng_,
                          e2_subscription_info_t& subscription_info_,
                          srslog::basic_logger&   logger_);

  void operator()(coro_context<eager_async_task<void>>& ctx);

private:
  /// Send E2 INDICATION message to the RIC.
  void send_e2_indication(e2_indication_message& e2_ind);

  e2_message_notifier&               notifier;
  e2_event_manager&                  ev_mng;
  e2_subscription_info_t&            subscription;
  srslog::basic_logger&              logger;
  std::vector<e2_indication_message> e2_ind_vec;
  asn1::e2sm::e2sm_kpm_ind_hdr_s     ric_ind_header;
  asn1::e2sm::e2sm_kpm_ind_msg_s     ric_ind_message;

  protocol_transaction_outcome_observer<asn1::e2ap::ric_sub_delete_request_s> transaction_sink;

  std::chrono::seconds time_to_wait{1};
  bool                 running = true;
};

} // namespace srsran
