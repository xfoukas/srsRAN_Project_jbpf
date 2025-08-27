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

#include "dpdk_ethernet_transmitter.h"
#include "srsran/adt/static_vector.h"
#include "srsran/ofh/ethernet/ethernet_transmitter_config.h"
#include <rte_ethdev.h>

using namespace srsran;
using namespace ether;

void dpdk_transmitter_impl::send(span<span<const uint8_t>> frames)
{
  // Receiving a frame burst larger than MAX_BURST_SIZE requires making several Tx bursts.
  for (unsigned offset = 0; offset < frames.size();) {
    auto frame_burst = frames.subspan(offset, std::min<unsigned>(MAX_BURST_SIZE, frames.size() - offset));
    offset += frame_burst.size();

    auto meas = metrics_collector.create_time_execution_measurer();

    static_vector<::rte_mbuf*, MAX_BURST_SIZE> mbufs(frame_burst.size());
    if (::rte_pktmbuf_alloc_bulk(port_ctx->get_mempool(), mbufs.data(), frame_burst.size()) < 0) {
      logger.warning("Not enough entries in the mempool to send '{}' frames to the NIC port '{}' in the DPDK Ethernet "
                     "transmitter ",
                     frame_burst.size(),
                     port_ctx->get_port_id());
      metrics_collector.update_stats(meas.stop());
      return;
    }

    uint64_t nof_bytes_in_burst = 0;
    for (unsigned idx = 0, end = frame_burst.size(); idx != end; ++idx) {
      const auto  frame = frame_burst[idx];
      ::rte_mbuf* mbuf  = mbufs[idx];

      if (::rte_pktmbuf_append(mbuf, frame.size()) == nullptr) {
        ::rte_pktmbuf_free(mbuf);
        logger.warning("Unable to append '{}' bytes to the allocated mbuf associated with the NIC port '{}' in the "
                       "DPDK Ethernet transmitter",
                       frame.size(),
                       port_ctx->get_port_id());
        ::rte_pktmbuf_free_bulk(mbufs.data(), mbufs.size());
        metrics_collector.update_stats(meas.stop());
        return;
      }
      mbuf->data_len = frame.size();
      mbuf->pkt_len  = frame.size();
      nof_bytes_in_burst += frame.size();

      uint8_t* data = rte_pktmbuf_mtod(mbuf, uint8_t*);
      std::memcpy(data, frame.data(), frame.size());
    }

    unsigned nof_sent_packets = ::rte_eth_tx_burst(port_ctx->get_dpdk_port_id(), 0, mbufs.data(), mbufs.size());

    if (SRSRAN_UNLIKELY(nof_sent_packets < mbufs.size())) {
      logger.warning(
          "DPDK dropped '{}' packets out of a total of '{}' in the tx burst while sending data to the NIC port '{}'",
          mbufs.size() - nof_sent_packets,
          mbufs.size(),
          port_ctx->get_port_id());

      // Recalculate actual number of bytes transmitted.
      if (!metrics_collector.disabled()) {
        nof_bytes_in_burst = 0;
        for (unsigned i = 0; i != nof_sent_packets; ++i) {
          nof_bytes_in_burst += frame_burst[i].size();
        }
      }
      for (unsigned buf_idx = nof_sent_packets, last_idx = mbufs.size(); buf_idx != last_idx; ++buf_idx) {
        ::rte_pktmbuf_free(mbufs[buf_idx]);
      }
    }
    metrics_collector.update_stats(meas.stop(), nof_bytes_in_burst, nof_sent_packets);
  }
}

transmitter_metrics_collector* dpdk_transmitter_impl::get_metrics_collector()
{
  return metrics_collector.disabled() ? nullptr : &metrics_collector;
}
