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

#include "ngap_test_helpers.h"
#include "srsran/ngap/ngap_setup.h"
#include "srsran/support/async/async_test_utils.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace srs_cu_cp;

/// Test the ng setup procedure timeout
TEST_F(ngap_test, when_ng_setup_procedure_times_out_then_amf_not_connected)
{
  // Action 1: Launch NG setup procedure
  test_logger.info("Launch ng setup request procedure...");
  unsigned                                 max_setup_retries = 1;
  async_task<ngap_ng_setup_result>         t                 = ngap->handle_ng_setup_request(max_setup_retries);
  lazy_task_launcher<ngap_ng_setup_result> t_launcher(t);

  // Status: AMF received NG Setup Request.
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.init_msg().value.type().value,
            asn1::ngap::ngap_elem_procs_o::init_msg_c::types_opts::ng_setup_request);

  // Status: Fail NG setup procedure (AMF doesn't respond).
  for (unsigned msec_elapsed = 0; msec_elapsed < 5000; ++msec_elapsed) {
    ASSERT_FALSE(t.ready());
    this->tick();
  }

  ASSERT_TRUE(t.ready());
  ASSERT_TRUE(std::holds_alternative<ngap_ng_setup_failure>(t.get()));
}

/// Test successful ng setup procedure
TEST_F(ngap_test, when_ng_setup_response_received_then_amf_connected)
{
  // Action 1: Launch NG setup procedure
  test_logger.info("Launch ng setup request procedure...");
  async_task<ngap_ng_setup_result>         t = ngap->handle_ng_setup_request(1);
  lazy_task_launcher<ngap_ng_setup_result> t_launcher(t);

  // Status: AMF received NG Setup Request.
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.init_msg().value.type().value,
            asn1::ngap::ngap_elem_procs_o::init_msg_c::types_opts::ng_setup_request);

  // Status: Procedure not yet ready.
  ASSERT_FALSE(t.ready());

  // Action 2: NG setup response received.
  ngap_message ng_setup_response = generate_ng_setup_response();
  test_logger.info("Injecting NGSetupResponse");
  ngap->handle_message(ng_setup_response);

  ASSERT_TRUE(t.ready());
  ASSERT_TRUE(std::holds_alternative<ngap_ng_setup_response>(t.get()));
  ASSERT_EQ(std::get<ngap_ng_setup_response>(t.get()).amf_name, "open5gs-amf0");
}

/// Test unsuccessful ng setup procedure with time to wait and successful retry
TEST_F(ngap_test, when_ng_setup_failure_with_time_to_wait_received_then_retry_with_success)
{
  // Action 1: Launch NG setup procedure
  test_logger.info("Launch ng setup request procedure...");
  async_task<ngap_ng_setup_result>         t = ngap->handle_ng_setup_request(1);
  lazy_task_launcher<ngap_ng_setup_result> t_launcher(t);

  // Status: AMF received NG Setup Request.
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.init_msg().value.type().value,
            asn1::ngap::ngap_elem_procs_o::init_msg_c::types_opts::ng_setup_request);

  // Status: Procedure not yet ready.
  ASSERT_FALSE(t.ready());

  // Action 2: NG setup failure with time to wait received.
  ngap_message ng_setup_failure = generate_ng_setup_failure_with_time_to_wait(asn1::ngap::time_to_wait_opts::v10s);
  test_logger.info("Injecting NGSetupFailure with time to wait");
  ngap->handle_message(ng_setup_failure);

  // Status: AMF does not receive new NG Setup Request until time-to-wait has ended.
  for (unsigned msec_elapsed = 0; msec_elapsed < 10000; ++msec_elapsed) {
    ASSERT_FALSE(t.ready());
    this->tick();
  }

  // Status: AMF received NG Setup Request again.
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.init_msg().value.type().value,
            asn1::ngap::ngap_elem_procs_o::init_msg_c::types_opts::ng_setup_request);

  // Successful outcome after reinitiated NG Setup
  ngap_message ng_setup_response = generate_ng_setup_response();
  test_logger.info("Injecting NGSetupResponse");
  ngap->handle_message(ng_setup_response);

  ASSERT_TRUE(t.ready());
  ASSERT_TRUE(std::holds_alternative<ngap_ng_setup_response>(t.get()));
  ASSERT_EQ(std::get<ngap_ng_setup_response>(t.get()).amf_name, "open5gs-amf0");
}

/// Test unsuccessful ng setup procedure with time to wait and unsuccessful retry
TEST_F(ngap_test, when_ng_setup_failure_with_time_to_wait_received_then_retry_without_success)
{
  // Action 1: Launch NG setup procedure
  test_logger.info("Launch ng setup request procedure...");
  async_task<ngap_ng_setup_result>         t = ngap->handle_ng_setup_request(1);
  lazy_task_launcher<ngap_ng_setup_result> t_launcher(t);

  // Status: AMF received NG Setup Request.
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.init_msg().value.type().value,
            asn1::ngap::ngap_elem_procs_o::init_msg_c::types_opts::ng_setup_request);

  // Status: Procedure not yet ready.
  ASSERT_FALSE(t.ready());

  // Action 2: NG setup failure with time to wait received.
  ngap_message ng_setup_failure = generate_ng_setup_failure_with_time_to_wait(asn1::ngap::time_to_wait_opts::v10s);
  test_logger.info("Injecting NGSetupFailure with time to wait");
  ngap->handle_message(ng_setup_failure);

  // Status: AMF received NG Setup Request again.
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.init_msg().value.type().value,
            asn1::ngap::ngap_elem_procs_o::init_msg_c::types_opts::ng_setup_request);

  // Status: AMF does not receive new NG Setup Request until time-to-wait has ended.
  for (unsigned msec_elapsed = 0; msec_elapsed < 10000; ++msec_elapsed) {
    ASSERT_FALSE(t.ready());
    this->tick();
  }

  // Unsuccessful outcome after reinitiated NG Setup
  ng_setup_failure = generate_ng_setup_failure();
  test_logger.info("Injecting NGSetupFailure");
  ngap->handle_message(ng_setup_failure);

  ASSERT_TRUE(t.ready());
  ASSERT_TRUE(std::holds_alternative<ngap_ng_setup_failure>(t.get()));
}

/// Test the ng setup procedure
TEST_F(ngap_test, when_retry_limit_reached_then_amf_not_connected)
{
  // Action 1: Launch NG setup procedure
  test_logger.info("Launch ng setup request procedure...");
  unsigned                                 max_setup_retries = 1;
  async_task<ngap_ng_setup_result>         t                 = ngap->handle_ng_setup_request(max_setup_retries);
  lazy_task_launcher<ngap_ng_setup_result> t_launcher(t);

  // Status: AMF received NG Setup Request.
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.init_msg().value.type().value,
            asn1::ngap::ngap_elem_procs_o::init_msg_c::types_opts::ng_setup_request);

  // Action 2: NG setup failure received.
  ngap_message ng_setup_response_msg = generate_ng_setup_failure_with_time_to_wait(asn1::ngap::time_to_wait_opts::v10s);
  ngap->handle_message(ng_setup_response_msg);

  for (unsigned i = 0; i < max_setup_retries; i++) {
    // Status: AMF does not receive new NG Setup Request until time-to-wait has ended.
    for (unsigned msec_elapsed = 0; msec_elapsed < 10000; ++msec_elapsed) {
      ASSERT_FALSE(t.ready());
      this->tick();
    }
    ngap->handle_message(ng_setup_response_msg);
  }

  ASSERT_TRUE(t.ready());
  ASSERT_TRUE(std::holds_alternative<ngap_ng_setup_failure>(t.get()));
}

/// Test the ng setup procedure
TEST_F(ngap_test, when_failure_is_misconfiguration_no_retries_are_done)
{
  // Action 1: Launch NG setup procedure
  test_logger.info("Launch ng setup request procedure...");
  unsigned                                 max_setup_retries = 10;
  async_task<ngap_ng_setup_result>         t                 = ngap->handle_ng_setup_request(max_setup_retries);
  lazy_task_launcher<ngap_ng_setup_result> t_launcher(t);

  // Status: AMF received NG Setup Request.
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.type().value, asn1::ngap::ngap_pdu_c::types_opts::init_msg);
  ASSERT_EQ(n2_gw.last_ngap_msgs.back().pdu.init_msg().value.type().value,
            asn1::ngap::ngap_elem_procs_o::init_msg_c::types_opts::ng_setup_request);

  // Action 2: NG setup failure received.
  ngap_message ng_setup_response_msg = generate_ng_setup_failure_with_bad_plmn(asn1::ngap::time_to_wait_opts::v10s);
  ngap->handle_message(ng_setup_response_msg);
  ASSERT_TRUE(t.ready());
}
