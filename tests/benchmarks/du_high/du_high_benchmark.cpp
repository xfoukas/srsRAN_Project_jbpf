/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

/// \file
/// \brief Benchmarks for the DU-high latency.
///
/// In this benchmark, we emulate the behaviors of the CU-CP, CU-UP and PHY in order to simulate a realistic deployment
/// of the instantiated DU-high class. However, there are some simplifications:
/// - the PHY is emulated by a single thread, rather than by separate threads for DL PHY, UL PHY, and slot indications.
/// In practice, this may lead to less contention faced by the notifications of the PHY that reach the MAC.
/// - metrics handlers and E2 agents are instantiated separately from the DU-high class. If these entities have blocking
/// calls, such as mutexes, they may introduce additional latency.
/// - Complicated CU-CP procedures (e.g. reconfig) and their impact on the MAC latency are left out of this benchmark.
/// - The UP traffic is continuous rather than bursty.
/// - The activity of higher priority threads, namely DL PHY and RU threads are not emulated. This will mean that
/// the DU-high threads will be preempted less often than in a real deployment. To circumvent it, we can run a parallel
/// stress job in the same machine. For instance, to run stressors that solely interfere with the gnb_ue and gnb_ctrl
/// threads (and not with the du_cell thread), one can run:
/// > sudo stress-ng --cpu 6 --taskset 2-7 --sched fifo --sched-prio 97 -l 90
/// which will instantiate 6 threads running in cores 2-7 (assuming du_cell is pinned to 0-1), with high priority
/// and a CPU load of 90%.

#include "lib/du_high/du_high_executor_strategies.h"
#include "lib/du_high/du_high_impl.h"
#include "lib/mac/mac_ul/ul_bsr.h"
#include "tests/test_doubles/f1ap/f1ap_test_messages.h"
#include "tests/unittests/f1ap/du/f1ap_du_test_helpers.h"
#include "srsran/asn1/f1ap/common.h"
#include "srsran/asn1/f1ap/f1ap_pdu_contents_ue.h"
#include "srsran/du/du_cell_config_helpers.h"
#include "srsran/du_high/du_high_configuration.h"
#include "srsran/f1u/du/f1u_gateway.h"
#include "srsran/support/benchmark_utils.h"
#include "srsran/support/event_tracing.h"
#include "srsran/support/executors/priority_task_worker.h"
#include "srsran/support/executors/task_worker.h"
#include "srsran/support/test_utils.h"
#include <pthread.h>

using namespace srsran;
using namespace srs_du;

/// Constant used to bound the number of bytes pushed to the DU DL F1-U interface per slot.
const unsigned MAX_F1U_DL_BITRATE_PER_PORT_BPS = 500e6;

/// \brief Parameters of the benchmark.
struct bench_params {
  /// \brief Number of runs for the benchmark. Each repetition corresponds to a slot.
  unsigned nof_repetitions = 100;
  /// \brief Number to DU UEs to consider for benchmark.
  std::vector<unsigned> nof_ues = {1};
  /// \brief Set duplex mode (FDD or TDD) to use for benchmark.
  duplex_mode dplx_mode = duplex_mode::FDD;
  /// \brief Set the number of bytes pushed to the DU DL F1-U interface per slot.
  ///
  /// Setting this value to 0 will disable DL Tx.
  /// If the air interface cannot keep up with the DL F1-U PDU rate, the F1-U will be throttled, to avoid depleting
  /// the buffer pool.
  unsigned dl_bytes_per_slot = 1500;
  /// \brief Set size of the UL Buffer status report to push for UL Tx. Setting this value to 0 will disable UL Tx.
  unsigned ul_bsr_bytes = 0;
  /// \brief Maximum number of RBs per UE DL grant per slot.
  unsigned max_dl_rb_grant = MAX_NOF_PRBS;
  /// \brief Size of the F1-U PDU used in bytes.
  units::bytes pdu_size{1500};
  /// \brief Logical cores used by the "du_cell" thread.
  std::vector<unsigned> du_cell_cores = {};
};

static void usage(const char* prog, const bench_params& params)
{
  fmt::print("Usage: {} [-R repetitions] [-U nof. ues] [-D Duplex mode] [-d DL bytes per slot] [-u UL BSR] [-r Max RBs "
             "per UE DL grant] [-a CPU affinity] [-p F1-U PDU size]\n",
             prog);
  fmt::print("\t-R Repetitions [Default {}]\n", params.nof_repetitions);
  fmt::print("\t-U Nof. DU UEs for each simulation (e.g. \"1,5,10\" would run three benchmarks with 1, 5 and 10 UEs) "
             "[Default {}]\n",
             params.nof_ues);
  fmt::print("\t-D Duplex mode (FDD/TDD) [Default {}]\n", to_string(params.dplx_mode));
  fmt::print("\t-d Number of bytes pushed to the DU DL F1-U interface every slot. Setting this value to 0 will "
             "disable DL Tx. [Default {}]\n",
             params.dl_bytes_per_slot);
  fmt::print("\t-u Size of the UL Buffer status report to push for UL Tx. Setting this value to 0 will disable UL Tx. "
             "[Default {}]\n",
             params.ul_bsr_bytes);
  fmt::print("\t-r Max RBs per UE DL grant per slot [Default 275]\n");
  fmt::print("\t-a \"du_cell\" cores that the benchmark should use [Default \"no CPU affinity\"]\n");
  fmt::print("\t-p F1-U PDU size used [Default {}]", params.pdu_size);
  fmt::print("\t-h Show this message\n");
}

template <typename Func, typename Ret = decltype(std::declval<Func>()(std::string{}))>
static std::vector<Ret> tokenize(const std::string& s, Func&& func)
{
  std::vector<Ret>   tokens;
  std::string        token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, ',')) {
    tokens.push_back(func(token));
  }
  return tokens;
}

static void parse_args(int argc, char** argv, bench_params& params)
{
  int opt = 0;
  while ((opt = getopt(argc, argv, "R:U:D:d:u:r:a:p:h")) != -1) {
    switch (opt) {
      case 'R':
        params.nof_repetitions = std::strtol(optarg, nullptr, 10);
        break;
      case 'U': {
        params.nof_ues = tokenize(
            optarg, [](const std::string& token) -> unsigned { return std::strtol(token.c_str(), nullptr, 10); });
      } break;
      case 'D': {
        if (std::string(optarg) == "FDD") {
          params.dplx_mode = duplex_mode::FDD;
        } else if (std::string(optarg) == "TDD") {
          params.dplx_mode = duplex_mode::TDD;
        } else {
          usage(argv[0], params);
          exit(0);
        }
        break;
      }
      case 'd':
        params.dl_bytes_per_slot = std::strtol(optarg, nullptr, 10);
        break;
      case 'u':
        params.ul_bsr_bytes = std::strtol(optarg, nullptr, 10);
        break;
      case 'r':
        params.max_dl_rb_grant = std::strtol(optarg, nullptr, 10);
        break;
      case 'a': {
        std::string optstr{optarg};
        if (optstr.find(",") != std::string::npos) {
          size_t pos = optstr.find(",");
          while (pos != std::string::npos) {
            params.du_cell_cores.push_back(std::strtol(optstr.substr(0, pos).c_str(), nullptr, 10));
            optstr = optstr.substr(pos + 1);
            pos    = optstr.find(",");
          }
        } else {
          params.du_cell_cores = {(unsigned)std::strtol(optstr.c_str(), nullptr, 10)};
        }
      } break;
      case 'p':
        params.pdu_size = units::bytes{(unsigned)std::strtol(optarg, nullptr, 10)};
        break;
      case 'h':
      default:
        usage(argv[0], params);
        exit(0);
    }
  }

  // apply limits.
  subcarrier_spacing scs = params.dplx_mode == duplex_mode::FDD ? subcarrier_spacing::kHz15 : subcarrier_spacing::kHz30;
  double             slot_dur              = 0.001 / get_nof_slots_per_subframe(scs);
  unsigned           max_dl_bytes_per_slot = MAX_F1U_DL_BITRATE_PER_PORT_BPS * slot_dur / 8;
  params.dl_bytes_per_slot                 = std::min(max_dl_bytes_per_slot, params.dl_bytes_per_slot);
}

static void print_args(const bench_params& params)
{
  unsigned nof_tests = params.nof_ues.size();

  fmt::print("Arguments ({} tests):\n", nof_tests);
  fmt::print("- Number of repetitions: {}\n", params.nof_repetitions);
  fmt::print("- Duplex Mode: {}\n", params.dplx_mode == srsran::duplex_mode::FDD ? "FDD" : "TDD");
  if (nof_tests == 1) {
    fmt::print("- Number of UEs: {}\n", params.nof_ues[0]);
  } else {
    for (unsigned i = 0; i != nof_tests; ++i) {
      fmt::print("- Test {}:\n", i);
      fmt::print("  - Number of UEs: {}\n", params.nof_ues[i]);
    }
  }
  const double slot_dur_sec = params.dplx_mode == srsran::duplex_mode::FDD ? 0.001 : 0.0005;
  fmt::print("- F1-U DL Bitrate [Mbps]: {}\n", params.dl_bytes_per_slot * 8.0 * 1.0e-6 / slot_dur_sec);
  fmt::print("- F1-U DL PDU size [bytes]: {}\n", params.pdu_size);
  fmt::print("- BSR size [bytes]: {}\n", params.ul_bsr_bytes);
  fmt::print("- Max DL RB grant size [RBs]: {}\n", params.max_dl_rb_grant);
}

class dummy_metrics_handler : public scheduler_ue_metrics_notifier
{
public:
  void report_metrics(span<const scheduler_ue_metrics> ue_metrics) override
  {
    unsigned sum_dl_bs = 0;
    for (const auto& ue : ue_metrics) {
      sum_dl_bs += ue.dl_bs;
    }
    tot_dl_bs.store(sum_dl_bs, std::memory_order_relaxed);
  }

  // This metric is used by benchmark to determine whether to push more traffic to DU F1-U. Therefore, it needs to be
  // protected.
  std::atomic<unsigned> tot_dl_bs{0};
};

/// \brief Simulator of the CU-CP from the perspective of the DU. This class should reply to the F1AP messages
/// that the DU sends in order for the DU normal operation to proceed.
class cu_cp_simulator : public srs_du::f1c_connection_client
{
public:
  cu_cp_simulator() : du_rx_pdu_notifier(nullptr)
  {
    for (auto& flag : ue_created_flag_list) {
      flag.store(false, std::memory_order_relaxed);
    }
  }

private:
  class f1ap_du_tx_pdu_notifier : public f1ap_message_notifier
  {
  public:
    f1ap_du_tx_pdu_notifier(cu_cp_simulator& parent_) : parent(parent_) {}
    ~f1ap_du_tx_pdu_notifier() override { parent.du_rx_pdu_notifier.reset(); }

    void on_new_message(const f1ap_message& msg) override { parent.handle_message(msg); }

  private:
    cu_cp_simulator& parent;
  };

public:
  std::unique_ptr<f1ap_message_notifier>        du_rx_pdu_notifier;
  std::array<std::atomic<bool>, MAX_NOF_DU_UES> ue_created_flag_list;
  unsigned                                      next_gnb_cu_ue_f1ap_id = 0;

  std::unique_ptr<f1ap_message_notifier>
  handle_du_connection_request(std::unique_ptr<f1ap_message_notifier> du_rx_pdu_notifier_) override
  {
    du_rx_pdu_notifier = std::move(du_rx_pdu_notifier_);
    return std::make_unique<f1ap_du_tx_pdu_notifier>(*this);
  }

private:
  unsigned get_next_gnb_cu_ue_f1ap_id() { return next_gnb_cu_ue_f1ap_id++; }

  void handle_message(const f1ap_message& msg)
  {
    switch (msg.pdu.type().value) {
      case asn1::f1ap::f1ap_pdu_c::types_opts::init_msg:
        handle_init_msg(msg);
        break;
      case asn1::f1ap::f1ap_pdu_c::types_opts::successful_outcome:
        handle_success_outcome(msg.pdu.successful_outcome());
        break;
      default:
        report_fatal_error("Received invalid PDU type {} in this benchmark", msg.pdu.type().value);
    }
  }

  void handle_init_msg(const f1ap_message& msg)
  {
    const asn1::f1ap::init_msg_s& init_msg = msg.pdu.init_msg();
    switch (init_msg.value.type().value) {
      case asn1::f1ap::f1ap_elem_procs_o::init_msg_c::types_opts::f1_setup_request: {
        du_rx_pdu_notifier->on_new_message(test_helpers::generate_f1_setup_response(msg));
      } break;
      case asn1::f1ap::f1ap_elem_procs_o::init_msg_c::types_opts::f1_removal_request: {
        du_rx_pdu_notifier->on_new_message(test_helpers::generate_f1_removal_response(msg));
      } break;
      case asn1::f1ap::f1ap_elem_procs_o::init_msg_c::types_opts::init_ul_rrc_msg_transfer: {
        // Send UE Context Modification to create DRB1.
        f1ap_message msg2 = generate_ue_context_modification_request({drb_id_t::drb1});
        // Do not send RRC container.
        msg2.pdu.init_msg().value.ue_context_mod_request()->rrc_container_present = false;
        // Note: Use UM because AM requires status PDUs.
        auto& drb1 = msg2.pdu.init_msg()
                         .value.ue_context_mod_request()
                         ->drbs_to_be_setup_mod_list[0]
                         ->drbs_to_be_setup_mod_item();
        drb1.rlc_mode.value = asn1::f1ap::rlc_mode_opts::rlc_um_bidirectional;
        drb1.qos_info.choice_ext()->drb_info().drb_qos.qos_characteristics.non_dyn_5qi().five_qi =
            7; // UM in default configs
        msg2.pdu.init_msg().value.ue_context_mod_request()->gnb_cu_ue_f1ap_id = get_next_gnb_cu_ue_f1ap_id();
        msg2.pdu.init_msg().value.ue_context_mod_request()->gnb_du_ue_f1ap_id =
            init_msg.value.init_ul_rrc_msg_transfer()->gnb_du_ue_f1ap_id;
        du_rx_pdu_notifier->on_new_message(msg2);
      } break;
      default:
        report_fatal_error("Unhandled PDU type {} in this benchmark", init_msg.value.type().to_string());
    }
  }

  void handle_success_outcome(const asn1::f1ap::successful_outcome_s& succ_outcome)
  {
    switch (succ_outcome.value.type().value) {
      case asn1::f1ap::f1ap_elem_procs_o::successful_outcome_c::types_opts::ue_context_mod_resp: {
        ue_created_flag_list[succ_outcome.value.ue_context_mod_resp()->gnb_du_ue_f1ap_id].store(
            true, std::memory_order_relaxed);
      } break;
      default:
        report_fatal_error("Unreachable code in this benchmark");
    }
  }
};

/// \brief Dummy F1-U bearer for the purpose of benchmark.
class f1u_gw_dummy_bearer : public f1u_du_gateway_bearer
{
public:
  void on_new_pdu(nru_ul_message msg) override {}
  void stop() override {}
};

/// \brief Simulator of the CU-UP from the perspective of the DU.
class cu_up_simulator : public f1u_du_gateway
{
public:
  static_vector<f1u_gw_dummy_bearer*, MAX_NOF_DU_UES>                       bearer_list;
  static_vector<srs_du::f1u_du_gateway_bearer_rx_notifier*, MAX_NOF_DU_UES> du_notif_list;

  std::unique_ptr<f1u_du_gateway_bearer> create_du_bearer(uint32_t                                   ue_index,
                                                          drb_id_t                                   drb_id,
                                                          srs_du::f1u_config                         config,
                                                          const up_transport_layer_info&             dl_up_tnl_info,
                                                          const up_transport_layer_info&             ul_up_tnl_info,
                                                          srs_du::f1u_du_gateway_bearer_rx_notifier& du_rx,
                                                          timer_factory                              timers,
                                                          task_executor& ue_executor) override
  {
    auto f1u_bearer = std::make_unique<f1u_gw_dummy_bearer>();
    du_notif_list.push_back(&du_rx);
    bearer_list.push_back(f1u_bearer.get());
    return f1u_bearer;
  }

  void remove_du_bearer(const up_transport_layer_info& dl_tnl) override {}

  expected<std::string> get_du_bind_address(gnb_du_id_t gnb_du_id) override { return std::string("127.0.0.1"); }
};

/// \brief Instantiation of the DU-high workers and executors for the benchmark.
struct du_high_single_cell_worker_manager {
  using cell_worker_type                       = priority_task_worker;
  using ue_worker_type                         = priority_task_worker;
  static const uint32_t task_worker_queue_size = 100000;

  explicit du_high_single_cell_worker_manager(span<unsigned> du_cell_cores) :
    ctrl_worker("du_ctrl",
                task_worker_queue_size,
                os_thread_realtime_priority::max() - 20,
                get_other_affinity_mask(du_cell_cores)),
    cell_worker(
        "du_cell",
        std::vector<concurrent_queue_params>({{concurrent_queue_policy::lockfree_spsc, 8},
                                              {concurrent_queue_policy::lockfree_mpmc, task_worker_queue_size}}),
        std::chrono::microseconds{10},
        os_thread_realtime_priority::max() - 10,
        get_du_cell_affinity_mask(du_cell_cores)),
    ue_worker("du_ue",
              std::vector<concurrent_queue_params>({{concurrent_queue_policy::lockfree_mpmc, task_worker_queue_size},
                                                    {concurrent_queue_policy::lockfree_mpmc, task_worker_queue_size}}),
              std::chrono::microseconds{500},
              os_thread_realtime_priority::max() - 50,
              get_other_affinity_mask(du_cell_cores)),
    slot_exec(make_priority_task_worker_executor(enqueue_priority::max, cell_worker)),
    cell_exec(make_priority_task_worker_executor(enqueue_priority::max - 1, cell_worker)),
    ue_ctrl_exec(make_priority_task_worker_executor(enqueue_priority::max, ue_worker)),
    dl_exec(make_priority_task_worker_executor(enqueue_priority::max - 1, ue_worker)),
    ul_exec(make_priority_task_worker_executor(enqueue_priority::max, ue_worker))
  {
  }

  void stop()
  {
    ctrl_worker.stop();
    cell_worker.stop();
    ue_worker.stop();
  }

  static os_sched_affinity_bitmask get_du_cell_affinity_mask(span<const unsigned> du_cell_cores)
  {
    os_sched_affinity_bitmask mask;
    if (not du_cell_cores.empty()) {
      for (auto core : du_cell_cores) {
        mask.set(core);
      }
    }
    return mask;
  }

  static os_sched_affinity_bitmask get_other_affinity_mask(span<const unsigned> du_cell_cores)
  {
    os_sched_affinity_bitmask mask;
    if (not du_cell_cores.empty()) {
      for (unsigned i = 0; i != mask.size(); ++i) {
        if (std::find(du_cell_cores.begin(), du_cell_cores.end(), i) == du_cell_cores.end()) {
          mask.set(i);
        }
      }
    }
    return mask;
  }

  task_worker                   ctrl_worker;
  cell_worker_type              cell_worker;
  ue_worker_type                ue_worker;
  task_worker_executor          ctrl_exec{ctrl_worker};
  priority_task_worker_executor slot_exec;
  priority_task_worker_executor cell_exec;
  priority_task_worker_executor ue_ctrl_exec;
  priority_task_worker_executor dl_exec;
  priority_task_worker_executor ul_exec;
  du_high_executor_mapper_impl  du_high_exec_mapper{
      std::make_unique<cell_executor_mapper>(std::initializer_list<task_executor*>{&cell_exec},
                                             std::initializer_list<task_executor*>{&slot_exec}),
      std::make_unique<pcell_ue_executor_mapper>(std::initializer_list<task_executor*>{&ue_ctrl_exec},
                                                 std::initializer_list<task_executor*>{&ul_exec},
                                                 std::initializer_list<task_executor*>{&dl_exec}),
      ctrl_exec,
      ctrl_exec,
      ctrl_exec};
};

/// \brief Metrics collected from the results passed by the MAC to the lower layers.
struct phy_metrics {
  unsigned slot_count    = 0;
  unsigned slot_dl_count = 0;
  unsigned nof_dl_grants = 0;
  uint64_t nof_dl_bytes  = 0;
  unsigned slot_ul_count = 0;
  unsigned nof_ul_grants = 0;
  uint64_t nof_ul_bytes  = 0;

  unsigned total_subframes_elapsed(subcarrier_spacing scs) { return slot_count / get_nof_slots_per_subframe(scs); }

  double dl_mbps(subcarrier_spacing scs)
  {
    unsigned sim_time_msec = total_subframes_elapsed(scs);
    return nof_dl_bytes * 8 * 1.0e-6 / (sim_time_msec * 1.0e-3);
  }

  double ul_mbps(subcarrier_spacing scs)
  {
    unsigned sim_time_msec = total_subframes_elapsed(scs);
    return nof_ul_bytes * 8 * 1.0e-6 / (sim_time_msec * 1.0e-3);
  }
};

/// \brief Emulator of the PHY, FAPI and UE from the perspective of the DU-high. This class should be able to provide
/// the required UE signalling (e.g. HARQ ACKs) for the DU-high normal operation.
class phy_simulator : public mac_result_notifier, public mac_cell_result_notifier
{
public:
  mac_cell_control_information_handler* ctrl_info_handler = nullptr;

  std::mutex              mutex;
  std::condition_variable cvar;
  bool                    slot_ended = false;

  mac_dl_sched_result slot_dl_result;
  mac_ul_sched_result slot_ul_result;
  mac_dl_data_result  slot_dl_data_result;

  phy_metrics metrics;

  void reset_metrics() { metrics = {}; }

  void new_slot()
  {
    slot_dl_result.dl_res = nullptr;
    slot_ul_result.ul_res = nullptr;
  }

  mac_cell_result_notifier& get_cell(du_cell_index_t cell_index) override { return *this; }

  void on_new_downlink_scheduler_results(const mac_dl_sched_result& dl_res) override { slot_dl_result = dl_res; }

  /// Notifies scheduled PDSCH PDUs.
  void on_new_downlink_data(const mac_dl_data_result& dl_data) override { slot_dl_data_result = dl_data; }

  /// Notifies slot scheduled PUCCH/PUSCH grants.
  void on_new_uplink_scheduler_results(const mac_ul_sched_result& ul_res) override { slot_ul_result = ul_res; }

  /// \brief Notifies the completion of all cell results for the given slot.
  void on_cell_results_completion(slot_point slot) override
  {
    {
      std::lock_guard<std::mutex> lock(mutex);
      slot_ended = true;
    }
    cvar.notify_one();
  }

  /// \brief This function blocks waiting for DU-high to finish the handling of the last slot_indication.
  void wait_slot_complete()
  {
    std::unique_lock<std::mutex> lock(mutex);
    while (not slot_ended) {
      cvar.wait(lock);
    }
    slot_ended = false;
  }

  void process_results()
  {
    metrics.slot_count++;
    process_dl_results();
    process_ul_results();
  }

private:
  void process_dl_results()
  {
    if (slot_dl_result.dl_res == nullptr or slot_dl_result.dl_res->nof_dl_symbols == 0) {
      // Early return.
      return;
    }

    metrics.slot_dl_count++;
    metrics.nof_dl_grants += slot_dl_data_result.ue_pdus.size();
    for (const auto& pdu : slot_dl_data_result.ue_pdus) {
      metrics.nof_dl_bytes += pdu.pdu.size();
    }
  }

  void process_ul_results()
  {
    if (slot_ul_result.ul_res == nullptr or slot_ul_result.ul_res->nof_ul_symbols == 0) {
      // Early return.
      return;
    }

    metrics.slot_ul_count++;
    metrics.nof_ul_grants += slot_ul_result.ul_res->puschs.size();
    for (const ul_sched_info& pusch : slot_ul_result.ul_res->puschs) {
      metrics.nof_ul_bytes += pusch.pusch_cfg.tb_size_bytes;
    }
  }
};

/// \brief TestBench for the DU-high.
class du_high_bench
{
  static const unsigned DEFAULT_DL_PDU_SIZE = 1500;

public:
  du_high_bench(unsigned                          dl_buffer_state_bytes_,
                unsigned                          ul_bsr_bytes_,
                unsigned                          max_nof_rbs_per_dl_grant,
                units::bytes                      f1u_pdu_size_,
                span<unsigned>                    du_cell_cores,
                const cell_config_builder_params& builder_params = {}) :
    params(builder_params),
    f1u_dl_pdu_bytes_per_slot(dl_buffer_state_bytes_),
    f1u_pdu_size(f1u_pdu_size_),
    workers(du_cell_cores),
    ul_bsr_bytes(ul_bsr_bytes_)
  {
    // Set slot point based on the SCS.
    next_sl_tx = slot_point{to_numerology_value(params.scs_common), 0};

    // Compute LBSR buffer size. According to TS38.321, 254 is the maximum value for the LBSR size.
    unsigned lbsr_buff_sz = 0;
    for (; lbsr_buff_sz < 255; ++lbsr_buff_sz) {
      if (buff_size_field_to_bytes(lbsr_buff_sz, srsran::bsr_format::LONG_BSR) > ul_bsr_bytes) {
        break;
      }
    }
    // Append the LBSR buffer size to the BSR subPDU.
    report_fatal_error_if_not(bsr_mac_subpdu.append(lbsr_buff_sz), "Failed to allocate PDU");

    // Instantiate a DU-high object.
    cfg.gnb_du_id    = (gnb_du_id_t)1;
    cfg.gnb_du_name  = fmt::format("srsgnb{}", cfg.gnb_du_id);
    cfg.du_bind_addr = transport_layer_address::create_from_string(fmt::format("127.0.0.{}", cfg.gnb_du_id));
    cfg.exec_mapper  = &workers.du_high_exec_mapper;
    cfg.f1c_client   = &sim_cu_cp;
    cfg.f1u_gw       = &sim_cu_up;
    cfg.phy_adapter  = &sim_phy;
    cfg.timers       = &timers;
    cfg.cells        = {config_helpers::make_default_du_cell_config(params)};
    cfg.sched_cfg    = config_helpers::make_default_scheduler_expert_config();
    cfg.sched_cfg.ue.pdsch_nof_rbs = {1, max_nof_rbs_per_dl_grant};
    cfg.mac_cfg                    = mac_expert_config{.configs = {{10000, 10000, 10000}}};
    cfg.qos                        = config_helpers::make_default_du_qos_config_list(/* warn_on_drop */ true, 1000);
    cfg.mac_p                      = &mac_pcap;
    cfg.rlc_p                      = &rlc_pcap;
    cfg.sched_ue_metrics_notifier  = &metrics_handler;

    // Increase nof. PUCCH resources to accommodate more UEs.
    cfg.cells[0].pucch_cfg.nof_sr_resources             = 30;
    cfg.cells[0].pucch_cfg.nof_csi_resources            = 30;
    cfg.cells[0].pucch_cfg.nof_ue_pucch_f2_res_harq     = 8;
    cfg.cells[0].pucch_cfg.nof_ue_pucch_f1_res_harq     = 8;
    cfg.cells[0].pucch_cfg.nof_cell_harq_pucch_res_sets = 4;
    cfg.cells[0].pucch_cfg.f1_params.nof_cyc_shifts     = srsran::nof_cyclic_shifts::six;
    cfg.cells[0].pucch_cfg.f1_params.occ_supported      = true;
    cfg.sched_cfg.ue.max_pucchs_per_slot                = 61;
    cfg.sched_cfg.ue.max_puschs_per_slot                = 61;
    cfg.sched_cfg.ue.max_ul_grants_per_slot             = 64;

    du_hi = std::make_unique<du_high_impl>(cfg);

    // Create PDCP PDU.
    report_fatal_error_if_not(pdcp_pdu.append(test_rgen::random_vector<uint8_t>(f1u_pdu_size.value())),
                              "Unable to allocate PDU");
    // Create MAC PDU.
    report_fatal_error_if_not(mac_pdu.append(test_rgen::random_vector<uint8_t>(
                                  buff_size_field_to_bytes(lbsr_buff_sz, srsran::bsr_format::LONG_BSR))),
                              "Unable to allocate PDU");

    // Start DU-high operation.
    du_hi->start();

    // Connect PHY back to DU-High.
    sim_phy.ctrl_info_handler = &du_hi->get_control_info_handler(to_du_cell_index(0));
  }

  void stop()
  {
    du_hi->stop();
    workers.stop();
  }

  ~du_high_bench() { stop(); }

  static rnti_t du_ue_index_to_rnti(du_ue_index_t ue_idx) { return to_rnti(0x4601 + ue_idx); }

  /// \brief Run a slot indication until completion.
  void run_slot()
  {
    // Clear results.
    sim_phy.new_slot();

    // Push slot indication to DU-high.
    du_hi->get_slot_handler(to_du_cell_index(0)).handle_slot_indication(next_sl_tx);

    // Wait DU-high to finish handling the slot.
    sim_phy.wait_slot_complete();
  }

  template <typename StopCondition>
  bool run_slot_until(const StopCondition& cond_func, unsigned slot_timeout = 1000)
  {
    unsigned count = 0;
    for (; count < slot_timeout; ++count) {
      run_slot();
      process_results();
      if (cond_func()) {
        break;
      }
    }
    return count < slot_timeout;
  }

  void process_results()
  {
    process_pucch_grants();

    // Send any pending UCI indications.
    auto uci_ind = pending_ucis.begin();
    while (uci_ind != pending_ucis.end()) {
      if (uci_ind->sl_rx == next_sl_tx - tx_rx_delay) {
        du_hi->get_control_info_handler(to_du_cell_index(0)).handle_uci(*uci_ind);
        uci_ind = pending_ucis.erase(uci_ind);
        continue;
      }
      ++uci_ind;
    }

    // Process PUSCH grants.
    process_pusch_grants();

    // Send any pending Rx Data indications.
    auto rx_data_ind = pending_puschs.begin();
    while (rx_data_ind != pending_puschs.end()) {
      if (rx_data_ind->sl_rx == next_sl_tx - tx_rx_delay) {
        du_hi->get_pdu_handler().handle_rx_data_indication(*rx_data_ind);
        rx_data_ind = pending_puschs.erase(rx_data_ind);
        continue;
      }
      ++rx_data_ind;
    }

    // Push UL CRC indications for PUSCH grants.
    push_ul_crc_indication();

    // Send any pending CRC indications.
    auto crc_ind = pending_crc.begin();
    while (crc_ind != pending_crc.end()) {
      if (crc_ind->sl_rx == next_sl_tx - tx_rx_delay) {
        du_hi->get_control_info_handler(to_du_cell_index(0)).handle_crc(*crc_ind);
        crc_ind = pending_crc.erase(crc_ind);
        continue;
      }
      ++crc_ind;
    }

    // Process PHY metrics.
    sim_phy.process_results();

    // Advance slot.
    ++next_sl_tx;
  }

  /// \brief Add a UE to the DU-high and wait for the DU-high to finish the setup of the UE.
  void add_ue(du_ue_index_t ue_idx)
  {
    using namespace std::chrono_literals;

    // Wait until it's a full UL slot to send Msg3.
    auto next_ul_slot = [this]() {
      return not cfg.cells[to_du_cell_index(0)].tdd_ul_dl_cfg_common.has_value() or
             not is_tdd_full_ul_slot(cfg.cells[to_du_cell_index(0)].tdd_ul_dl_cfg_common.value(),
                                     slot_point(next_sl_tx - tx_rx_delay - 1).slot_index());
    };
    run_slot_until(next_ul_slot);

    // Received Msg3 with UL-CCCH message.
    mac_rx_data_indication rx_ind;
    rx_ind.sl_rx      = next_sl_tx - tx_rx_delay;
    rx_ind.cell_index = to_du_cell_index(0);
    rx_ind.pdus.push_back(
        mac_rx_pdu{du_ue_index_to_rnti(ue_idx),
                   0,
                   0,
                   byte_buffer::create({0x34, 0x1e, 0x4f, 0xc0, 0x4f, 0xa6, 0x06, 0x3f, 0x00, 0x00, 0x00}).value()});
    du_hi->get_pdu_handler().handle_rx_data_indication(std::move(rx_ind));

    // Wait for UE Context Modification Response to arrive to CU.
    while (not sim_cu_cp.ue_created_flag_list[ue_idx]) {
      // Need to run one slot for scheduler to handle pending events.
      run_slot();
      process_results();
      std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
  }

  // \brief Push a DL PDUs to DU-high via F1-U interface.
  void push_pdcp_pdus()
  {
    // Value of DL Buffer Occupancy at which we consider that the DU DL is saturated, and there is no point in pushing
    // more PDUs.
    static const size_t SATURATION_DL_BS_BYTES = 1e5;

    if (f1u_dl_pdu_bytes_per_slot == 0) {
      // Early return.
      return;
    }
    uint64_t bytes_to_sched = f1u_dl_total_bytes.load(std::memory_order_relaxed);
    bytes_to_sched -= std::min(bytes_to_sched, sim_phy.metrics.nof_dl_bytes);
    if (bytes_to_sched > SATURATION_DL_BS_BYTES) {
      // Saturation of the DU DL detected. We throttle the F1-U interface to avoid depleting the byte buffer pool.
      return;
    }

    while (not workers.dl_exec.defer([this]() {
      static std::array<uint32_t, MAX_NOF_DU_UES> pdcp_sn_list{0};
      const unsigned nof_dl_pdus_per_slot = divide_ceil(f1u_dl_pdu_bytes_per_slot, this->f1u_pdu_size.value());
      const unsigned last_dl_pdu_size     = f1u_dl_pdu_bytes_per_slot % this->f1u_pdu_size.value();

      // Forward DL buffer occupancy updates to all bearers in a Round-robin fashion.
      for (unsigned i = 0; i != nof_dl_pdus_per_slot; ++i) {
        unsigned    bearer_idx = f1u_dl_rr_count++ % sim_cu_up.du_notif_list.size();
        const auto& du_notif   = sim_cu_up.du_notif_list[bearer_idx];

        // Update PDCP SN.
        pdcp_sn_list[bearer_idx] = (pdcp_sn_list[bearer_idx] + 1) % (1U << 18U);
        // We perform a deep-copy of the byte buffer to better simulate a real deployment, where there is stress over
        // the byte buffer pool.
        auto pdu_copy = pdcp_pdu.deep_copy();
        if (pdu_copy.is_error()) {
          test_logger.warning("Byte buffer segment pool depleted");
          return;
        }
        if (i == nof_dl_pdus_per_slot - 1 and last_dl_pdu_size != 0) {
          // If it is last DL PDU.
          if (!pdu_copy.value().resize(last_dl_pdu_size)) {
            test_logger.warning("Unable to resize PDU to {} bytes", last_dl_pdu_size);
            return;
          }
        }
        f1u_dl_total_bytes.fetch_add(pdu_copy.value().length(), std::memory_order_relaxed);
        du_notif->on_new_pdu(nru_dl_message{.t_pdu = std::move(pdu_copy.value())});
      }
    })) {
      // keep trying to push new PDUs.
      // Sleeping in the test main thread will also slow down the rate at which slot indications reach the MAC. This
      // is ok because in a real RT deployment the slot indication rate is limited by the radio.
      std::this_thread::sleep_for(std::chrono::microseconds{500});
    }
  }

  // \brief Process PUCCH grants and send UCI indication based on the grants.
  void process_pucch_grants()
  {
    // Early return.
    if (sim_phy.slot_ul_result.ul_res == nullptr) {
      return;
    }

    // Forwards HARQ-ACKs to the DU-High given the scheduled PUCCHs.
    mac_uci_indication_message uci{};
    uci.sl_rx = sim_phy.slot_ul_result.slot;
    for (const pucch_info& pucch : sim_phy.slot_ul_result.ul_res->pucchs) {
      mac_uci_pdu& uci_pdu = uci.ucis.emplace_back();
      uci_pdu.rnti         = pucch.crnti;
      switch (pucch.format) {
        case pucch_format::FORMAT_1: {
          mac_uci_pdu::pucch_f0_or_f1_type f1{};
          if (pucch.format_1.harq_ack_nof_bits > 0) {
            f1.harq_info.emplace();
            // Set PUCCHs with SR as DTX.
            const uci_pucch_f0_or_f1_harq_values ack_val = pucch.format_1.sr_bits == sr_nof_bits::no_sr
                                                               ? uci_pucch_f0_or_f1_harq_values::ack
                                                               : uci_pucch_f0_or_f1_harq_values::dtx;
            f1.harq_info->harqs.resize(pucch.format_1.harq_ack_nof_bits, ack_val);
          }
          // Forward positive SRs to scheduler only if UL is enabled for the benchmark, PUCCH grant is for SR and nof.
          // UL grants is 0 or scheduler stops allocating UL grants.
          if (ul_bsr_bytes != 0 and pucch.format_1.sr_bits != sr_nof_bits::no_sr and
              (sim_phy.metrics.nof_ul_grants == 0 or
               (sim_phy.metrics.nof_ul_grants ==
                sim_phy.metrics.nof_ul_grants + sim_phy.slot_ul_result.ul_res->puschs.size()))) {
            f1.sr_info.emplace();
            f1.sr_info->detected = true;
          }
          uci_pdu.pdu = f1;
        } break;
        case pucch_format::FORMAT_2: {
          mac_uci_pdu::pucch_f2_or_f3_or_f4_type f2{};
          if (pucch.format_2.harq_ack_nof_bits > 0) {
            f2.harq_info.emplace();
            f2.harq_info->is_valid = true;
            f2.harq_info->payload.resize(pucch.format_2.harq_ack_nof_bits);
            f2.harq_info->payload.fill(0, pucch.format_2.harq_ack_nof_bits, true);
          }
          // Forward positive SRs to scheduler only if UL is enabled for the benchmark, PUCCH grant is for SR and nof.
          // UL grants is 0 or scheduler stops allocating UL grants.
          if (ul_bsr_bytes != 0 and pucch.format_2.sr_bits != sr_nof_bits::no_sr and
              (sim_phy.metrics.nof_ul_grants == 0 or
               (sim_phy.metrics.nof_ul_grants ==
                sim_phy.metrics.nof_ul_grants + sim_phy.slot_ul_result.ul_res->puschs.size()))) {
            f2.sr_info.emplace();
            f2.sr_info->resize(sr_nof_bits_to_uint(pucch.format_2.sr_bits));
            f2.sr_info->fill(0, sr_nof_bits_to_uint(pucch.format_2.sr_bits), true);
          }
          if (pucch.csi_rep_cfg.has_value()) {
            f2.csi_part1_info.emplace();
            f2.csi_part1_info->is_valid = true;
            f2.csi_part1_info->payload.resize(4);
            f2.csi_part1_info->payload.fill(0, 4, true);
          }
          uci_pdu.pdu = f2;
        } break;
        default:
          report_fatal_error("PUCCH format not supported");
      }
    }
    if (not uci.ucis.empty()) {
      pending_ucis.push_back(std::move(uci));
    }
  }

  // \brief Process PUSCH grants and send MAC Rx data indication based on the grants.
  void process_pusch_grants()
  {
    // Early return.
    if (ul_bsr_bytes == 0) {
      return;
    }

    // Early return.
    if (sim_phy.slot_ul_result.ul_res == nullptr) {
      return;
    }

    // Forwards UCI to the DU-High given the scheduled PUSCHs.
    mac_uci_indication_message uci_ind{};
    uci_ind.sl_rx = sim_phy.slot_ul_result.slot;

    mac_rx_data_indication rx_ind{};
    rx_ind.sl_rx      = sim_phy.slot_ul_result.slot;
    rx_ind.cell_index = to_du_cell_index(0);
    for (const ul_sched_info& pusch : sim_phy.slot_ul_result.ul_res->puschs) {
      unsigned payload_len = std::min(mac_pdu.length(), static_cast<size_t>(pusch.pusch_cfg.tb_size_bytes));

      // 1 Byte for LCID and other fields and 1/2 Byte(s) for payload length.
      static const uint8_t mac_header_size = payload_len > 255 ? 3 : 2;

      // Early return - the TB is too small.
      if (pusch.pusch_cfg.tb_size_bytes <= mac_header_size + bsr_mac_subpdu.length()) {
        return;
      }

      // Prepare MAC SDU for LCID 4.
      static const lcid_t drb_lcid = uint_to_lcid(4);
      mac_rx_pdu          rx_pdu{pusch.pusch_cfg.rnti, 0, pusch.pusch_cfg.harq_id, {}};
      // Pack header and payload length.
      // Subtract BSR length.
      payload_len -= mac_header_size + bsr_mac_subpdu.length();
      if (payload_len > 255) {
        report_fatal_error_if_not(rx_pdu.pdu.append(0x40 | drb_lcid), "Failed to allocate PDU");
        report_fatal_error_if_not(rx_pdu.pdu.append((payload_len & 0xff00) >> 8), "Failed to allocate PDU");
        report_fatal_error_if_not(rx_pdu.pdu.append(payload_len & 0x00ff), "Failed to allocate PDU");
      } else {
        report_fatal_error_if_not(rx_pdu.pdu.append(drb_lcid), "Failed to allocate PDU");
        report_fatal_error_if_not(rx_pdu.pdu.append(payload_len & 0x00ff), "Failed to allocate PDU");
      }
      static const uint8_t rlc_um_complete_pdu_header = 0x00;
      report_fatal_error_if_not(rx_pdu.pdu.append(rlc_um_complete_pdu_header), "Failed to allocate PDU");
      // Exclude RLC header from payload length.
      report_fatal_error_if_not(rx_pdu.pdu.append(mac_pdu.begin(), mac_pdu.begin() + (payload_len - 1)),
                                "Failed to allocate PDU");
      // Append Long BSR bytes.
      report_fatal_error_if_not(rx_pdu.pdu.append(bsr_mac_subpdu.begin(), bsr_mac_subpdu.end()),
                                "Failed to allocate PDU");

      rx_ind.pdus.push_back(rx_pdu);

      // Remove PUCCH UCI if UCI mltplxd on PUSCH.
      if (pusch.uci.has_value()) {
        // Remove PUCCH UCI if UCI mltplxd on PUSCH.
        auto pending_uci_it = pending_ucis.begin();
        while (pending_uci_it != pending_ucis.end()) {
          if (pending_uci_it->sl_rx == rx_ind.sl_rx) {
            auto* pending_uci_pdu_it = pending_uci_it->ucis.begin();
            while (pending_uci_pdu_it != pending_uci_it->ucis.end()) {
              if (pending_uci_pdu_it->rnti == pusch.pusch_cfg.rnti) {
                pending_uci_pdu_it = pending_uci_it->ucis.erase(pending_uci_pdu_it);
                continue;
              }
              ++pending_uci_pdu_it;
            }
            if (pending_uci_it->ucis.empty()) {
              pending_uci_it = pending_ucis.erase(pending_uci_it);
              continue;
            }
          }
          ++pending_uci_it;
        }

        // Build UCI PDU on PUSCH.
        mac_uci_pdu& uci_pdu = uci_ind.ucis.emplace_back();
        uci_pdu.rnti         = pusch.pusch_cfg.rnti;
        mac_uci_pdu::pusch_type push_uci{};
        if (pusch.uci->harq->harq_ack_nof_bits > 0) {
          push_uci.harq_info.emplace();
          push_uci.harq_info->is_valid = true;
          push_uci.harq_info->payload.resize(pusch.uci->harq->harq_ack_nof_bits);
          push_uci.harq_info->payload.fill(0, pusch.uci->harq->harq_ack_nof_bits, true);
        }
        if (pusch.uci->csi->csi_part1_nof_bits > 0) {
          push_uci.csi_part1_info.emplace();
          push_uci.csi_part1_info->is_valid = true;
          push_uci.csi_part1_info->payload.resize(pusch.uci->csi->csi_part1_nof_bits);
          push_uci.csi_part1_info->payload.fill(0, pusch.uci->csi->csi_part1_nof_bits, true);
        }
        uci_pdu.pdu = push_uci;
      }
    }
    if (not rx_ind.pdus.empty()) {
      pending_puschs.push_back(std::move(rx_ind));
    }
    if (not uci_ind.ucis.empty()) {
      pending_ucis.push_back(std::move(uci_ind));
    }
  }

  // \brief Push a UL CRC indication to DU-high.
  void push_ul_crc_indication()
  {
    // Early return.
    if (ul_bsr_bytes == 0) {
      return;
    }

    // Early return.
    if (sim_phy.slot_ul_result.ul_res == nullptr) {
      return;
    }

    mac_crc_indication_message crc{};
    crc.sl_rx = sim_phy.slot_ul_result.slot;
    for (const ul_sched_info& pusch : sim_phy.slot_ul_result.ul_res->puschs) {
      mac_crc_pdu& crc_pdu   = crc.crcs.emplace_back();
      crc_pdu.rnti           = pusch.pusch_cfg.rnti;
      crc_pdu.harq_id        = pusch.pusch_cfg.harq_id;
      crc_pdu.tb_crc_success = true;
      crc_pdu.ul_sinr_dB     = 21.0;
    }
    if (not sim_phy.slot_ul_result.ul_res->puschs.empty()) {
      pending_crc.push_back(std::move(crc));
    }
  }

  const unsigned             tx_rx_delay = 4;
  cell_config_builder_params params;
  du_high_configuration      cfg{};
  /// Size of the DL buffer status to push for DL Tx.
  unsigned     f1u_dl_pdu_bytes_per_slot;
  units::bytes f1u_pdu_size{DEFAULT_DL_PDU_SIZE};

  srslog::basic_logger&              test_logger = srslog::fetch_basic_logger("TEST");
  dummy_metrics_handler              metrics_handler;
  cu_cp_simulator                    sim_cu_cp;
  cu_up_simulator                    sim_cu_up;
  phy_simulator                      sim_phy;
  timer_manager                      timers;
  du_high_single_cell_worker_manager workers;
  std::unique_ptr<du_high_impl>      du_hi;
  slot_point                         next_sl_tx{0, 0};
  null_mac_pcap                      mac_pcap;
  null_rlc_pcap                      rlc_pcap;

  /// Queue of MAC UCI indication message to be sent in their expected receive slot.
  std::deque<mac_uci_indication_message> pending_ucis;
  /// Queue of MAC Rx data indication message to be sent in their expected receive slot.
  std::deque<mac_rx_data_indication> pending_puschs;
  /// Queue of MAC CRC indication message to be sent in their expected receive slot.
  std::deque<mac_crc_indication_message> pending_crc;

  byte_buffer pdcp_pdu;
  byte_buffer mac_pdu;

  // - 8-bit R/LCID MAC subheader.
  // - MAC CE with Long BSR.
  //
  // |   |   |   |   |   |   |   |   |
  // | R | R |         LCID          |  Octet 1
  // |              L                |  Octet 2
  // | LCG7 | LCG6 |    ...   | LCG0 |  Octet 3
  // |         Buffer Size 1         |  Octet 4

  // Construct LBSR MAC subPDU for LCG 1.
  // NOTE: LBSR buffer size is populated in the constructor.
  byte_buffer bsr_mac_subpdu = byte_buffer::create({0x3e, 0x02, 0x02}).value();

  /// Size of the UL Buffer status report to push for UL Tx.
  unsigned ul_bsr_bytes;

  // Round-robin indexer for pushing DL PDUs to attached UEs.
  unsigned f1u_dl_rr_count = 0;
  // Sum of total F1-U DL bytes pushed into DU.
  std::atomic<uint64_t> f1u_dl_total_bytes{0};
};

/// \brief Generate custom cell configuration builder params based on duplex mode.
static cell_config_builder_params generate_custom_cell_config_builder_params(duplex_mode dplx_mode)
{
  cell_config_builder_params params{};
  params.scs_common = dplx_mode == duplex_mode::FDD ? subcarrier_spacing::kHz15 : subcarrier_spacing::kHz30;
  params.dl_arfcn   = dplx_mode == duplex_mode::FDD ? 530000 : 520002;
  params.band       = band_helper::get_band_from_dl_arfcn(params.dl_arfcn);
  params.channel_bw_mhz =
      dplx_mode == duplex_mode::FDD ? srsran::bs_channel_bandwidth_fr1::MHz20 : bs_channel_bandwidth_fr1::MHz100;
  const unsigned nof_crbs = band_helper::get_n_rbs_from_bw(
      params.channel_bw_mhz, params.scs_common, band_helper::get_freq_range(*params.band));
  static const uint8_t                                   ss0_idx = 0;
  std::optional<band_helper::ssb_coreset0_freq_location> ssb_freq_loc =
      band_helper::get_ssb_coreset0_freq_location(params.dl_arfcn,
                                                  *params.band,
                                                  nof_crbs,
                                                  params.scs_common,
                                                  params.scs_common,
                                                  ss0_idx,
                                                  params.max_coreset0_duration);
  if (!ssb_freq_loc.has_value()) {
    report_error("Unable to derive a valid SSB pointA and k_SSB for cell id ({}).\n", params.pci);
  }
  params.offset_to_point_a   = (*ssb_freq_loc).offset_to_point_A;
  params.k_ssb               = (*ssb_freq_loc).k_ssb;
  params.coreset0_index      = (*ssb_freq_loc).coreset0_idx;
  params.search_space0_index = ss0_idx;

  if (dplx_mode == duplex_mode::TDD) {
    params.tdd_ul_dl_cfg_common.emplace();
    params.tdd_ul_dl_cfg_common->ref_scs                            = subcarrier_spacing::kHz30;
    params.tdd_ul_dl_cfg_common->pattern1.dl_ul_tx_period_nof_slots = 10;
    params.tdd_ul_dl_cfg_common->pattern1.nof_dl_slots              = 7;
    params.tdd_ul_dl_cfg_common->pattern1.nof_ul_slots              = 2;
  }

  return params;
}

/// \brief Benchmark DU-high with DL and/or UL only traffic using an RLC UM bearer.
void benchmark_dl_ul_only_rlc_um(benchmarker&   bm,
                                 unsigned       nof_ues,
                                 duplex_mode    dplx_mode,
                                 unsigned       dl_buffer_state_bytes,
                                 unsigned       ul_bsr_bytes,
                                 unsigned       max_nof_rbs_per_dl_grant,
                                 units::bytes   dl_pdu_size,
                                 span<unsigned> du_cell_cores)
{
  auto                benchname = fmt::format("{}{}{}, {} UEs, RLC UM",
                               dl_buffer_state_bytes > 0 ? "DL" : "",
                               std::min(dl_buffer_state_bytes, ul_bsr_bytes) > 0 ? "+" : "",
                               ul_bsr_bytes > 0 ? "UL" : "",
                               nof_ues);
  test_delimit_logger test_delim(benchname.c_str());
  du_high_bench       bench{dl_buffer_state_bytes,
                      ul_bsr_bytes,
                      max_nof_rbs_per_dl_grant,
                      dl_pdu_size,
                      du_cell_cores,
                      generate_custom_cell_config_builder_params(dplx_mode)};
  for (unsigned ue_count = 0; ue_count < nof_ues; ++ue_count) {
    bench.add_ue(to_du_ue_index(ue_count));
  }

  // Start of the benchmark. Reset metrics collected so far.
  bench.sim_phy.reset_metrics();

  // Run benchmark.
  bm.new_measure(
      benchname,
      1,
      [&bench]() mutable {
        // Run slot to completion.
        bench.run_slot();
      },
      [&bench]() {
        // Push DL PDUs.
        bench.push_pdcp_pdus();

        // Advance slot
        bench.process_results();
      });

  // Stop benchmark.
  bench.stop();
  srslog::flush();

  fmt::print("\nStats: #slots={}, #PDSCHs={}, #PDSCHs-per-slot={:.3}, dl_bitrate={:.3} Mbps, #PUSCHs={}, "
             "#PUSCHs-per-slot={:.3}, ul_bitrate={:.3} Mbps\n",
             bench.sim_phy.metrics.slot_count,
             bench.sim_phy.metrics.nof_dl_grants,
             bench.sim_phy.metrics.nof_dl_grants / (double)bench.sim_phy.metrics.slot_dl_count,
             bench.sim_phy.metrics.dl_mbps(bench.cfg.cells[0].scs_common),
             bench.sim_phy.metrics.nof_ul_grants,
             bench.sim_phy.metrics.nof_ul_grants / (double)bench.sim_phy.metrics.slot_ul_count,
             bench.sim_phy.metrics.ul_mbps(bench.cfg.cells[0].scs_common));
}

/// \brief Configure main thread priority and affinity to avoid interference from other processes (including stressors).
static void configure_main_thread(span<const unsigned> du_cell_cores)
{
  pthread_t self = pthread_self();

  int prio_level = ::sched_get_priority_max(SCHED_FIFO);
  if (prio_level == -1) {
    fmt::print("Warning: Unable to get the max thread priority. Falling back to normal priority.");
    return;
  }
  // set priority to -1 less than RT to avoid interfering with kernel.
  ::sched_param sch{prio_level - 1};
  if (::pthread_setschedparam(self, SCHED_FIFO, &sch)) {
    fmt::print("Warning: Unable to set the test thread priority to max. Falling back to normal priority.");
    return;
  }

  // Set main test thread to use same cores as du_cell.
  if (not du_cell_cores.empty()) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (unsigned i : du_cell_cores) {
      CPU_SET(i, &cpuset);
    }
    int ret;
    if ((ret = pthread_setaffinity_np(self, sizeof(cpuset), &cpuset)) != 0) {
      fmt::print("Warning: Unable to set affinity for test thread. Cause: '{}'\n", strerror(ret));
      return;
    }
  }
}

int main(int argc, char** argv)
{
  static const std::size_t byte_buffer_nof_segments = 1U << 19U;
  static const std::size_t byte_buffer_segment_size = 2048;

  // Set DU-high logging.
  auto test_log_level = srslog::basic_levels::warning;
  srslog::fetch_basic_logger("TEST").set_level(test_log_level);
  srslog::fetch_basic_logger("RLC").set_level(test_log_level);
  srslog::fetch_basic_logger("MAC", true).set_level(test_log_level);
  srslog::fetch_basic_logger("SCHED", true).set_level(test_log_level);
  srslog::fetch_basic_logger("DU-F1").set_level(test_log_level);
  srslog::fetch_basic_logger("DU-F1-U").set_level(test_log_level);
  srslog::fetch_basic_logger("UE-MNG").set_level(test_log_level);
  srslog::fetch_basic_logger("DU-MNG").set_level(test_log_level);
  srslog::init();

  std::string tracing_filename = "";
  if (not tracing_filename.empty()) {
    fmt::print("Opening event tracer...\n");
    open_trace_file(tracing_filename);
    fmt::print("Event tracer opened successfully\n");
  }

  // Parses benchmark parameters.
  bench_params params{};
  parse_args(argc, argv, params);
  print_args(params);

  // Setup size of byte buffer pool.
  init_byte_buffer_segment_pool(byte_buffer_nof_segments, byte_buffer_segment_size);

  // Configure main thread.
  configure_main_thread(params.du_cell_cores);

  // Start benchmarker.
  benchmarker bm("DU-High", params.nof_repetitions);

  // Run scenarios.
  const unsigned nof_scenarios = params.nof_ues.size();

  for (unsigned sim_idx = 0; sim_idx != nof_scenarios; ++sim_idx) {
    benchmark_dl_ul_only_rlc_um(bm,
                                params.nof_ues[sim_idx],
                                params.dplx_mode,
                                params.dl_bytes_per_slot,
                                params.ul_bsr_bytes,
                                params.max_dl_rb_grant,
                                params.pdu_size,
                                params.du_cell_cores);
  }

  if (not tracing_filename.empty()) {
    fmt::print("Closing event tracer...\n");
    close_trace_file();
    fmt::print("Event tracer closed successfully\n");
  }

  // Output results.
  bm.print_percentiles_time();
}
