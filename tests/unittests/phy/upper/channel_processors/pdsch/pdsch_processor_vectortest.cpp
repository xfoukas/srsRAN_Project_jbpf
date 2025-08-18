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

#include "pdsch_processor_test_data.h"
#include "pdsch_processor_test_doubles.h"
#include "srsran/phy/support/support_factories.h"
#include "srsran/phy/upper/channel_processors/channel_processor_formatters.h"
#include "srsran/phy/upper/channel_processors/pdsch/factories.h"
#include "srsran/phy/upper/channel_processors/pdsch/formatters.h"
#include "srsran/ran/pdsch/pdsch_constants.h"
#include "srsran/support/executors/task_worker_pool.h"
#ifdef HWACC_PDSCH_ENABLED
#include "srsran/hal/dpdk/bbdev/bbdev_acc.h"
#include "srsran/hal/dpdk/bbdev/bbdev_acc_factory.h"
#include "srsran/hal/dpdk/dpdk_eal_factory.h"
#include "srsran/hal/phy/upper/channel_processors/hw_accelerator_factories.h"
#include "srsran/hal/phy/upper/channel_processors/hw_accelerator_pdsch_enc_factory.h"
#endif // HWACC_PDSCH_ENABLED
#include "fmt/ostream.h"
#include <gtest/gtest.h>

using namespace srsran;

static std::string eal_arguments = "pdsch_processor_vectortest";
#ifdef HWACC_PDSCH_ENABLED
static bool                            skip_hwacc_test = false;
static std::unique_ptr<dpdk::dpdk_eal> dpdk_interface  = nullptr;

// Separates EAL and non-EAL arguments.
// The function assumes that 'eal_arg' flags the start of the EAL arguments and that no more non-EAL arguments follow.
static std::string capture_eal_args(int* argc, char*** argv)
{
  // Searchs for the EAL args (if any), flagged by 'eal_args', while removing all the rest (except argv[0]).
  bool        eal_found = false;
  char**      mod_argv  = *argv;
  std::string eal_argv  = {mod_argv[0]};
  int         opt_ind   = *argc;
  for (int j = 1; j < opt_ind; ++j) {
    // Search for the 'eal_args' flag (if any).
    if (!eal_found) {
      if (strcmp(mod_argv[j], "eal_args") == 0) {
        // 'eal_args' flag found.
        eal_found = true;
        // Remove all main app arguments starting from that point, while copying them to the EAL argument string.
        mod_argv[j] = NULL;
        for (int k = j + 1; k < opt_ind; ++k) {
          eal_argv += " ";
          eal_argv += mod_argv[k];
          mod_argv[k] = NULL;
        }
        *argc = j;
      }
    }
  }
  *argv = mod_argv;

  fmt::print("eal_arg={}\n", eal_argv);

  return eal_argv;
}
#endif // HWACC_PDSCH_ENABLED

namespace srsran {

std::ostream& operator<<(std::ostream& os, const test_case_t& test_case)
{
  fmt::print(os, "{}", test_case.context.pdu);
  return os;
}

namespace {

using PdschProcessorParams = std::tuple<std::string, test_case_t>;

// Number of concurrent threads.
static constexpr unsigned NOF_CONCURRENT_THREADS = 16;
static constexpr unsigned CB_BATCH_LENGTH        = 4;

class PdschProcessorFixture : public ::testing::TestWithParam<PdschProcessorParams>
{
private:
  static std::shared_ptr<pdsch_encoder_factory>
  create_generic_pdsch_encoder_factory(std::shared_ptr<crc_calculator_factory> crc_calculator_factory)
  {
    std::shared_ptr<ldpc_encoder_factory> ldpc_encoder_factory = create_ldpc_encoder_factory_sw("generic");
    TESTASSERT(ldpc_encoder_factory);

    std::shared_ptr<ldpc_rate_matcher_factory> ldpc_rate_matcher_factory = create_ldpc_rate_matcher_factory_sw();
    TESTASSERT(ldpc_rate_matcher_factory);

    std::shared_ptr<ldpc_segmenter_tx_factory> segmenter_factory =
        create_ldpc_segmenter_tx_factory_sw(crc_calculator_factory);
    TESTASSERT(segmenter_factory);

    pdsch_encoder_factory_sw_configuration encoder_factory_config;
    encoder_factory_config.encoder_factory      = ldpc_encoder_factory;
    encoder_factory_config.rate_matcher_factory = ldpc_rate_matcher_factory;
    encoder_factory_config.segmenter_factory    = segmenter_factory;
    return create_pdsch_encoder_factory_sw(encoder_factory_config);
  }

  static std::shared_ptr<hal::hw_accelerator_pdsch_enc_factory>
  create_hw_accelerator_pdsch_enc_factory(const std::string& eal_arguments)
  {
#ifdef HWACC_PDSCH_ENABLED
    //  Hardcoded stdout and error logging.
    srslog::sink* log_sink = srslog::create_stdout_sink();
    srslog::set_default_sink(*log_sink);
    srslog::init();
    srslog::basic_logger& logger = srslog::fetch_basic_logger("HAL", false);
    logger.set_level(srslog::basic_levels::error);

    // Pointer to a dpdk-based hardware-accelerator interface.
    if (!dpdk_interface && !skip_hwacc_test) {
      // :TODO: Enable passing EAL arguments.
      dpdk_interface = dpdk::create_dpdk_eal(eal_arguments, logger);
      if (!dpdk_interface) {
        skip_hwacc_test = true;
        return nullptr;
      }
    } else if (skip_hwacc_test) {
      return nullptr;
    }

    // Intefacing to the bbdev-based hardware-accelerator.
    dpdk::bbdev_acc_configuration bbdev_config;
    bbdev_config.id                                    = 0;
    bbdev_config.nof_ldpc_enc_lcores                   = NOF_CONCURRENT_THREADS;
    bbdev_config.nof_ldpc_dec_lcores                   = 0;
    bbdev_config.nof_fft_lcores                        = 0;
    bbdev_config.nof_mbuf                              = static_cast<unsigned>(pow2(log2_ceil(MAX_NOF_SEGMENTS)));
    std::shared_ptr<dpdk::bbdev_acc> bbdev_accelerator = create_bbdev_acc(bbdev_config, logger);
    if (!bbdev_accelerator || skip_hwacc_test) {
      skip_hwacc_test = true;
      return nullptr;
    }

    // Set the PDSCH encoder hardware-accelerator factory configuration for the ACC100.
    hal::bbdev_hwacc_pdsch_enc_factory_configuration hw_encoder_config;
    hw_encoder_config.acc_type          = "acc100";
    hw_encoder_config.bbdev_accelerator = bbdev_accelerator;
    hw_encoder_config.cb_mode           = false;
    hw_encoder_config.max_tb_size       = RTE_BBDEV_LDPC_E_MAX_MBUF;
    hw_encoder_config.dedicated_queue   = true;

    // ACC100 hardware-accelerator implementation.
    return srsran::hal::create_bbdev_pdsch_enc_acc_factory(hw_encoder_config);
#else  // HWACC_PDSCH_ENABLED
    return nullptr;
#endif // HWACC_PDSCH_ENABLED
  }

  static std::shared_ptr<pdsch_encoder_factory>
  create_acc100_pdsch_encoder_factory(std::shared_ptr<crc_calculator_factory> crc_calculator_factory,
                                      const std::string&                      eal_arguments)
  {
    std::shared_ptr<ldpc_segmenter_tx_factory> segmenter_factory =
        create_ldpc_segmenter_tx_factory_sw(crc_calculator_factory);
    if (!segmenter_factory) {
      return nullptr;
    }

    std::shared_ptr<hal::hw_accelerator_pdsch_enc_factory> hw_encoder_factory =
        create_hw_accelerator_pdsch_enc_factory(eal_arguments);
    if (!hw_encoder_factory) {
      return nullptr;
    }

    // Set the hardware-accelerated PDSCH encoder configuration.
    pdsch_encoder_factory_hw_configuration encoder_hw_factory_config;
    encoder_hw_factory_config.crc_factory        = crc_calculator_factory;
    encoder_hw_factory_config.segmenter_factory  = segmenter_factory;
    encoder_hw_factory_config.hw_encoder_factory = hw_encoder_factory;
    return create_pdsch_encoder_factory_hw(encoder_hw_factory_config);
  }

  static std::shared_ptr<pdsch_encoder_factory>
  create_pdsch_encoder_factory(std::shared_ptr<crc_calculator_factory> crc_calculator_factory,
                               const std::string&                      encoder_type,
                               const std::string&                      eal_arguments)
  {
    if (encoder_type == "generic") {
      return create_generic_pdsch_encoder_factory(crc_calculator_factory);
    }

    if (encoder_type == "acc100") {
      return create_acc100_pdsch_encoder_factory(crc_calculator_factory, eal_arguments);
    }

    return nullptr;
  }

  std::shared_ptr<pdsch_processor_factory> create_pdsch_processor_factory(const std::string& factory_type,
                                                                          const std::string& eal_arguments)
  {
    std::string encoder_type = "generic";
    if (factory_type.find("acc100") != std::string::npos) {
      encoder_type = "acc100";
    }

    std::shared_ptr<crc_calculator_factory> crc_calc_factory = create_crc_calculator_factory_sw("auto");
    if (!crc_calc_factory) {
      return nullptr;
    }

    std::shared_ptr<ldpc_encoder_factory> ldpc_encoder_factory = create_ldpc_encoder_factory_sw("auto");
    if (!ldpc_encoder_factory) {
      return nullptr;
    }

    std::shared_ptr<ldpc_rate_matcher_factory> ldpc_rate_matcher_factory = create_ldpc_rate_matcher_factory_sw();
    if (!ldpc_rate_matcher_factory) {
      return nullptr;
    }

    std::shared_ptr<ldpc_segmenter_tx_factory> ldpc_segmenter_tx_factory =
        create_ldpc_segmenter_tx_factory_sw(crc_calc_factory);
    if (!ldpc_segmenter_tx_factory) {
      return nullptr;
    }

    std::shared_ptr<modulation_mapper_factory> modulator_factory = create_modulation_mapper_factory();
    if (!modulator_factory) {
      return nullptr;
    }

    std::shared_ptr<pseudo_random_generator_factory> prg_factory = create_pseudo_random_generator_sw_factory();
    if (!prg_factory) {
      return nullptr;
    }

    std::shared_ptr<channel_precoder_factory> precoding_factory = create_channel_precoder_factory("auto");
    if (!precoding_factory) {
      return nullptr;
    }

    std::shared_ptr<resource_grid_mapper_factory> rg_mapper_factory =
        create_resource_grid_mapper_factory(precoding_factory);
    if (!rg_mapper_factory) {
      return nullptr;
    }

    std::shared_ptr<pdsch_modulator_factory> pdsch_modulator_factory =
        create_pdsch_modulator_factory_sw(modulator_factory, prg_factory, rg_mapper_factory);
    if (!pdsch_modulator_factory) {
      return nullptr;
    }

    std::shared_ptr<dmrs_pdsch_processor_factory> dmrs_pdsch_factory =
        create_dmrs_pdsch_processor_factory_sw(prg_factory, rg_mapper_factory);
    if (!dmrs_pdsch_factory) {
      return nullptr;
    }

    std::shared_ptr<ptrs_pdsch_generator_factory> ptrs_pdsch_factory =
        create_ptrs_pdsch_generator_generic_factory(prg_factory, rg_mapper_factory);
    if (!dmrs_pdsch_factory) {
      return nullptr;
    }

    std::shared_ptr<pdsch_encoder_factory>         pdsch_encoder_factory;
    std::shared_ptr<pdsch_block_processor_factory> block_processor_factory;
    if (factory_type.find("generic") != std::string::npos) {
      pdsch_encoder_factory = create_pdsch_encoder_factory(crc_calc_factory, encoder_type, eal_arguments);
      if (!pdsch_encoder_factory) {
        return nullptr;
      }

      return create_pdsch_processor_factory_sw(
          pdsch_encoder_factory, pdsch_modulator_factory, dmrs_pdsch_factory, ptrs_pdsch_factory);
    } else {
      if (encoder_type != "acc100") {
        block_processor_factory = create_pdsch_block_processor_factory_sw(
            ldpc_encoder_factory, ldpc_rate_matcher_factory, prg_factory, modulator_factory);
      } else {
        std::shared_ptr<hal::hw_accelerator_pdsch_enc_factory> hw_encoder_factory =
            create_hw_accelerator_pdsch_enc_factory(eal_arguments);
        if (!hw_encoder_factory) {
          return nullptr;
        }

        block_processor_factory =
            create_pdsch_block_processor_factory_hw(hw_encoder_factory, prg_factory, modulator_factory);
      }
      if (!block_processor_factory) {
        return nullptr;
      }
    }

    if (factory_type.find("flexible") != std::string::npos) {
      // Default parameters for flexible-lite.
      unsigned nof_concurrent_threads = 1;
      unsigned cb_batch_length        = std::numeric_limits<unsigned>::max();

      if (factory_type.find("flexible-concurrent") != std::string::npos) {
        // Adjust number of threads of concurrent implementation.
        nof_concurrent_threads = NOF_CONCURRENT_THREADS + 1;

        // Configure batches.
        if (factory_type.find("-batched") != std::string::npos) {
          cb_batch_length = CB_BATCH_LENGTH;
        } else {
          cb_batch_length = 0;
        }
      }

      return create_pdsch_flexible_processor_factory_sw(ldpc_segmenter_tx_factory,
                                                        block_processor_factory,
                                                        rg_mapper_factory,
                                                        dmrs_pdsch_factory,
                                                        ptrs_pdsch_factory,
                                                        *executor,
                                                        nof_concurrent_threads,
                                                        cb_batch_length);
    }

    return nullptr;
  }

protected:
  // PDSCH processor.
  std::unique_ptr<pdsch_processor> pdsch_proc;
  // PDSCH validator.
  std::unique_ptr<pdsch_pdu_validator> pdu_validator;
  // Worker pool.
  static std::unique_ptr<task_worker_pool<concurrent_queue_policy::locking_mpmc>>          worker_pool;
  static std::unique_ptr<task_worker_pool_executor<concurrent_queue_policy::locking_mpmc>> executor;

  static void SetUpTestSuite()
  {
    // Create worker pool and executor.
    if (!worker_pool || !executor) {
      worker_pool = std::make_unique<task_worker_pool<concurrent_queue_policy::locking_mpmc>>(
          "pdsch_proc", NOF_CONCURRENT_THREADS, 128);
      executor = std::make_unique<task_worker_pool_executor<concurrent_queue_policy::locking_mpmc>>(*worker_pool);
    }
  }

  void SetUp() override
  {
    const PdschProcessorParams& param        = GetParam();
    const std::string&          factory_type = std::get<0>(param);

    ASSERT_NE(worker_pool, nullptr) << "Invalid worker pool.";
    ASSERT_NE(executor, nullptr) << "Invalid executor.";

    // Create PDSCH processor factory.
    std::shared_ptr<pdsch_processor_factory> pdsch_proc_factory =
        create_pdsch_processor_factory(factory_type, eal_arguments);
#ifdef HWACC_PDSCH_ENABLED
    if ((factory_type.find("acc100") != std::string::npos) && skip_hwacc_test) {
      GTEST_SKIP() << "[WARNING] ACC100 not found. Skipping test.";
    }
#endif // HWACC_PDSCH_ENABLED
    ASSERT_NE(pdsch_proc_factory, nullptr) << "Invalid PDSCH processor factory.";

    // Create actual PDSCH processor.
    pdsch_proc = pdsch_proc_factory->create();
    ASSERT_NE(pdsch_proc, nullptr) << "Cannot create PDSCH processor";

    // Create actual PDSCH processor validator.
    pdu_validator = pdsch_proc_factory->create_validator();
    ASSERT_NE(pdu_validator, nullptr) << "Cannot create PDSCH validator";
  }

  static void TearDownTestSuite()
  {
    if (executor) {
      executor.reset();
    }
    if (worker_pool) {
      worker_pool->stop();
    }
  }
};

std::unique_ptr<task_worker_pool<concurrent_queue_policy::locking_mpmc>> PdschProcessorFixture::worker_pool = nullptr;
std::unique_ptr<task_worker_pool_executor<concurrent_queue_policy::locking_mpmc>> PdschProcessorFixture::executor =
    nullptr;

TEST_P(PdschProcessorFixture, PdschProcessorVectortest)
{
  ASSERT_NE(pdsch_proc, nullptr) << "Invalid PDSCH processor.";
  ASSERT_NE(pdu_validator, nullptr) << "Invalid PDSCH PDU validator.";

  pdsch_processor_notifier_spy notifier_spy;
  const PdschProcessorParams&  param     = GetParam();
  const test_case_t&           test_case = std::get<1>(param);
  const test_case_context&     context   = test_case.context;
  pdsch_processor::pdu_t       config    = context.pdu;

  unsigned max_symb  = context.rg_nof_symb;
  unsigned max_prb   = context.rg_nof_rb;
  unsigned max_ports = config.precoding.get_nof_ports();

  // Prepare resource grid and resource grid mapper spies.
  resource_grid_writer_spy grid(max_ports, max_symb, max_prb);

  // Read input data as a bit-packed transport block.
  std::vector<uint8_t> transport_block = test_case.sch_data.read();
  ASSERT_FALSE(transport_block.empty()) << "Failed to load transport block.";

  // Prepare transport blocks view.
  static_vector<shared_transport_block, pdsch_processor::MAX_NOF_TRANSPORT_BLOCKS> transport_blocks;
  transport_blocks.emplace_back(transport_block);

  // Make sure the configuration is valid.
  ASSERT_TRUE(pdu_validator->is_valid(config));

  // Process PDSCH.
  pdsch_proc->process(grid, notifier_spy, transport_blocks, config);

  // Waits for the processor to finish.
  notifier_spy.wait_for_finished();

  // Assert results.
  grid.assert_entries(test_case.grid_expected.read(), std::sqrt(max_ports));
}

// Creates test suite that combines all possible parameters.
INSTANTIATE_TEST_SUITE_P(PdschProcessorVectortest,
                         PdschProcessorFixture,
#ifdef HWACC_PDSCH_ENABLED
                         testing::Combine(testing::Values("generic",
                                                          "flexible-sync",
                                                          "flexible-concurrent",
                                                          "flexible-concurrent-batched",
                                                          "generic-acc100",
                                                          "flexible-sync-acc100",
                                                          "flexible-concurrent-acc100"),
#else  // HWACC_PDSCH_ENABLED
                         testing::Combine(testing::Values("generic", "flexible-sync", "flexible-concurrent"),
#endif // HWACC_PDSCH_ENABLED
                                          ::testing::ValuesIn(pdsch_processor_test_data)));
} // namespace

} // namespace srsran

int main(int argc, char** argv)
{
#ifdef HWACC_PDSCH_ENABLED
  if (argc > 1) {
    // Separate EAL and non-EAL arguments.
    eal_arguments = capture_eal_args(&argc, &argv);
  }
#endif // HWACC_PDSCH_ENABLED

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
