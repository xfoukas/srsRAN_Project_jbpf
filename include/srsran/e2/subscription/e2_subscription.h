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

#include "srsran/asn1/asn1_utils.h"
#include "srsran/asn1/e2ap/e2ap.h"
#include "srsran/asn1/e2sm/e2sm_kpm_ies.h"
#include "srsran/e2/e2.h"
#include "srsran/e2/e2sm/e2sm.h"

namespace srsran {

/// This defines the RIC action struct.
struct ric_action_t {
  byte_buffer                          action_definition;
  uint16_t                             ric_action_id;
  asn1::e2ap::ric_action_type_e        ric_action_type;
  std::unique_ptr<e2sm_report_service> report_service;
};

/// Here we define a subscription struct.
struct e2_subscription_info_t {
  asn1::e2ap::ric_request_id_s request_id;
  uint16_t                     ran_function_id;
  std::vector<ric_action_t>    action_list;
  uint64_t                     report_period;
};

struct e2_subscription_t {
  e2_subscription_info_t subscription_info;
  eager_async_task<void> indication_task;
};

class e2_subscription_proc
{
public:
  virtual ~e2_subscription_proc() = default;
  /// \brief Handle the incoming subscription message.
  virtual e2_subscribe_reponse_message handle_subscription_setup(const asn1::e2ap::ric_sub_request_s& msg) = 0;
  /// \brief Handle the incoming subscription delete message.
  virtual e2_subscribe_delete_response_message
  handle_subscription_delete(const asn1::e2ap::ric_sub_delete_request_s& msg) = 0;
  /// \brief start the subscription request
  virtual void start_subscription(const asn1::e2ap::ric_request_id_s& ric_request_id,
                                  uint16_t                            ran_func_id,
                                  e2_event_manager&                   ev_mng,
                                  e2_message_notifier&                tx_pdu_notifier) = 0;
  /// \brief void stop the subscription request
  virtual void stop_subscription(const asn1::e2ap::ric_request_id_s&         ric_request_id,
                                 e2_event_manager&                           ev_mng,
                                 const asn1::e2ap::ric_sub_delete_request_s& msg) = 0;
};

class e2_subscriber_mgmt
{
public:
  virtual ~e2_subscriber_mgmt() = default;
  /// \brief  adds e2sm service to the subscriber
  /// \param oid The oid of the e2sm service
  /// \param e2sm_iface pointer to the e2sm interface
  virtual void add_e2sm_service(std::string oid, std::unique_ptr<e2sm_interface> e2sm_iface) = 0;

  virtual e2sm_interface* get_e2sm_interface(const std::string oid) = 0;

  virtual void add_ran_function_oid(uint16_t ran_func_id, std::string oid) = 0;

  virtual void stop() = 0;
};

class e2_subscription_manager : public e2_subscription_proc, public e2_subscriber_mgmt
{
public:
  virtual ~e2_subscription_manager() = default;
};

} // namespace srsran
