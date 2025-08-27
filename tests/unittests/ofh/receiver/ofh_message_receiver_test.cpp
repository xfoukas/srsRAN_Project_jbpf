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

#include "../../../../lib/ofh/receiver/ofh_closed_rx_window_handler.h"
#include "../../../../lib/ofh/receiver/ofh_message_receiver_impl.h"
#include "../../../../lib/ofh/receiver/ofh_rx_window_checker.h"
#include "../../../../lib/ofh/receiver/ofh_sequence_id_checker_dummy_impl.h"
#include "../../support/task_executor_test_doubles.h"
#include "../compression/ofh_iq_decompressor_test_doubles.h"
#include "srsran/ofh/ethernet/ethernet_controller.h"
#include "srsran/ofh/ethernet/ethernet_receiver_metrics_collector.h"
#include "srsran/ofh/ethernet/ethernet_unique_buffer.h"
#include "srsran/ofh/ofh_factories.h"
#include "srsran/phy/support/shared_resource_grid.h"
#include <gtest/gtest.h>

using namespace srsran;
using namespace ofh;
using namespace srsran::ofh::testing;

namespace {

/// Dummy User-Plane received symbol notifier.
class dummy_uplane_rx_symbol_notifier : public uplane_rx_symbol_notifier
{
public:
  void on_new_uplink_symbol(const uplane_rx_symbol_context& context, shared_resource_grid grid) override {}
  void on_new_prach_window_data(const prach_buffer_context& context, const prach_buffer& buffer) override {}
};

/// Dummy Ethernet receive buffer.
class dummy_eth_rx_buffer : public ether::rx_buffer
{
public:
  explicit dummy_eth_rx_buffer(std::vector<uint8_t>&& init_values) { buffer = init_values; }

  span<const uint8_t> data() const override { return buffer; }

private:
  std::vector<uint8_t> buffer;
};
/// Data flow User-Plane uplink PRACH spy.
class data_flow_uplane_uplink_prach_spy : public data_flow_uplane_uplink_prach
{
  bool decode_function_called = false;

public:
  // See interface for documentation.
  void decode_type1_message(unsigned eaxc, span<const uint8_t> msg) override { decode_function_called = true; }

  /// Returns true if the decode_type1_message function has been called, otherwise false.
  bool has_decode_function_been_called() const { return decode_function_called; }
};

/// Data flow User-Plane uplink PRACH spy.
class data_flow_uplane_uplink_data_spy : public data_flow_uplane_uplink_data
{
  bool decode_function_called = false;

public:
  // See interface for documentation.
  void decode_type1_message(unsigned eaxc, span<const uint8_t> msg) override { decode_function_called = true; }

  /// Returns true if the decode_type1_message function has been called, otherwise false.
  bool has_decode_function_been_called() const { return decode_function_called; }
};

/// VLAN frame decoder spy.
class vlan_frame_decoder_spy : public ether::vlan_frame_decoder
{
  bool                     decode_function_called = false;
  ether::vlan_frame_params eth_params;

public:
  explicit vlan_frame_decoder_spy(const ether::vlan_frame_params& eth_params_) : eth_params(eth_params_) {}

  span<const uint8_t> decode(span<const uint8_t> frame, ether::vlan_frame_params& eth_params_) override
  {
    decode_function_called = true;
    eth_params_            = eth_params;
    return frame;
  }

  /// Sets the VLAN frame parameters to the given one.
  void set_vlan_params(const ether::vlan_frame_params& eth_params_) { eth_params = eth_params_; }

  /// Returns true if the decode_type1_message function has been called, otherwise false.
  bool has_decode_function_been_called() const { return decode_function_called; }
};

/// eCPRI packet decoder spy.
class ecpri_packet_decoder_spy : public ecpri::packet_decoder
{
  bool                     decode_function_called = false;
  ecpri::packet_parameters ecpri_params;

public:
  explicit ecpri_packet_decoder_spy(const ecpri::packet_parameters& params) : ecpri_params(params) {}

  // See interface for documentation.
  span<const uint8_t> decode(span<const uint8_t> packet, ecpri::packet_parameters& params) override
  {
    decode_function_called = true;
    params                 = ecpri_params;
    return packet;
  }

  /// Sets the VLAN frame parameters to the given one.
  void set_ecpri_params(const ecpri::packet_parameters& params) { ecpri_params = params; }

  /// Returns true if the decode_type1_message function has been called, otherwise false.
  bool has_decode_function_been_called() const { return decode_function_called; }
};

/// Dummy Ethernet receiver class.
class dummy_eth_receiver : public ether::receiver, public ether::receiver_operation_controller
{
  // See interface for documentation.
  void start(ether::frame_notifier& notifier) override {}

  // See interface for documentation.
  void stop() override {}

  // See interface for documentation.
  ether::receiver_operation_controller& get_operation_controller() override { return *this; }

  // See interface for documentation.
  ether::receiver_metrics_collector* get_metrics_collector() override { return nullptr; }
};

} // namespace

class ofh_message_receiver_fixture : public ::testing::Test
{
protected:
  ether::vlan_frame_params                        vlan_params   = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                                                                   {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                                                                   4,
                                                                   0xaefe};
  static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC> ul_prach_eaxc = {0, 1};
  static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC> ul_eaxc       = {4, 5};
  manual_task_worker_always_enqueue_tasks         executor;
  closed_rx_window_handler                        closed_window_handler;
  rx_window_checker                               window_checker;
  ecpri::packet_parameters                        ecpri_params;
  data_flow_uplane_uplink_data_spy*               df_uplink;
  data_flow_uplane_uplink_prach_spy*              df_prach;
  ecpri_packet_decoder_spy*                       ecpri_decoder;
  vlan_frame_decoder_spy*                         vlan_decoder;
  message_receiver_impl                           ul_handler;

public:
  ofh_message_receiver_fixture() :
    executor(20),
    closed_window_handler({},
                          {&srslog::fetch_basic_logger("TEST"),
                           &executor,
                           std::make_shared<prach_context_repository>(20),
                           std::make_shared<uplink_context_repository>(20),
                           std::make_shared<dummy_uplane_rx_symbol_notifier>()}),
    window_checker(false, {}),
    ul_handler(generate_config(), generate_dependencies())
  {
    window_checker.on_new_symbol({{{1, 0}, 0, 14}, {}});
  }

  message_receiver_config generate_config()
  {
    message_receiver_config config;
    config.nof_symbols = 14;
    config.scs         = subcarrier_spacing::kHz30;
    config.vlan_params = vlan_params;
    config.ul_eaxc     = ul_eaxc;
    config.prach_eaxc  = ul_prach_eaxc;

    return config;
  }

  message_receiver_dependencies generate_dependencies()
  {
    message_receiver_dependencies dependencies;
    dependencies.logger         = &srslog::fetch_basic_logger("TEST");
    dependencies.window_checker = &window_checker;
    dependencies.window_handler = &closed_window_handler;

    {
      auto temp                    = std::make_unique<data_flow_uplane_uplink_prach_spy>();
      df_prach                     = temp.get();
      dependencies.data_flow_prach = std::move(temp);
    }
    {
      auto temp                     = std::make_unique<data_flow_uplane_uplink_data_spy>();
      df_uplink                     = temp.get();
      dependencies.data_flow_uplink = std::move(temp);
    }
    {
      auto temp                      = std::make_unique<vlan_frame_decoder_spy>(vlan_params);
      vlan_decoder                   = temp.get();
      dependencies.eth_frame_decoder = std::move(temp);
    }
    {
      ecpri_params.header.msg_type       = ecpri::message_type::iq_data;
      ecpri_params.header.is_last_packet = true;
      ecpri_params.header.payload_size   = units::bytes(1);
      ecpri_params.header.revision       = 1U;
      ecpri_params.type_params.emplace<ecpri::iq_data_parameters>(ecpri::iq_data_parameters{1, 2});

      auto temp                  = std::make_unique<ecpri_packet_decoder_spy>(ecpri_params);
      ecpri_decoder              = temp.get();
      dependencies.ecpri_decoder = std::move(temp);
    }
    dependencies.eth_receiver   = std::make_unique<dummy_eth_receiver>();
    dependencies.seq_id_checker = std::make_unique<sequence_id_checker_dummy_impl>();

    return dependencies;
  }
};

TEST_F(ofh_message_receiver_fixture, discard_frames_with_unexpected_ethernet_type)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{1}));

  ether::vlan_frame_params params = vlan_params;
  params.eth_type                 = 7777;
  vlan_decoder->set_vlan_params(params);

  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_FALSE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_FALSE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, discard_frames_with_unexpected_src_mac)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{1}));

  ether::vlan_frame_params params = vlan_params;
  params.mac_src_address          = {0xbe, 0xbe, 0xca, 0xfe, 0xba, 0xca};
  vlan_decoder->set_vlan_params(params);

  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_FALSE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_FALSE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, discard_frames_with_unexpected_dst_mac)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{1}));

  ether::vlan_frame_params params = vlan_params;
  params.mac_dst_address          = {0xbe, 0xbe, 0xca, 0xfe, 0xba, 0xca};
  vlan_decoder->set_vlan_params(params);

  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_FALSE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_FALSE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, discard_ecpri_control_frames)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{1}));

  ecpri::packet_parameters params;
  params.header.msg_type = srsran::ecpri::message_type::rt_control_data;
  params.type_params.emplace<ecpri::realtime_control_parameters>(ecpri::realtime_control_parameters{1, 2});
  ecpri_decoder->set_ecpri_params(params);

  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_TRUE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_FALSE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, discard_frames_with_unexpected_uplink_eacx)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{1}));

  ecpri::packet_parameters params                               = ecpri_params;
  std::get<ecpri::iq_data_parameters>(params.type_params).pc_id = 2;
  ecpri_decoder->set_ecpri_params(params);

  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_TRUE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_FALSE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, discard_frames_with_unexpected_prach_eacx)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{1}));

  ecpri::packet_parameters params                               = ecpri_params;
  std::get<ecpri::iq_data_parameters>(params.type_params).pc_id = 6;
  ecpri_decoder->set_ecpri_params(params);

  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_TRUE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_FALSE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, invalid_slot_point_peek_does_not_call_data_flows)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{1, 0, 0}));
  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_TRUE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_FALSE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, valid_uplink_message_gets_processed_by_data_flow)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{0, 0, 0, 0}));
  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_TRUE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_TRUE(df_uplink->has_decode_function_been_called());
  ASSERT_FALSE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, valid_long_prach_message_gets_processed_by_data_flow)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{1, 0, 0, 0}));
  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_TRUE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_TRUE(df_prach->has_decode_function_been_called());
}

TEST_F(ofh_message_receiver_fixture, valid_short_prach_message_gets_processed_by_data_flow)
{
  ether::unique_rx_buffer buffer(dummy_eth_rx_buffer(std::vector<uint8_t>{3, 0, 0, 0}));
  ul_handler.on_new_frame(std::move(buffer));

  ASSERT_TRUE(vlan_decoder->has_decode_function_been_called());
  ASSERT_TRUE(ecpri_decoder->has_decode_function_been_called());
  ASSERT_FALSE(df_uplink->has_decode_function_been_called());
  ASSERT_TRUE(df_prach->has_decode_function_been_called());
}
