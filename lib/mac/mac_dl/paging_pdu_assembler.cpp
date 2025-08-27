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

#include "paging_pdu_assembler.h"
#include "srsran/asn1/rrc_nr/pcch_msg.h"
#include "srsran/scheduler/result/pdsch_info.h"

using namespace srsran;

/// \brief Build RRC-NR Paging message from the Paging grant.
/// \param[in] pg Paging grant information.
/// \return RRC-NR Paging message.
static void make_asn1_rrc_cell_paging_msg(asn1::rrc_nr::paging_s& rrc_pg, const dl_paging_allocation& pg)
{
  for (const paging_ue_info& info : pg.paging_ue_list) {
    asn1::rrc_nr::paging_record_s pg_rec{};
    if (info.paging_type_indicator == paging_ue_info::cn_ue_paging_identity) {
      auto& rrc_pg_id = pg_rec.ue_id.set_ng_5_g_s_tmsi();
      rrc_pg_id       = rrc_pg_id.from_number(info.paging_identity);
    } else {
      auto& rrc_pg_id = pg_rec.ue_id.set_full_i_rnti();
      rrc_pg_id       = rrc_pg_id.from_number(info.paging_identity);
    }
    rrc_pg.paging_record_list.push_back(pg_rec);
  }
}

/// \brief Derive packed cell PCCH-PCH Paging message.
/// \param[in] pg Paging grant information.
/// \return byte buffer with packed cell PCCH-PCH message.
static byte_buffer make_asn1_rrc_cell_pcch_pch_msg(const dl_paging_allocation& pg)
{
  byte_buffer   buf{byte_buffer::fallback_allocation_tag{}};
  asn1::bit_ref bref{buf};

  asn1::rrc_nr::pcch_msg_s pcch_msg{};
  auto&                    pcch_c1 = pcch_msg.msg.set_c1();
  auto&                    rrc_pg  = pcch_c1.set_paging();
  make_asn1_rrc_cell_paging_msg(rrc_pg, pg);

  const asn1::SRSASN_CODE ret = pcch_msg.pack(bref);
  srsran_assert(ret == asn1::SRSASN_SUCCESS, "Failed to pack PCCH-PCH Paging message");
  return buf;
}

span<const uint8_t> paging_pdu_assembler::encode_paging_pdu(const dl_paging_allocation& pg)
{
  byte_buffer payload = make_asn1_rrc_cell_pcch_pch_msg(pg);
  srsran_assert(pg.pdsch_cfg.codewords[0].tb_size_bytes >= payload.length(),
                "The TBS for Paging cannot be smaller than the Paging payload");

  span<uint8_t> pdu_bytes = pdu_pool.allocate_buffer(pg.pdsch_cfg.codewords[0].tb_size_bytes);
  for (unsigned idx = 0; idx < payload.length(); ++idx) {
    pdu_bytes[idx] = payload[idx];
  }
  // Apply RRC padding if RRC message does not fill a transport block. See TS 38.321, clause 6.1.4 and TS 38.331,
  // clause 8.5.
  std::fill(pdu_bytes.data() + payload.length(), pdu_bytes.data() + pdu_bytes.size(), 0);
  return pdu_bytes;
}
