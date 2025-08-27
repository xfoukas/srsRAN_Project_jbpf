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

#include "lib/rlc/rlc_rx_am_entity.h"
#include "lib/rlc/rlc_tx_am_entity.h"
#include "tests/test_doubles/pdcp/pdcp_pdu_generator.h"
#include "srsran/support/benchmark_utils.h"
#include "srsran/support/executors/inline_task_executor.h"
#include "srsran/support/executors/manual_task_worker.h"
#include <getopt.h>

using namespace srsran;

// Initialize timer backend queue. The unique_timer will push into this queue
// upon destruction.
static void initialize_thead_local_storage(timer_manager& timers)
{
  inline_task_executor inline_exec;
  auto                 dummy_timer = timers.create_unique_timer(inline_exec);
}

/// Mocking class of the surrounding layers invoked by the RLC AM Tx entity.
class rlc_tx_am_test_frame : public rlc_tx_upper_layer_data_notifier,
                             public rlc_tx_upper_layer_control_notifier,
                             public rlc_tx_lower_layer_notifier,
                             public rlc_rx_am_status_provider,
                             public rlc_metrics_notifier
{
public:
  rlc_am_sn_size    sn_size;
  rlc_am_status_pdu status;
  bool              status_required = false;
  uint32_t          bsr             = 0;
  uint32_t          bsr_count       = 0;
  uint32_t          max_retx_count  = 0;

  rlc_tx_am_test_frame(rlc_am_sn_size sn_size_) : sn_size(sn_size_), status(sn_size_) {}

  // rlc_tx_upper_layer_data_notifier interface
  void on_transmitted_sdu(uint32_t max_tx_pdcp_sn, uint32_t desired_buf_size) override {}
  void on_delivered_sdu(uint32_t max_deliv_pdcp_sn) override {}
  void on_retransmitted_sdu(uint32_t max_retx_pdcp_sn) override {}
  void on_delivered_retransmitted_sdu(uint32_t max_deliv_retx_pdcp_sn) override {}

  // rlc_tx_upper_layer_control_notifier interface
  void on_protocol_failure() override {}
  void on_max_retx() override {}

  // rlc_tx_buffer_state_update_notifier interface
  void on_buffer_state_update(const rlc_buffer_state& bs) override {}

  // rlc_rx_am_status_provider interface
  rlc_am_status_pdu& get_status_pdu() override { return status; }
  uint32_t           get_status_pdu_length() override { return status.get_packed_size(); }
  bool               status_report_required() override { return status_required; }

  // rlc_metrics_notifier
  void report_metrics(const rlc_metrics& metrics) override {}
};

/// Mocking class of the surrounding layers invoked by the RLC AM Rx entity.
class rlc_rx_am_test_frame : public rlc_rx_upper_layer_data_notifier,
                             public rlc_tx_am_status_handler,
                             public rlc_tx_am_status_notifier,
                             public rlc_metrics_notifier
{
public:
  rlc_rx_am_test_frame() {}

  // rlc_rx_upper_layer_data_notifier interface
  void on_new_sdu(byte_buffer_chain sdu) override {}

  // rlc_tx_am_status_handler interface
  void on_status_pdu(rlc_am_status_pdu status_) override {}
  // rlc_tx_am_status_notifier interface
  void on_status_report_changed() override {}
  // rlc_metrics_notifier
  void report_metrics(const rlc_metrics& metrics) override {}
};

struct bench_params {
  unsigned nof_repetitions = 10000;
  unsigned sdu_size        = 1500;
  unsigned pdu_size        = 1550;
};

enum class rx_order {
  in_order,      ///< 0, 1, 2,..., N-1
  swapped_edges, ///< N-1, 1, 2,..., N-2, 0
  reverse_order, ///< N-1, N-2, N-3,..., 0
  even_odd       ///< 0, 2, 4,...,1, 3, 5,...
};

static void usage(const char* prog, const bench_params& params)
{
  fmt::print("Usage: {} [-R repetitions] [-s silent]\n", prog);
  fmt::print("\t-R Repetitions [Default {}]\n", params.nof_repetitions);
  fmt::print("\t-s SDU size [Default {}]\n", params.sdu_size);
  fmt::print("\t-p PDU size [Default {}]\n", params.pdu_size);
  fmt::print("\t-h Show this message\n");
}

static void parse_args(int argc, char** argv, bench_params& params)
{
  int opt = 0;
  while ((opt = getopt(argc, argv, "R:s:p:h")) != -1) {
    switch (opt) {
      case 'R':
        params.nof_repetitions = std::strtol(optarg, nullptr, 10);
        break;
      case 's':
        params.sdu_size = std::strtol(optarg, nullptr, 10);
        break;
      case 'p':
        params.pdu_size = std::strtol(optarg, nullptr, 10);
        break;
      case 'h':
      default:
        usage(argv[0], params);
        exit(0);
    }
  }
}

static std::vector<byte_buffer> generate_pdus(bench_params params, rx_order order, timer_manager& timers)
{
  // Set Tx config
  rlc_tx_am_config config;
  config.sn_field_length  = rlc_am_sn_size::size18bits;
  config.pdcp_sn_len      = pdcp_sn_size::size18bits;
  config.t_poll_retx      = 45;
  config.max_retx_thresh  = 4;
  config.poll_pdu         = 4;
  config.poll_byte        = 25;
  config.queue_size       = 4096;
  config.queue_size_bytes = 4096 * 1507;
  config.max_window       = 0;

  // Create test frame
  auto tester = std::make_unique<rlc_tx_am_test_frame>(config.sn_field_length);

  manual_task_worker pcell_worker{128};
  manual_task_worker ue_worker{128};

  // Create RLC AM TX entity
  std::unique_ptr<rlc_tx_am_entity>             rlc_tx       = nullptr;
  std::unique_ptr<rlc_bearer_metrics_collector> metrics_coll = nullptr;

  auto& logger = srslog::fetch_basic_logger("RLC");
  logger.set_level(srslog::basic_levels::warning);

  null_rlc_pcap pcap;

  metrics_coll = std::make_unique<rlc_bearer_metrics_collector>(
      gnb_du_id_t{}, du_ue_index_t{}, rb_id_t{}, timer_duration{0}, tester.get(), ue_worker);

  // Make PDUs
  std::vector<byte_buffer> pdus;
  rlc_tx = std::make_unique<rlc_tx_am_entity>(gnb_du_id_t::min,
                                              du_ue_index_t::MIN_DU_UE_INDEX,
                                              drb_id_t::drb1,
                                              config,
                                              *tester,
                                              *tester,
                                              *tester,
                                              *metrics_coll,
                                              pcap,
                                              pcell_worker,
                                              ue_worker,
                                              timers);

  // Bind AM Rx/Tx interconnect
  rlc_tx->set_status_provider(tester.get());

  // Prepare SDU list for benchmark
  int num_sdus  = params.nof_repetitions + 1; // +1 to expire t_reassembly on setup
  int num_bytes = params.sdu_size;
  int num_pdus  = 0;
  int pdu_size  = params.pdu_size;
  for (int i = 0; i < num_sdus; i++) {
    byte_buffer sdu = test_helpers::create_pdcp_pdu(config.pdcp_sn_len, /* is_srb = */ false, i, num_bytes, i);
    rlc_tx->handle_sdu(std::move(sdu), false);
    while (rlc_tx->get_buffer_state().pending_bytes > 0) {
      std::vector<uint8_t> pdu_buf;
      pdu_buf.resize(pdu_size);
      size_t pdu_len = rlc_tx->pull_pdu(pdu_buf);
      pdu_buf.resize(pdu_len);
      auto buf = byte_buffer::create(pdu_buf);
      report_error_if_not(buf.has_value(), "Failed to allocate byte_buffer");
      pdus.emplace_back(std::move(buf.value()));
      num_pdus++;
    }
  }

  // shuffle PDUs according to requested order
  switch (order) {
    case rx_order::in_order:
      break;
    case rx_order::swapped_edges:
      std::swap(pdus.front(), pdus.back());
      break;
    case rx_order::reverse_order:
      std::reverse(pdus.begin(), pdus.end());
      break;
    case rx_order::even_odd:
      std::vector<byte_buffer> pdus_mod;
      for (int i = 0; i < num_pdus; i += 2) {
        pdus_mod.push_back(std::move(pdus[i]));
      }
      for (int i = 1; i < num_pdus; i += 2) {
        pdus_mod.push_back(std::move(pdus[i]));
      }
      pdus = std::move(pdus_mod);
      break;
  }

  timers.tick();
  return pdus;
}

static void benchmark_rx_pdu(const bench_params& params, rx_order order, timer_manager& timers)
{
  fmt::memory_buffer buffer;
  fmt::format_to(std::back_inserter(buffer), "Benchmark RLC AM RX PDUs ({})", order);
  std::unique_ptr<benchmarker> bm = std::make_unique<benchmarker>(to_c_str(buffer), params.nof_repetitions);

  auto tester = std::make_unique<rlc_rx_am_test_frame>();

  manual_task_worker pcell_worker{128};
  manual_task_worker ue_worker{128};

  null_rlc_pcap pcap;

  // Set Tx config
  rlc_rx_am_config config;
  config.sn_field_length   = rlc_am_sn_size::size18bits;
  config.t_status_prohibit = 0;
  config.t_reassembly      = 200;

  auto metrics_agg = std::make_unique<rlc_bearer_metrics_collector>(
      gnb_du_id_t{}, du_ue_index_t{}, rb_id_t{}, timer_duration{0}, tester.get(), ue_worker);

  // Create RLC AM RX entity
  std::unique_ptr<rlc_rx_am_entity> rlc_rx = std::make_unique<rlc_rx_am_entity>(gnb_du_id_t::min,
                                                                                du_ue_index_t::MIN_DU_UE_INDEX,
                                                                                drb_id_t::drb1,
                                                                                config,
                                                                                *tester,
                                                                                *metrics_agg,
                                                                                pcap,
                                                                                ue_worker,
                                                                                timers);

  // Bind AM Rx/Tx interconnect
  rlc_rx->set_status_notifier(tester.get());

  std::vector<byte_buffer> pdus = generate_pdus(params, order, timers);

  unsigned i = 0;

  // Push first PDU and expire reassembly timer to advance rx_highest_status
  rlc_rx->handle_pdu(std::move(pdus[i++]));
  for (int32_t t = 0; t < config.t_reassembly + 1; t++) {
    timers.tick();
  }
  ue_worker.run_pending_tasks();

  auto measure = [&rlc_rx, &i, &pdus]() mutable {
    rlc_rx->handle_pdu(std::move(pdus[i]));
    i++;
  };
  bm->new_measure("RX RLC AM PDU", 1500 * 8, measure);

  // Output results.
  bm->print_percentiles_time();
  bm->print_percentiles_throughput(" bps");
}

int main(int argc, char** argv)
{
  srslog::fetch_basic_logger("RLC").set_level(srslog::basic_levels::error);

  srslog::init();

  bench_params params{};
  parse_args(argc, argv, params);

  // Add pre-initialization of resources to created threads.
  timer_manager timers;
  initialize_thead_local_storage(timers);

  // Setup size of byte buffer pool.
  init_byte_buffer_segment_pool(524288);

  {
    benchmark_rx_pdu(params, rx_order::in_order, timers);
  }
  {
    benchmark_rx_pdu(params, rx_order::swapped_edges, timers);
  }
  {
    benchmark_rx_pdu(params, rx_order::reverse_order, timers);
  }
  {
    benchmark_rx_pdu(params, rx_order::even_odd, timers);
  }
  srslog::flush();
}

//
// Formatters
//
namespace fmt {

// RLC mode
template <>
struct formatter<rx_order> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(rx_order order, FormatContext& ctx) const
  {
    static constexpr const char* options[] = {"in order", "swapped edges", "reverse order", "even odd"};
    return format_to(ctx.out(), "{}", options[static_cast<unsigned>(order)]);
  }
};

} // namespace fmt
