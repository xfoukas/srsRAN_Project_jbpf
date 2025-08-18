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

#include "radio_notifier_sample.h"
#include "srsran/adt/spsc_queue.h"
#include "srsran/adt/to_array.h"
#include "srsran/gateways/baseband/baseband_gateway_receiver.h"
#include "srsran/gateways/baseband/baseband_gateway_transmitter.h"
#include "srsran/gateways/baseband/buffer/baseband_gateway_buffer_dynamic.h"
#include "srsran/radio/radio_factory.h"
#include "srsran/support/executors/task_worker.h"
#include "srsran/support/file_vector.h"
#include "srsran/support/math/math_utils.h"
#include "srsran/support/signal_handling.h"
#include <getopt.h>
#include <random>

using namespace srsran;

/// Describes the benchmark configuration.
static std::string                               rx_filename;
static double                                    duration_s                    = 0.1;
static srslog::basic_levels                      log_level                     = srslog::basic_levels::info;
static std::string                               driver_name                   = "uhd";
static std::string                               device_arguments              = "type=b200";
static std::vector<std::string>                  tx_channel_arguments          = {};
static std::vector<std::string>                  rx_channel_arguments          = {};
static double                                    sampling_rate_Hz              = 23.04e6;
static unsigned                                  nof_tx_streams                = 1;
static unsigned                                  nof_rx_streams                = 1;
static unsigned                                  nof_channels_per_stream       = 1;
static unsigned                                  block_size                    = sampling_rate_Hz / 15e3;
static double                                    tx_freq                       = 3.5e9;
static double                                    tx_gain                       = 60.0;
static double                                    rx_freq                       = 3.5e9;
static double                                    rx_gain                       = 60.0;
static double                                    tx_rx_delay_s                 = 0.001;
static bool                                      enable_discontinuous_tx       = false;
static unsigned                                  nof_consecutive_empty_buffers = 0;
static float                                     power_ramping_us              = 200.0F;
static radio_configuration::over_the_wire_format otw_format = radio_configuration::over_the_wire_format::SC16;

namespace {
/// Describes a benchmark configuration profile.
struct benchmark_configuration_profile {
  std::string           name;
  std::string           description;
  std::function<void()> function;
};
} // namespace

/// Defines a set of configuration profiles.
static const auto benchmark_profiles = to_array<benchmark_configuration_profile>({
    {"b200_20MHz",
     "Single channel B200 USRP 20MHz bandwidth.",
     []() {
       // Do nothing.
     }},
    {"b200_50MHz",
     "Single channel B200 USRP 50MHz bandwidth.",
     []() {
       device_arguments = "type=b200";
       sampling_rate_Hz = 61.44e6;
       otw_format       = radio_configuration::over_the_wire_format::SC12;
       block_size       = sampling_rate_Hz / 15e3;
     }},
    {"b200_20MHz_2x2",
     "Dual channel B200 USRP 20MHz bandwidth.",
     []() {
       device_arguments        = "type=b200";
       otw_format              = radio_configuration::over_the_wire_format::SC12;
       block_size              = sampling_rate_Hz / 15e3;
       nof_channels_per_stream = 1;
       nof_tx_streams          = 2;
       nof_rx_streams          = 2;
     }},
    {"x300_20MHz",
     "Single channel X3x0 USRP 20MHz bandwidth.",
     []() {
       device_arguments = "type=x300,master_clock_rate=184.32e6,send_frame_size=8000,recv_frame_size=8000";
       sampling_rate_Hz = 23.04e6;
       otw_format       = radio_configuration::over_the_wire_format::SC16;
       block_size       = sampling_rate_Hz / 15e3;
       tx_gain          = 10.0;
       rx_gain          = 10.0;
     }},
    {"x300_50MHz",
     "Single channel X3x0 USRP 50MHz bandwidth.",
     []() {
       device_arguments = "type=x300,master_clock_rate=184.32e6,send_frame_size=8000,recv_frame_size=8000";
       sampling_rate_Hz = 92.16e6;
       otw_format       = radio_configuration::over_the_wire_format::SC16;
       block_size       = sampling_rate_Hz / 15e3;
       tx_gain          = 10.0;
       rx_gain          = 10.0;
     }},
    {"x300_100MHz",
     "Single channel X3x0 USRP 100MHz bandwidth.",
     []() {
       device_arguments = "type=x300,master_clock_rate=184.32e6,send_frame_size=8000,recv_frame_size=8000";
       sampling_rate_Hz = 184.32e6;
       otw_format       = radio_configuration::over_the_wire_format::SC16;
       block_size       = sampling_rate_Hz / 15e3;
       tx_gain          = 10.0;
       rx_gain          = 10.0;
     }},
    {"x300_20MHz_2x2",
     "Dual channel X3x0 USRP 20MHz bandwidth.",
     []() {
       device_arguments        = "type=x300,master_clock_rate=184.32e6,send_frame_size=8000,recv_frame_size=8000";
       sampling_rate_Hz        = 23.04e6;
       otw_format              = radio_configuration::over_the_wire_format::SC16;
       block_size              = sampling_rate_Hz / 15e3;
       tx_gain                 = 10.0;
       rx_gain                 = 10.0;
       nof_channels_per_stream = 1;
       nof_tx_streams          = 2;
       nof_rx_streams          = 2;
     }},
    {"x300_50MHz_2x2",
     "Dual channel X3x0 USRP 50MHz bandwidth.",
     []() {
       device_arguments        = "type=x300,master_clock_rate=184.32e6,send_frame_size=8000,recv_frame_size=8000";
       sampling_rate_Hz        = 92.16e6;
       otw_format              = radio_configuration::over_the_wire_format::SC16;
       block_size              = sampling_rate_Hz / 15e3;
       tx_gain                 = 10.0;
       rx_gain                 = 10.0;
       nof_channels_per_stream = 1;
       nof_tx_streams          = 2;
       nof_rx_streams          = 2;
     }},
    {"x300_100MHz_2x2",
     "Dual channel X3x0 USRP 100MHz bandwidth.",
     []() {
       device_arguments        = "type=x300,master_clock_rate=184.32e6,send_frame_size=8000,recv_frame_size=8000";
       sampling_rate_Hz        = 184.32e6;
       otw_format              = radio_configuration::over_the_wire_format::SC16;
       block_size              = sampling_rate_Hz / 15e3;
       tx_gain                 = 10.0;
       rx_gain                 = 10.0;
       nof_channels_per_stream = 1;
       nof_tx_streams          = 2;
       nof_rx_streams          = 2;
     }},
    {"zmq_20MHz_loopback",
     "Single channel ZMQ.",
     []() {
       driver_name          = "zmq";
       device_arguments     = "";
       tx_channel_arguments = {"tcp://*:5554"};
       rx_channel_arguments = {"tcp://localhost:5554"};
       sampling_rate_Hz     = 23.04e6;
       otw_format           = radio_configuration::over_the_wire_format::DEFAULT;
       block_size           = sampling_rate_Hz / 15e3;
     }},
});

/// Set to true to stop.
static std::atomic<bool>              stop  = {false};
static std::unique_ptr<radio_session> radio = nullptr;

/// Function to call when the application is interrupted.
static void interrupt_signal_handler(int signal)
{
  if (radio != nullptr) {
    radio->stop();
  }
  stop = true;
}

/// Function to call when the application is going to be forcefully shutdown.
static void cleanup_signal_handler(int signal)
{
  srslog::flush();
}

static void usage(std::string_view prog)
{
  fmt::print("Usage: {} [-P profile] [-D duration] [-v level] [-o file name]\n", prog);
  fmt::print("\t-P Profile. [Default {}]\n", benchmark_profiles.front().name);
  for (const benchmark_configuration_profile& profile : benchmark_profiles) {
    fmt::print("\t\t {:<30}{}\n", profile.name, profile.description);
  }
  fmt::print("\t-D Duration in seconds. [Default {}]\n", duration_s);
  fmt::print("\t-d Enable discontinuous transmission mode. [Default {}]\n", enable_discontinuous_tx);
  fmt::print("\t-g Discontinuous transmission gap in number of consecutive empty buffers. [Default {}]\n",
             nof_consecutive_empty_buffers);
  fmt::print("\t-v Logging level. [Default {}]\n", srslog::basic_level_to_string(log_level));
  fmt::print("\t-o saves received signal of stream:port 0:0 in a file. Ignored if none. [Default {}]\n",
             rx_filename.empty() ? "none" : rx_filename);
  fmt::print("\t-h print this message.\n");
}

static void parse_args(int argc, char** argv)
{
  std::string profile_name;

  int opt = 0;
  while ((opt = getopt(argc, argv, "o:D:dg:P:v:h")) != -1) {
    switch (opt) {
      case 'o':
        rx_filename = std::string(optarg);
        break;
      case 'P':
        if (optarg != nullptr) {
          profile_name = std::string(optarg);
        }
        break;
      case 'D':
        if (optarg != nullptr) {
          duration_s = std::strtod(optarg, nullptr);
        }
        break;
      case 'd':
        enable_discontinuous_tx = true;
        break;
      case 'g':
        if (optarg != nullptr) {
          nof_consecutive_empty_buffers = std::strtol(optarg, nullptr, 10);
        }
        break;
      case 'v': {
        auto level = srslog::str_to_basic_level(std::string(optarg));
        log_level  = level.has_value() ? level.value() : srslog::basic_levels::info;
        break;
      }
      case 'h':
      default:
        usage(argv[0]);
        exit(-1);
    }
  }

  // Search profile if set.
  if (!profile_name.empty()) {
    bool found = false;
    for (const benchmark_configuration_profile& profile : benchmark_profiles) {
      if (strcmp(profile_name.c_str(), profile.name.c_str()) == 0) {
        profile.function();
        found = true;
        break;
      }
    }
    if (!found) {
      usage(argv[0]);
      report_error("Invalid profile {}.\n", profile_name);
    }
  }
}

int main(int argc, char** argv)
{
  // Set interrupt and cleanup signal handlers.
  register_interrupt_signal_handler(interrupt_signal_handler);
  register_cleanup_signal_handler(cleanup_signal_handler);

  // Parse arguments.
  parse_args(argc, argv);

  // Random data generator.
  std::mt19937                          rgen(0);
  std::uniform_real_distribution<float> dist(-1.0, +1.0);

  unsigned tx_rx_delay_samples = static_cast<unsigned>(sampling_rate_Hz * tx_rx_delay_s);

  // Asynchronous task executor.
  task_worker                    async_task_worker("async_thread", RADIO_MAX_NOF_PORTS);
  std::unique_ptr<task_executor> async_task_executor = make_task_executor_ptr(async_task_worker);

  // Create radio factory.
  std::unique_ptr<radio_factory> factory = create_radio_factory(driver_name);
  report_fatal_error_if_not(factory, "Driver {} is not available.", driver_name.c_str());

  // Create radio configuration.
  radio_configuration::radio config;
  config.clock.sync       = radio_configuration::clock_sources::source::DEFAULT;
  config.clock.clock      = radio_configuration::clock_sources::source::DEFAULT;
  config.sampling_rate_Hz = sampling_rate_Hz;
  config.otw_format       = otw_format;
  config.tx_mode          = enable_discontinuous_tx ? radio_configuration::transmission_mode::discontinuous
                                                    : radio_configuration::transmission_mode::continuous;
  config.power_ramping_us = power_ramping_us;
  config.args             = device_arguments;
  config.log_level        = log_level;
  radio_configuration::stream rx_stream_config;

  // Create Tx stream and channels.
  std::vector<baseband_gateway_buffer_dynamic> tx_baseband_buffers;
  for (unsigned stream_idx = 0; stream_idx != nof_tx_streams; ++stream_idx) {
    // Create transmit baseband buffer for the stream.
    tx_baseband_buffers.emplace_back(nof_channels_per_stream, block_size);

    // For each channel in the stream...
    radio_configuration::stream stream_config;
    for (unsigned channel_idx = 0; channel_idx != nof_channels_per_stream; ++channel_idx) {
      // Create channel configuration and append it to the previous ones.
      radio_configuration::channel ch_config;
      ch_config.freq.center_frequency_Hz = tx_freq;
      ch_config.gain_dB                  = tx_gain;
      if (!tx_channel_arguments.empty()) {
        ch_config.args = tx_channel_arguments[stream_idx * nof_channels_per_stream + channel_idx];
      }
      stream_config.channels.emplace_back(ch_config);

      // Generate random data in buffer.
      span<cf_t> data = tx_baseband_buffers.back()[channel_idx];
      for (cf_t& iq : data) {
        iq = {dist(rgen), dist(rgen)};
      }
    }
    config.tx_streams.emplace_back(stream_config);
  }

  // Create Rx stream and channels.
  std::vector<baseband_gateway_buffer_dynamic> rx_baseband_buffers;
  for (unsigned stream_idx = 0; stream_idx != nof_rx_streams; ++stream_idx) {
    // Create receive baseband buffer for the stream.
    rx_baseband_buffers.emplace_back(nof_channels_per_stream, block_size);

    // For each channel in the stream...
    radio_configuration::stream stream_config;
    for (unsigned channel_idx = 0; channel_idx != nof_channels_per_stream; ++channel_idx) {
      radio_configuration::channel ch_config;
      ch_config.freq.center_frequency_Hz = rx_freq;
      ch_config.gain_dB                  = rx_gain;
      if (!rx_channel_arguments.empty()) {
        ch_config.args = rx_channel_arguments[stream_idx * nof_channels_per_stream + channel_idx];
      }
      stream_config.channels.emplace_back(ch_config);
    }
    config.rx_streams.emplace_back(stream_config);
  }

  // Create notification handler
  radio_notifier_spy notification_handler(log_level);

  // Create radio.
  radio = factory->create(config, *async_task_executor, notification_handler);
  report_fatal_error_if_not(radio, "Failed to create radio.");

  // Calculate duration in samples.
  uint64_t total_nof_samples = static_cast<uint64_t>(duration_s * sampling_rate_Hz);
  uint64_t sample_count      = 0;

  // Open file sink if mot empty.
  std::unique_ptr<file_vector<cf_t>> rx_file;
  if (!rx_filename.empty()) {
    rx_file = std::make_unique<file_vector<cf_t>>(rx_filename.c_str(), openmode::write_only);
  }

  // Counter for the number of empty transmission buffers.
  unsigned tx_gap_count = nof_consecutive_empty_buffers;

  // Calculate starting time.
  double                     delay_s      = 0.1;
  baseband_gateway_timestamp current_time = radio->read_current_time();
  baseband_gateway_timestamp start_time   = current_time + static_cast<uint64_t>(delay_s * sampling_rate_Hz);

  // Start processing.
  radio->start(start_time);

  // Receive and transmit per block basis.
  while (!stop && sample_count < total_nof_samples) {
    // For each stream...
    for (unsigned stream_id = 0; stream_id != nof_rx_streams; ++stream_id) {
      // Get transmitter data plane.
      baseband_gateway_transmitter& transmitter = radio->get_baseband_gateway(stream_id).get_transmitter();

      // Get receiver data plane.
      baseband_gateway_receiver& receiver = radio->get_baseband_gateway(stream_id).get_receiver();

      // Receive baseband.
      static_vector<baseband_gateway_receiver::metadata, RADIO_MAX_NOF_STREAMS> rx_metadata(nof_rx_streams);
      rx_metadata[stream_id] = receiver.receive(rx_baseband_buffers[stream_id].get_writer());

      // Save file.
      if (stream_id == 0 && rx_file) {
        rx_file->write(rx_baseband_buffers[stream_id][0]);
      }

      // Prepare transmit metadata.
      baseband_gateway_transmitter_metadata tx_metadata;
      tx_metadata.ts = rx_metadata.front().ts + tx_rx_delay_samples;
      if (tx_gap_count > 0) {
        // Send empty baseband buffer.
        tx_metadata.is_empty = true;
        --tx_gap_count;
      } else {
        tx_metadata.is_empty = false;
        // Reset counter.
        tx_gap_count = nof_consecutive_empty_buffers;
      }

      // Transmit baseband.
      transmitter.transmit(tx_baseband_buffers[stream_id].get_reader(), tx_metadata);
    }

    // Increment sample counter.
    sample_count += block_size;
  }

  if (!stop) {
    // Stop radio operation prior destruction.
    radio->stop();
  }

  // Stop asynchronous thread.
  async_task_worker.stop();

  notification_handler.print();

  return 0;
}
