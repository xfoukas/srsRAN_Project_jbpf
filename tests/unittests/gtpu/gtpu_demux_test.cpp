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

#include "gtpu_test_shared.h"
#include "srsran/gtpu/gtpu_demux.h"
#include "srsran/gtpu/gtpu_demux_factory.h"
#include "srsran/srslog/srslog.h"
#include "srsran/support/executors/manual_task_worker.h"
#include <gtest/gtest.h>
#include <sys/socket.h>

using namespace srsran;

class gtpu_tunnel_rx_upper_dummy : public gtpu_tunnel_common_rx_upper_layer_interface
{
public:
  void handle_pdu(byte_buffer pdu, const sockaddr_storage& src_addr) final { last_rx = std::move(pdu); }

  byte_buffer last_rx;
};

/// Fixture class for GTPU demux tests
class gtpu_demux_test : public ::testing::Test
{
protected:
  void SetUp() override
  {
    srslog::fetch_basic_logger("TEST").set_level(srslog::basic_levels::debug);
    srslog::init();

    gtpu_tunnel = std::make_unique<gtpu_tunnel_rx_upper_dummy>();

    // create DUT object
    gtpu_demux_creation_request msg = {};
    msg.gtpu_pcap                   = &dummy_pcap;
    dut                             = create_gtpu_demux(msg);
  }

  void TearDown() override
  {
    // flush logger after each test
    srslog::flush();
  }

  std::unique_ptr<gtpu_tunnel_rx_upper_dummy> gtpu_tunnel;
  manual_task_worker                          teid_worker{128};
  null_dlt_pcap                               dummy_pcap;

  std::unique_ptr<gtpu_demux> dut;
  srslog::basic_logger&       test_logger = srslog::fetch_basic_logger("TEST", false);
};

//// GTPU demux tesst
TEST_F(gtpu_demux_test, when_tunnel_not_registered_pdu_is_dropped)
{
  sockaddr_storage src_addr = {};
  byte_buffer      pdu      = byte_buffer::create(gtpu_ping_vec_teid_1).value();

  dut->handle_pdu(std::move(pdu), src_addr);
  teid_worker.run_pending_tasks();

  ASSERT_EQ(gtpu_tunnel->last_rx.length(), 0);
}

TEST_F(gtpu_demux_test, when_tunnel_registered_pdu_is_forwarded)
{
  sockaddr_storage src_addr = {};
  byte_buffer      pdu      = byte_buffer::create(gtpu_ping_vec_teid_1).value();
  auto             queue    = dut->add_tunnel(gtpu_teid_t{0x1}, teid_worker, gtpu_tunnel.get());

  dut->handle_pdu(std::move(pdu), src_addr);
  teid_worker.run_pending_tasks();

  ASSERT_EQ(gtpu_tunnel->last_rx.length(), sizeof(gtpu_ping_vec_teid_1));
}

TEST_F(gtpu_demux_test, when_tunnel_was_removed_pdu_is_dropped)
{
  sockaddr_storage src_addr = {};
  byte_buffer      pdu      = byte_buffer::create(gtpu_ping_vec_teid_1).value();
  auto             queue    = dut->add_tunnel(gtpu_teid_t{0x1}, teid_worker, gtpu_tunnel.get());
  dut->remove_tunnel(gtpu_teid_t{0x1});

  // pass and handle PDU when tunnel was already removed
  dut->handle_pdu(std::move(pdu), src_addr);
  teid_worker.run_pending_tasks();

  ASSERT_EQ(gtpu_tunnel->last_rx.length(), 0);
}

TEST_F(gtpu_demux_test, when_tunnel_is_being_removed_pdu_is_dropped)
{
  sockaddr_storage src_addr = {};
  byte_buffer      pdu      = byte_buffer::create(gtpu_ping_vec_teid_1).value();
  auto             queue    = dut->add_tunnel(gtpu_teid_t{0x1}, teid_worker, gtpu_tunnel.get());

  // pass PDU while tunnel is present
  dut->handle_pdu(std::move(pdu), src_addr);

  // remove tunnel while the PDU handler is queued for execution
  dut->remove_tunnel(gtpu_teid_t{0x1});

  // handle PDU when tunnel was removed; the PDU shall be dropped
  teid_worker.run_pending_tasks();

  ASSERT_EQ(gtpu_tunnel->last_rx.length(), 0);
}

TEST_F(gtpu_demux_test, when_different_tunnel_registered_pdu_is_dropped)
{
  sockaddr_storage src_addr = {};
  byte_buffer      pdu      = byte_buffer::create(gtpu_ping_vec_teid_2).value();
  auto             queue    = dut->add_tunnel(gtpu_teid_t{0x1}, teid_worker, gtpu_tunnel.get());

  dut->handle_pdu(std::move(pdu), src_addr);
  teid_worker.run_pending_tasks();

  ASSERT_EQ(gtpu_tunnel->last_rx.length(), 0);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
