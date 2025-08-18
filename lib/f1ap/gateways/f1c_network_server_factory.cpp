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

#include "srsran/f1ap/gateways/f1c_network_server_factory.h"
#include "srsran/asn1/f1ap/f1ap.h"
#include "srsran/f1ap/f1ap_message.h"
#include "srsran/gateways/sctp_network_server_factory.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/support/error_handling.h"
#include <cstdint>

using namespace srsran;

namespace {

/// Notifier passed to the CU-CP, which the CU-CP will use to send F1AP Tx PDUs.
class f1c_to_gw_pdu_notifier final : public f1ap_message_notifier
{
public:
  f1c_to_gw_pdu_notifier(std::unique_ptr<sctp_association_sdu_notifier> sctp_sender_,
                         dlt_pcap&                                      pcap_writer_,
                         srslog::basic_logger&                          logger_) :
    sctp_sender(std::move(sctp_sender_)), pcap_writer(pcap_writer_), logger(logger_)
  {
  }

  /// Handle unpacked Tx F1AP PDU by packing and forwarding it into the SCTP GW.
  void on_new_message(const f1ap_message& msg) override
  {
    // pack F1AP PDU into SCTP SDU.
    byte_buffer   tx_sdu{byte_buffer::fallback_allocation_tag{}};
    asn1::bit_ref bref(tx_sdu);
    if (msg.pdu.pack(bref) != asn1::SRSASN_SUCCESS) {
      logger.error("Failed to pack F1AP PDU");
      return;
    }

    // Push Tx PDU to pcap.
    if (pcap_writer.is_write_enabled()) {
      pcap_writer.push_pdu(tx_sdu.copy());
    }

    // Forward packed F1AP Tx PDU to SCTP gateway.
    sctp_sender->on_new_sdu(std::move(tx_sdu));
  }

private:
  std::unique_ptr<sctp_association_sdu_notifier> sctp_sender;
  dlt_pcap&                                      pcap_writer;
  srslog::basic_logger&                          logger;
};

/// Notifier passed to the SCTP GW, which the GW will use to forward F1AP Rx PDUs to the CU-CP.
class gw_to_f1c_pdu_notifier final : public sctp_association_sdu_notifier
{
public:
  gw_to_f1c_pdu_notifier(std::unique_ptr<f1ap_message_notifier> f1ap_notifier_,
                         dlt_pcap&                              pcap_writer_,
                         srslog::basic_logger&                  logger_) :
    f1ap_notifier(std::move(f1ap_notifier_)), pcap_writer(pcap_writer_), logger(logger_)
  {
  }

  bool on_new_sdu(byte_buffer sdu) override
  {
    // Unpack SCTP SDU into F1AP PDU.
    asn1::cbit_ref bref(sdu);
    f1ap_message   msg;
    if (msg.pdu.unpack(bref) != asn1::SRSASN_SUCCESS) {
      logger.error("Couldn't unpack F1AP PDU");
      return false;
    }

    // Forward SCTP Rx SDU to pcap, if enabled.
    if (pcap_writer.is_write_enabled()) {
      pcap_writer.push_pdu(sdu.copy());
    }

    // Forward unpacked Rx PDU to the CU-CP.
    f1ap_notifier->on_new_message(msg);

    return true;
  }

private:
  std::unique_ptr<f1ap_message_notifier> f1ap_notifier;
  dlt_pcap&                              pcap_writer;
  srslog::basic_logger&                  logger;
};

/// Adapter of the SCTP server to the F1-C interface of the CU-CP.
class f1c_sctp_server final : public srs_cu_cp::f1c_connection_server, public sctp_network_association_factory
{
public:
  f1c_sctp_server(const f1c_cu_sctp_gateway_config& params_) : params(params_)
  {
    // Create SCTP server.
    sctp_server = create_sctp_network_server(
        sctp_network_server_config{params.sctp, params.broker, params.io_rx_executor, *this});
    report_error_if_not(sctp_server != nullptr, "Failed to create SCTP server");
  }

  void attach_cu_cp(srs_cu_cp::cu_cp_f1c_handler& cu_f1c_handler_) override
  {
    cu_f1c_handler = &cu_f1c_handler_;

    // Start listening for new DU SCTP connections.
    bool result = sctp_server->listen();
    report_error_if_not(result, "Failed to start SCTP server.\n");
    fmt::print("{}: Listening for new connections on {}:{}...\n",
               params.sctp.if_name,
               params.sctp.bind_address,
               params.sctp.bind_port);
  }

  std::optional<uint16_t> get_listen_port() const override { return sctp_server->get_listen_port(); }

  std::unique_ptr<sctp_association_sdu_notifier>
  create(std::unique_ptr<sctp_association_sdu_notifier> sctp_send_notifier) override
  {
    // Create an unpacked F1AP PDU notifier and pass it to the CU-CP.
    auto f1c_sender = std::make_unique<f1c_to_gw_pdu_notifier>(std::move(sctp_send_notifier), params.pcap, logger);

    std::unique_ptr<f1ap_message_notifier> f1c_receiver =
        cu_f1c_handler->handle_new_du_connection(std::move(f1c_sender));

    // Wrap the received F1AP Rx PDU notifier in an SCTP notifier and return it.
    if (f1c_receiver == nullptr) {
      return nullptr;
    }

    return std::make_unique<gw_to_f1c_pdu_notifier>(std::move(f1c_receiver), params.pcap, logger);
  }

private:
  const f1c_cu_sctp_gateway_config params;
  srslog::basic_logger&            logger         = srslog::fetch_basic_logger("CU-CP-F1");
  srs_cu_cp::cu_cp_f1c_handler*    cu_f1c_handler = nullptr;

  std::unique_ptr<sctp_network_server> sctp_server;
};

} // namespace

std::unique_ptr<srs_cu_cp::f1c_connection_server>
srsran::create_f1c_gateway_server(const f1c_cu_sctp_gateway_config& cfg)
{
  return std::make_unique<f1c_sctp_server>(cfg);
}
