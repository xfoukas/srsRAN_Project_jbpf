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

#include "srsran/phy/support/time_alignment_estimator/time_alignment_estimator_factories.h"
#include "srsran/phy/upper/sequence_generators/sequence_generator_factories.h"
#include "srsran/phy/upper/signal_processors/srs/srs_estimator.h"
#include "srsran/phy/upper/signal_processors/srs/srs_estimator_configuration_validator.h"
#include "srsran/srslog/logger.h"
#include <memory>

namespace srsran {

/// Sounding Reference Signal propagation channel estimator factory.
class srs_estimator_factory
{
public:
  /// Default destructor.
  virtual ~srs_estimator_factory() = default;

  /// Creates a Sounding Reference Signal based propagation channel estimator.
  virtual std::unique_ptr<srs_estimator> create() = 0;

  /// Creates a Sounding Reference Signal based propagation channel estimator with logging.
  virtual std::unique_ptr<srs_estimator> create(srslog::basic_logger& logger);

  /// Creates a Sounding Reference Signal channel estimator configuration validator.
  virtual std::unique_ptr<srs_estimator_configuration_validator> create_validator() = 0;
};

/// Create a generic SRS propagation channel estimator factory.
std::shared_ptr<srs_estimator_factory>
create_srs_estimator_generic_factory(std::shared_ptr<low_papr_sequence_generator_factory> sequence_generator_factory,
                                     std::shared_ptr<time_alignment_estimator_factory>    ta_estimator_factory,
                                     unsigned                                             max_nof_prb);

/// Creates a Sounding Reference Signal propagation channel estimator pool.
std::shared_ptr<srs_estimator_factory> create_srs_estimator_pool(std::shared_ptr<srs_estimator_factory> base_factory,
                                                                 unsigned max_nof_threads);

} // namespace srsran
