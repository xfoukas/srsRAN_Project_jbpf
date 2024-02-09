/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/ran/nr_cgi.h"
#include "srsran/ran/pci.h"
#include "srsran/ran/rnti.h"
#include <chrono>
#include <memory>

namespace srsran {
namespace srs_cu_cp {

/// CU-CP Metrics report.
struct metrics_report {
  struct ue_info {
    rnti_t rnti;
    pci_t  pci;
  };
  struct cell_info {
    nr_cell_global_id_t cgi;
    pci_t               pci;
  };
  struct du_info {
    std::vector<cell_info> cells;
  };

  std::vector<ue_info> ues;
  std::vector<du_info> dus;
};

/// Interface used by the CU-CP to report metrics.
class metrics_report_notifier
{
public:
  virtual ~metrics_report_notifier() = default;

  /// \brief Used to notify a new metrics report.
  virtual void notify_metrics_report_request(const metrics_report& report) = 0;
};

/// Request for CU-CP metrics reporting.
struct periodic_metric_report_request {
  /// \brief The period at which the CU-CP metrics are reported. In case, the period is 0, the CU-CP metrics are only
  /// sent once.
  std::chrono::milliseconds period{0};
  /// \brief The notifier used by the CU-CP to report metrics.
  metrics_report_notifier* report_notifier = nullptr;
};

/// Metric Report session instance.
class metrics_report_session
{
public:
  /// Closes the session and destroys it.
  virtual ~metrics_report_session() = default;

  /// Configure the metrics report session.
  virtual void reconfigure_request(const periodic_metric_report_request& request) = 0;

  /// Close the session, explicitly stopping the reporting of new metrics.
  virtual void stop() = 0;
};

/// Interface for requesting CU-CP metrics
class metrics_handler
{
public:
  virtual ~metrics_handler() = default;

  /// \brief Create a new metrics periodic report session.
  /// \return Handle to the session. When the session object goes out of scope, the session is automatically closed.
  virtual std::unique_ptr<metrics_report_session>
  create_periodic_report_session(const periodic_metric_report_request& request) = 0;

  /// \brief Synchronously request a new single metrics report. The call blocks until the report is ready.
  ///
  /// Note: Given its blocking nature, avoid calling this method in contexts other than unit tests.
  /// \return The metrics report.
  virtual metrics_report request_metrics_report() = 0;
};

} // namespace srs_cu_cp
} // namespace srsran