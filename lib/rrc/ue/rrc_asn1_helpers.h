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

#include "srsran/adt/byte_buffer.h"
#include "srsran/adt/expected.h"
#include "srsran/asn1/rrc_nr/dl_dcch_msg.h"
#include "srsran/asn1/rrc_nr/ul_dcch_msg.h"
#include "srsran/asn1/rrc_nr/ul_dcch_msg_ies.h"
#include "srsran/rrc/rrc_types.h"

namespace srsran {
namespace srs_cu_cp {

/// \brief Fills ASN.1 RRC Setup struct.
/// \param[out] rrc_setup The RRC Setup ASN.1 struct to fill.
/// \param[in] init_ul_rrc_transfer_msg The Init_UL_RRC_Transfer message received by the CU.
/// \return True on success, otherwise false.
bool fill_asn1_rrc_setup_msg(asn1::rrc_nr::rrc_setup_s& rrc_setup, const byte_buffer& mcg, uint8_t rrc_transaction_id);

/// Extracts transaction id of RRC Setup complete message.
expected<uint8_t> get_transaction_id(const asn1::rrc_nr::rrc_setup_complete_s& msg);

/// Extracts transaction id of UL-DCCH message.
expected<uint8_t> get_transaction_id(const asn1::rrc_nr::ul_dcch_msg_s& msg);

/// \brief Fills ASN.1 RRC Security Mode Command struct.
/// \param[out] rrc_smc The RRC security mode command ASN.1 struct to fill.
/// \param[in] int_algo The selected integrity protection algorithm.
/// \param[in] ciph_algo The selected ciphering algorithm.
/// \param[in] rrc_transaction_id The RRC transaction id.
void fill_asn1_rrc_smc_msg(asn1::rrc_nr::security_mode_cmd_s&   rrc_smc,
                           const security::integrity_algorithm& int_algo,
                           const security::ciphering_algorithm& ciph_algo,
                           uint8_t                              rrc_transaction_id);

/// \brief Fills ASN.1 RRC Setup struct.
/// \param[out] asn1_rrc_reconf The RRC Reconfiguration ASN.1 struct to fill.
/// \param[in] rrc_transaction_id The RRC transaction id.
/// \param[in] rrc_reconf The common type struct.
void fill_asn1_rrc_reconfiguration_msg(asn1::rrc_nr::rrc_recfg_s&            asn1_rrc_reconf,
                                       uint8_t                               rrc_transaction_id,
                                       rrc_reconfiguration_procedure_request rrc_reconf);

} // namespace srs_cu_cp
} // namespace srsran
