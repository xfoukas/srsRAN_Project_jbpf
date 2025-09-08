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

#include "lib/pdcp/pdcp_entity_tx.h"
#include "pdcp_test_vectors.h"
#include "srsran/pdcp/pdcp_config.h"
#include "srsran/pdcp/pdcp_tx.h"
#include "srsran/support/executors/manual_task_worker.h"
#include "srsran/support/timers.h"
#include <cstdlib>
#include <getopt.h>
#include <queue>

using namespace srsran;

struct pdcp_gen_helper_args {
  std::string sn_size = {};
  uint32_t    count   = {};
  unsigned    algo    = 1;
  unsigned    rb_type = 0;
};

/// Mocking class of the surrounding layers invoked by the PDCP.
class pdcp_tx_gen_frame : public pdcp_tx_lower_notifier, public pdcp_tx_upper_control_notifier
{
public:
  std::queue<byte_buffer> pdu_queue   = {};
  std::queue<byte_buffer> retx_queue  = {};
  uint32_t                pdu_counter = 0;

  /// PDCP TX upper layer control notifier
  void on_max_count_reached() final {}
  void on_protocol_failure() final {}

  /// PDCP TX lower layer data notifier
  void on_new_pdu(byte_buffer pdu, bool is_retx) final
  {
    if (is_retx) {
      retx_queue.push(std::move(pdu));
    } else {
      pdu_queue.push(std::move(pdu));
    }
  }

  void on_discard_pdu(uint32_t pdcp_sn) final {}
};

bool parse_args(pdcp_gen_helper_args& args, int argc, char* argv[])
{
  static const struct option long_options[] = {{"help", no_argument, nullptr, 'h'},
                                               {"sn_size", required_argument, nullptr, 's'},
                                               {"count", required_argument, nullptr, 'c'},
                                               {"algo", required_argument, nullptr, 'a'},
                                               {nullptr, 0, nullptr, 0}};

  static const char usage[] = "Usage: pdcp_gen_helper [options]\n"
                              "\n"
                              "  -h, --help                 Show help message and quit.\n"
                              "  -s, --sn_size <SN size>    Specify 12bit, or 18bit\n"
                              "  -c, --count <COUNT>        Specify COUNT of PDU to generate\n"
                              "  -a, --algo <algo>          Specify ciphering and integrity algorithm\n"
                              "  -r, --rb_type <rb_type>    Specify RB type. 0 means SRB, 1 DRB\n"
                              "\n";
  // Parse arguments
  while (true) {
    int option_index = 0;
    int c            = getopt_long(argc, argv, "hs:c:a:r:", long_options, &option_index);
    if (c == -1) {
      break;
    }

    switch (c) {
      case 'h':
        fprintf(stdout, "%s", usage);
        break;
      case 's':
        args.sn_size = std::string(optarg);
        fprintf(stdout, "PDCP SN Size %s\n", args.sn_size.c_str());
        break;
      case 'c':
        args.count = strtod(optarg, nullptr);
        fprintf(stdout, "PDCP COUNT %u\n", args.count);
        break;
      case 'a':
        args.algo = strtod(optarg, nullptr);
        fprintf(stdout, "PDCP NIA%u/NEA%u\n", args.algo, args.algo);
        break;
      case 'r':
        args.rb_type = strtod(optarg, nullptr);
        fprintf(stdout, "PDCP %s\n", args.rb_type == 0 ? "SRB" : "DRB");
        break;
      default:
        fprintf(stderr, "error parsing arguments\n");
        return false;
    }
  }
  return true;
}

int main(int argc, char** argv)
{
  pdcp_gen_helper_args args;
  parse_args(args, argc, argv);

  srslog::init();
  srslog::basic_logger& logger = srslog::fetch_basic_logger("PDCP", false);
  logger.set_level(srslog::basic_levels::debug);
  logger.set_hex_dump_max_size(1500);

  pdcp_sn_size sn_size = args.sn_size == "12" ? pdcp_sn_size::size12bits : pdcp_sn_size::size18bits;
  logger.info("Creating PDCP TX ({} bit)", sn_size);

  timer_manager      timers;
  manual_task_worker worker{64};

  // Set TX config
  pdcp_tx_config config = {};
  config.rb_type        = args.rb_type == 0 ? pdcp_rb_type::srb : pdcp_rb_type::drb;
  config.rlc_mode       = pdcp_rlc_mode::am;
  config.sn_size        = sn_size;
  config.direction      = pdcp_security_direction::downlink;
  if (args.rb_type == 1) {
    config.discard_timer = pdcp_discard_timer::ms10;
  }
  config.status_report_required = false;

  security::sec_128_as_config sec_cfg = {};

  // Set security domain
  sec_cfg.domain = args.rb_type == 0 ? security::sec_domain::rrc : security::sec_domain::up;

  // Set security keys
  sec_cfg.k_128_int = k_128_int;
  sec_cfg.k_128_enc = k_128_enc;

  // Set encription/integrity algorithms
  sec_cfg.integ_algo  = static_cast<security::integrity_algorithm>(args.algo);
  sec_cfg.cipher_algo = static_cast<security::ciphering_algorithm>(args.algo);

  pdcp_tx_gen_frame                        frame = {};
  std::unique_ptr<pdcp_metrics_aggregator> metrics_agg =
      std::make_unique<pdcp_metrics_aggregator>(0, drb_id_t::drb1, timer_duration{100}, nullptr, worker);
  // Create PDCP entities
  std::unique_ptr<pdcp_entity_tx> pdcp_tx = std::make_unique<pdcp_entity_tx>(
      0, drb_id_t::drb1, config, frame, frame, timer_factory{timers, worker}, worker, worker, *metrics_agg);
  pdcp_tx_state st = {args.count, args.count, args.count};
  pdcp_tx->set_state(st);
  pdcp_tx->configure_security(sec_cfg, security::integrity_enabled::on, security::ciphering_enabled::on);

  // Write SDU
  byte_buffer sdu = byte_buffer::create(sdu1).value();
  pdcp_tx->handle_sdu(std::move(sdu));
  logger.info(frame.pdu_queue.front().begin(),
              frame.pdu_queue.front().end(),
              "PDCP PDU. pdu_len={}",
              frame.pdu_queue.front().length());
  return 0;
}
