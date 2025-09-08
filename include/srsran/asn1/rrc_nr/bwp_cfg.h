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

/*******************************************************************************
 *
 *                    3GPP TS ASN1 RRC NR v17.4.0 (2023-03)
 *
 ******************************************************************************/

#pragma once

#include "srsran/asn1/rrc_nr/common.h"

namespace asn1 {
namespace rrc_nr {

/*******************************************************************************
 *                              Struct Definitions
 ******************************************************************************/

// BWP ::= SEQUENCE
struct bwp_s {
  bool                 cp_present      = false;
  uint16_t             location_and_bw = 0;
  subcarrier_spacing_e subcarrier_spacing;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// MsgA-DMRS-Config-r16 ::= SEQUENCE
struct msg_a_dmrs_cfg_r16_s {
  struct msg_a_dmrs_add_position_r16_opts {
    enum options { pos0, pos1, pos3, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using msg_a_dmrs_add_position_r16_e_ = enumerated<msg_a_dmrs_add_position_r16_opts>;

  // member variables
  bool                           msg_a_dmrs_add_position_r16_present    = false;
  bool                           msg_a_max_len_r16_present              = false;
  bool                           msg_a_pusch_dmrs_cdm_group_r16_present = false;
  bool                           msg_a_pusch_nrof_ports_r16_present     = false;
  bool                           msg_a_scrambling_id0_r16_present       = false;
  bool                           msg_a_scrambling_id1_r16_present       = false;
  msg_a_dmrs_add_position_r16_e_ msg_a_dmrs_add_position_r16;
  uint8_t                        msg_a_pusch_dmrs_cdm_group_r16 = 0;
  uint8_t                        msg_a_pusch_nrof_ports_r16     = 0;
  uint32_t                       msg_a_scrambling_id0_r16       = 0;
  uint32_t                       msg_a_scrambling_id1_r16       = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RA-Prioritization ::= SEQUENCE
struct ra_prioritization_s {
  struct pwr_ramp_step_high_prio_opts {
    enum options { db0, db2, db4, db6, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pwr_ramp_step_high_prio_e_ = enumerated<pwr_ramp_step_high_prio_opts>;
  struct scaling_factor_bi_opts {
    enum options { zero, dot25, dot5, dot75, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using scaling_factor_bi_e_ = enumerated<scaling_factor_bi_opts>;

  // member variables
  bool                       ext                       = false;
  bool                       scaling_factor_bi_present = false;
  pwr_ramp_step_high_prio_e_ pwr_ramp_step_high_prio;
  scaling_factor_bi_e_       scaling_factor_bi;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// MsgA-PUSCH-Resource-r16 ::= SEQUENCE
struct msg_a_pusch_res_r16_s {
  struct nrof_msg_a_po_per_slot_r16_opts {
    enum options { one, two, three, six, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using nrof_msg_a_po_per_slot_r16_e_ = enumerated<nrof_msg_a_po_per_slot_r16_opts>;
  struct map_type_msg_a_pusch_r16_opts {
    enum options { type_a, type_b, nulltype } value;

    const char* to_string() const;
  };
  using map_type_msg_a_pusch_r16_e_ = enumerated<map_type_msg_a_pusch_r16_opts>;
  struct nrof_msg_a_po_fdm_r16_opts {
    enum options { one, two, four, eight, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using nrof_msg_a_po_fdm_r16_e_ = enumerated<nrof_msg_a_po_fdm_r16_opts>;
  struct msg_a_alpha_r16_opts {
    enum options { alpha0, alpha04, alpha05, alpha06, alpha07, alpha08, alpha09, alpha1, nulltype } value;
    typedef float number_type;

    const char* to_string() const;
    float       to_number() const;
    const char* to_number_string() const;
  };
  using msg_a_alpha_r16_e_ = enumerated<msg_a_alpha_r16_opts>;

  // member variables
  bool                          ext                                            = false;
  bool                          msg_a_pusch_time_domain_alloc_r16_present      = false;
  bool                          start_symbol_and_len_msg_a_po_r16_present      = false;
  bool                          map_type_msg_a_pusch_r16_present               = false;
  bool                          guard_period_msg_a_pusch_r16_present           = false;
  bool                          msg_a_intra_slot_freq_hop_r16_present          = false;
  bool                          msg_a_hop_bits_r16_present                     = false;
  bool                          msg_a_alpha_r16_present                        = false;
  bool                          interlace_idx_first_po_msg_a_pusch_r16_present = false;
  bool                          nrof_interlaces_per_msg_a_po_r16_present       = false;
  uint8_t                       msg_a_mcs_r16                                  = 0;
  uint8_t                       nrof_slots_msg_a_pusch_r16                     = 1;
  nrof_msg_a_po_per_slot_r16_e_ nrof_msg_a_po_per_slot_r16;
  uint8_t                       msg_a_pusch_time_domain_offset_r16 = 1;
  uint8_t                       msg_a_pusch_time_domain_alloc_r16  = 1;
  uint8_t                       start_symbol_and_len_msg_a_po_r16  = 0;
  map_type_msg_a_pusch_r16_e_   map_type_msg_a_pusch_r16;
  uint8_t                       guard_period_msg_a_pusch_r16 = 0;
  uint8_t                       guard_band_msg_a_pusch_r16   = 0;
  uint16_t                      freq_start_msg_a_pusch_r16   = 0;
  uint8_t                       nrof_prbs_per_msg_a_po_r16   = 1;
  nrof_msg_a_po_fdm_r16_e_      nrof_msg_a_po_fdm_r16;
  fixed_bitstring<2>            msg_a_hop_bits_r16;
  msg_a_dmrs_cfg_r16_s          msg_a_dmrs_cfg_r16;
  uint8_t                       nrof_dmrs_seqs_r16 = 1;
  msg_a_alpha_r16_e_            msg_a_alpha_r16;
  uint8_t                       interlace_idx_first_po_msg_a_pusch_r16 = 1;
  uint8_t                       nrof_interlaces_per_msg_a_po_r16       = 1;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// NSAG-List-r17 ::= SEQUENCE (SIZE (1..8)) OF BIT STRING (SIZE (8))
using nsag_list_r17_l = bounded_array<fixed_bitstring<8>, 8>;

// RA-PrioritizationSliceInfo-r17 ::= SEQUENCE
struct ra_prioritization_slice_info_r17_s {
  using nsag_id_list_r17_l_ = bounded_array<fixed_bitstring<8>, 8>;

  // member variables
  bool                ext = false;
  nsag_id_list_r17_l_ nsag_id_list_r17;
  ra_prioritization_s ra_prioritization_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// FeatureCombination-r17 ::= SEQUENCE
struct feature_combination_r17_s {
  bool            red_cap_r17_present      = false;
  bool            small_data_r17_present   = false;
  bool            msg3_repeats_r17_present = false;
  bool            spare4_present           = false;
  bool            spare3_present           = false;
  bool            spare2_present           = false;
  bool            spare1_present           = false;
  nsag_list_r17_l nsag_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// MsgA-PUSCH-Config-r16 ::= SEQUENCE
struct msg_a_pusch_cfg_r16_s {
  struct msg_a_transform_precoder_r16_opts {
    enum options { enabled, disabled, nulltype } value;

    const char* to_string() const;
  };
  using msg_a_transform_precoder_r16_e_ = enumerated<msg_a_transform_precoder_r16_opts>;

  // member variables
  bool                            msg_a_pusch_res_group_a_r16_present   = false;
  bool                            msg_a_pusch_res_group_b_r16_present   = false;
  bool                            msg_a_transform_precoder_r16_present  = false;
  bool                            msg_a_data_scrambling_idx_r16_present = false;
  bool                            msg_a_delta_preamb_r16_present        = false;
  msg_a_pusch_res_r16_s           msg_a_pusch_res_group_a_r16;
  msg_a_pusch_res_r16_s           msg_a_pusch_res_group_b_r16;
  msg_a_transform_precoder_r16_e_ msg_a_transform_precoder_r16;
  uint16_t                        msg_a_data_scrambling_idx_r16 = 0;
  int8_t                          msg_a_delta_preamb_r16        = -1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PathlossReferenceRS-Config ::= CHOICE
struct pathloss_ref_rs_cfg_c {
  struct types_opts {
    enum options { ssb_idx, csi_rs_idx, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  pathloss_ref_rs_cfg_c() = default;
  pathloss_ref_rs_cfg_c(const pathloss_ref_rs_cfg_c& other);
  pathloss_ref_rs_cfg_c& operator=(const pathloss_ref_rs_cfg_c& other);
  ~pathloss_ref_rs_cfg_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  uint8_t& ssb_idx()
  {
    assert_choice_type(types::ssb_idx, type_, "PathlossReferenceRS-Config");
    return c.get<uint8_t>();
  }
  uint8_t& csi_rs_idx()
  {
    assert_choice_type(types::csi_rs_idx, type_, "PathlossReferenceRS-Config");
    return c.get<uint8_t>();
  }
  const uint8_t& ssb_idx() const
  {
    assert_choice_type(types::ssb_idx, type_, "PathlossReferenceRS-Config");
    return c.get<uint8_t>();
  }
  const uint8_t& csi_rs_idx() const
  {
    assert_choice_type(types::csi_rs_idx, type_, "PathlossReferenceRS-Config");
    return c.get<uint8_t>();
  }
  uint8_t& set_ssb_idx();
  uint8_t& set_csi_rs_idx();

private:
  types               type_;
  pod_choice_buffer_t c;

  void destroy_();
};

// PUCCH-PathlossReferenceRS-r16 ::= SEQUENCE
struct pucch_pathloss_ref_rs_r16_s {
  struct ref_sig_r16_c_ {
    struct types_opts {
      enum options { ssb_idx_r16, csi_rs_idx_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ref_sig_r16_c_() = default;
    ref_sig_r16_c_(const ref_sig_r16_c_& other);
    ref_sig_r16_c_& operator=(const ref_sig_r16_c_& other);
    ~ref_sig_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx_r16()
    {
      assert_choice_type(types::ssb_idx_r16, type_, "referenceSignal-r16");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx_r16()
    {
      assert_choice_type(types::csi_rs_idx_r16, type_, "referenceSignal-r16");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_idx_r16() const
    {
      assert_choice_type(types::ssb_idx_r16, type_, "referenceSignal-r16");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx_r16() const
    {
      assert_choice_type(types::csi_rs_idx_r16, type_, "referenceSignal-r16");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_idx_r16();
    uint8_t& set_csi_rs_idx_r16();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  uint8_t        pucch_pathloss_ref_rs_id_r16 = 4;
  ref_sig_r16_c_ ref_sig_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RA-PrioritizationSliceInfoList-r17 ::= SEQUENCE (SIZE (1..8)) OF RA-PrioritizationSliceInfo-r17
using ra_prioritization_slice_info_list_r17_l = dyn_array<ra_prioritization_slice_info_r17_s>;

// BetaOffsets ::= SEQUENCE
struct beta_offsets_s {
  bool    beta_offset_ack_idx1_present       = false;
  bool    beta_offset_ack_idx2_present       = false;
  bool    beta_offset_ack_idx3_present       = false;
  bool    beta_offset_csi_part1_idx1_present = false;
  bool    beta_offset_csi_part1_idx2_present = false;
  bool    beta_offset_csi_part2_idx1_present = false;
  bool    beta_offset_csi_part2_idx2_present = false;
  uint8_t beta_offset_ack_idx1               = 0;
  uint8_t beta_offset_ack_idx2               = 0;
  uint8_t beta_offset_ack_idx3               = 0;
  uint8_t beta_offset_csi_part1_idx1         = 0;
  uint8_t beta_offset_csi_part1_idx2         = 0;
  uint8_t beta_offset_csi_part2_idx1         = 0;
  uint8_t beta_offset_csi_part2_idx2         = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BFR-CSIRS-Resource ::= SEQUENCE
struct bfr_csi_rs_res_s {
  using ra_occasion_list_l_ = dyn_array<uint16_t>;

  // member variables
  bool                ext                   = false;
  bool                ra_preamb_idx_present = false;
  uint8_t             csi_rs                = 0;
  ra_occasion_list_l_ ra_occasion_list;
  uint8_t             ra_preamb_idx = 0;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BFR-SSB-Resource ::= SEQUENCE
struct bfr_ssb_res_s {
  bool    ext           = false;
  uint8_t ssb           = 0;
  uint8_t ra_preamb_idx = 0;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DMRS-UplinkTransformPrecoding-r16 ::= SEQUENCE
struct dmrs_ul_transform_precoding_r16_s {
  bool     pi2_bpsk_scrambling_id0_present = false;
  bool     pi2_bpsk_scrambling_id1_present = false;
  uint32_t pi2_bpsk_scrambling_id0         = 0;
  uint32_t pi2_bpsk_scrambling_id1         = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// FDM-TDM-r16 ::= SEQUENCE
struct fdm_tdm_r16_s {
  struct repeat_scheme_r16_opts {
    enum options { fdm_scheme_a, fdm_scheme_b, tdm_scheme_a, nulltype } value;

    const char* to_string() const;
  };
  using repeat_scheme_r16_e_ = enumerated<repeat_scheme_r16_opts>;

  // member variables
  bool                 start_symbol_offset_k_r16_present = false;
  repeat_scheme_r16_e_ repeat_scheme_r16;
  uint8_t              start_symbol_offset_k_r16 = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// FeatureCombinationPreambles-r17 ::= SEQUENCE
struct feature_combination_preambs_r17_s {
  struct group_bcfg_r17_s_ {
    struct ra_size_group_a_r17_opts {
      enum options {
        b56,
        b144,
        b208,
        b256,
        b282,
        b480,
        b640,
        b800,
        b1000,
        b72,
        spare6,
        spare5,
        spare4,
        spare3,
        spare2,
        spare1,
        nulltype
      } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using ra_size_group_a_r17_e_ = enumerated<ra_size_group_a_r17_opts>;
    struct msg_pwr_offset_group_b_r17_opts {
      enum options { minusinfinity, db0, db5, db8, db10, db12, db15, db18, nulltype } value;
      typedef int8_t number_type;

      const char* to_string() const;
      int8_t      to_number() const;
    };
    using msg_pwr_offset_group_b_r17_e_ = enumerated<msg_pwr_offset_group_b_r17_opts>;

    // member variables
    ra_size_group_a_r17_e_        ra_size_group_a_r17;
    msg_pwr_offset_group_b_r17_e_ msg_pwr_offset_group_b_r17;
    uint8_t                       nof_ra_preambs_group_a_r17 = 1;
  };

  // member variables
  bool                      ext                                  = false;
  bool                      ssb_shared_ro_mask_idx_r17_present   = false;
  bool                      group_bcfg_r17_present               = false;
  bool                      separate_msg_a_pusch_cfg_r17_present = false;
  bool                      msg_a_rsrp_thres_r17_present         = false;
  bool                      rsrp_thres_ssb_r17_present           = false;
  bool                      delta_preamb_r17_present             = false;
  feature_combination_r17_s feature_combination_r17;
  uint8_t                   start_preamb_for_this_partition_r17        = 0;
  uint8_t                   nof_preambs_per_ssb_for_this_partition_r17 = 1;
  uint8_t                   ssb_shared_ro_mask_idx_r17                 = 1;
  group_bcfg_r17_s_         group_bcfg_r17;
  msg_a_pusch_cfg_r16_s     separate_msg_a_pusch_cfg_r17;
  uint8_t                   msg_a_rsrp_thres_r17 = 0;
  uint8_t                   rsrp_thres_ssb_r17   = 0;
  int8_t                    delta_preamb_r17     = -1;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// GroupB-ConfiguredTwoStepRA-r16 ::= SEQUENCE
struct group_b_cfg_two_step_ra_r16_s {
  struct ra_msg_a_size_group_a_opts {
    enum options {
      b56,
      b144,
      b208,
      b256,
      b282,
      b480,
      b640,
      b800,
      b1000,
      b72,
      spare6,
      spare5,
      spare4,
      spare3,
      spare2,
      spare1,
      nulltype
    } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using ra_msg_a_size_group_a_e_ = enumerated<ra_msg_a_size_group_a_opts>;
  struct msg_pwr_offset_group_b_opts {
    enum options { minusinfinity, db0, db5, db8, db10, db12, db15, db18, nulltype } value;
    typedef int8_t number_type;

    const char* to_string() const;
    int8_t      to_number() const;
  };
  using msg_pwr_offset_group_b_e_ = enumerated<msg_pwr_offset_group_b_opts>;

  // member variables
  ra_msg_a_size_group_a_e_  ra_msg_a_size_group_a;
  msg_pwr_offset_group_b_e_ msg_pwr_offset_group_b;
  uint8_t                   nof_ra_preambs_group_a = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PathlossReferenceRS-r16 ::= SEQUENCE
struct pathloss_ref_rs_r16_s {
  uint8_t               srs_pathloss_ref_rs_id_r16 = 0;
  pathloss_ref_rs_cfg_c pathloss_ref_rs_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PathlossReferenceRSs-v1610 ::= SEQUENCE (SIZE (1..60)) OF PUCCH-PathlossReferenceRS-r16
using pathloss_ref_rss_v1610_l = dyn_array<pucch_pathloss_ref_rs_r16_s>;

// PDSCH-TimeDomainResourceAllocation-r16 ::= SEQUENCE
struct pdsch_time_domain_res_alloc_r16_s {
  struct map_type_r16_opts {
    enum options { type_a, type_b, nulltype } value;

    const char* to_string() const;
  };
  using map_type_r16_e_ = enumerated<map_type_r16_opts>;
  struct repeat_num_r16_opts {
    enum options { n2, n3, n4, n5, n6, n7, n8, n16, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using repeat_num_r16_e_ = enumerated<repeat_num_r16_opts>;
  struct repeat_num_v1730_opts {
    enum options { n2, n3, n4, n5, n6, n7, n8, n16, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using repeat_num_v1730_e_ = enumerated<repeat_num_v1730_opts>;

  // member variables
  bool              ext                    = false;
  bool              k0_r16_present         = false;
  bool              repeat_num_r16_present = false;
  uint8_t           k0_r16                 = 0;
  map_type_r16_e_   map_type_r16;
  uint8_t           start_symbol_and_len_r16 = 0;
  repeat_num_r16_e_ repeat_num_r16;
  // ...
  // group 0
  bool    k0_v1710_present = false;
  uint8_t k0_v1710         = 33;
  // group 1
  bool                repeat_num_v1730_present = false;
  repeat_num_v1730_e_ repeat_num_v1730;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PTRS-DownlinkConfig ::= SEQUENCE
struct ptrs_dl_cfg_s {
  using freq_density_l_ = std::array<uint16_t, 2>;
  using time_density_l_ = std::array<uint8_t, 3>;
  struct res_elem_offset_opts {
    enum options { offset01, offset10, offset11, nulltype } value;
    typedef float number_type;

    const char* to_string() const;
    float       to_number() const;
    const char* to_number_string() const;
  };
  using res_elem_offset_e_ = enumerated<res_elem_offset_opts>;
  struct max_nrof_ports_r16_opts {
    enum options { n1, n2, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using max_nrof_ports_r16_e_ = enumerated<max_nrof_ports_r16_opts>;

  // member variables
  bool               ext                     = false;
  bool               freq_density_present    = false;
  bool               time_density_present    = false;
  bool               epre_ratio_present      = false;
  bool               res_elem_offset_present = false;
  freq_density_l_    freq_density;
  time_density_l_    time_density;
  uint8_t            epre_ratio = 0;
  res_elem_offset_e_ res_elem_offset;
  // ...
  // group 0
  bool                  max_nrof_ports_r16_present = false;
  max_nrof_ports_r16_e_ max_nrof_ports_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PTRS-UplinkConfig ::= SEQUENCE
struct ptrs_ul_cfg_s {
  struct transform_precoder_disabled_s_ {
    using freq_density_l_ = std::array<uint16_t, 2>;
    using time_density_l_ = std::array<uint8_t, 3>;
    struct max_nrof_ports_opts {
      enum options { n1, n2, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using max_nrof_ports_e_ = enumerated<max_nrof_ports_opts>;
    struct res_elem_offset_opts {
      enum options { offset01, offset10, offset11, nulltype } value;
      typedef float number_type;

      const char* to_string() const;
      float       to_number() const;
      const char* to_number_string() const;
    };
    using res_elem_offset_e_ = enumerated<res_elem_offset_opts>;
    struct ptrs_pwr_opts {
      enum options { p00, p01, p10, p11, nulltype } value;
      typedef float number_type;

      const char* to_string() const;
      float       to_number() const;
      const char* to_number_string() const;
    };
    using ptrs_pwr_e_ = enumerated<ptrs_pwr_opts>;

    // member variables
    bool               freq_density_present    = false;
    bool               time_density_present    = false;
    bool               res_elem_offset_present = false;
    freq_density_l_    freq_density;
    time_density_l_    time_density;
    max_nrof_ports_e_  max_nrof_ports;
    res_elem_offset_e_ res_elem_offset;
    ptrs_pwr_e_        ptrs_pwr;
  };
  struct transform_precoder_enabled_s_ {
    using sample_density_l_ = std::array<uint16_t, 5>;

    // member variables
    bool              time_density_transform_precoding_present = false;
    sample_density_l_ sample_density;
  };

  // member variables
  bool                           ext                                 = false;
  bool                           transform_precoder_disabled_present = false;
  bool                           transform_precoder_enabled_present  = false;
  transform_precoder_disabled_s_ transform_precoder_disabled;
  transform_precoder_enabled_s_  transform_precoder_enabled;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-MaxCodeRate ::= ENUMERATED
struct pucch_max_code_rate_opts {
  enum options { zero_dot08, zero_dot15, zero_dot25, zero_dot35, zero_dot45, zero_dot60, zero_dot80, nulltype } value;
  typedef float number_type;

  const char* to_string() const;
  float       to_number() const;
  const char* to_number_string() const;
};
using pucch_max_code_rate_e = enumerated<pucch_max_code_rate_opts>;

// PUSCH-Allocation-r16 ::= SEQUENCE
struct pusch_alloc_r16_s {
  struct map_type_r16_opts {
    enum options { type_a, type_b, nulltype } value;

    const char* to_string() const;
  };
  using map_type_r16_e_ = enumerated<map_type_r16_opts>;
  struct nof_repeats_r16_opts {
    enum options { n1, n2, n3, n4, n7, n8, n12, n16, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using nof_repeats_r16_e_ = enumerated<nof_repeats_r16_opts>;
  struct nof_repeats_ext_r17_opts {
    enum options {
      n1,
      n2,
      n3,
      n4,
      n7,
      n8,
      n12,
      n16,
      n20,
      n24,
      n28,
      n32,
      spare4,
      spare3,
      spare2,
      spare1,
      nulltype
    } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using nof_repeats_ext_r17_e_ = enumerated<nof_repeats_ext_r17_opts>;
  struct nof_slots_t_bo_ms_r17_opts {
    enum options { n1, n2, n4, n8, spare4, spare3, spare2, spare1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using nof_slots_t_bo_ms_r17_e_ = enumerated<nof_slots_t_bo_ms_r17_opts>;

  // member variables
  bool               ext                              = false;
  bool               map_type_r16_present             = false;
  bool               start_symbol_and_len_r16_present = false;
  bool               start_symbol_r16_present         = false;
  bool               len_r16_present                  = false;
  bool               nof_repeats_r16_present          = false;
  map_type_r16_e_    map_type_r16;
  uint8_t            start_symbol_and_len_r16 = 0;
  uint8_t            start_symbol_r16         = 0;
  uint8_t            len_r16                  = 1;
  nof_repeats_r16_e_ nof_repeats_r16;
  // ...
  // group 0
  bool                     nof_repeats_ext_r17_present   = false;
  bool                     nof_slots_t_bo_ms_r17_present = false;
  bool                     extended_k2_r17_present       = false;
  nof_repeats_ext_r17_e_   nof_repeats_ext_r17;
  nof_slots_t_bo_ms_r17_e_ nof_slots_t_bo_ms_r17;
  uint8_t                  extended_k2_r17 = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RA-PrioritizationForSlicing-r17 ::= SEQUENCE
struct ra_prioritization_for_slicing_r17_s {
  bool                                    ext = false;
  ra_prioritization_slice_info_list_r17_l ra_prioritization_slice_info_list_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RACH-ConfigGenericTwoStepRA-r16 ::= SEQUENCE
struct rach_cfg_generic_two_step_ra_r16_s {
  struct msg_a_ro_fdm_r16_opts {
    enum options { one, two, four, eight, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using msg_a_ro_fdm_r16_e_ = enumerated<msg_a_ro_fdm_r16_opts>;
  struct msg_a_preamb_pwr_ramp_step_r16_opts {
    enum options { db0, db2, db4, db6, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using msg_a_preamb_pwr_ramp_step_r16_e_ = enumerated<msg_a_preamb_pwr_ramp_step_r16_opts>;
  struct msg_b_resp_win_r16_opts {
    enum options { sl1, sl2, sl4, sl8, sl10, sl20, sl40, sl80, sl160, sl320, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using msg_b_resp_win_r16_e_ = enumerated<msg_b_resp_win_r16_opts>;
  struct preamb_trans_max_r16_opts {
    enum options { n3, n4, n5, n6, n7, n8, n10, n20, n50, n100, n200, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using preamb_trans_max_r16_e_ = enumerated<preamb_trans_max_r16_opts>;
  struct msg_b_resp_win_v1700_opts {
    enum options { sl240, sl640, sl960, sl1280, sl1920, sl2560, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using msg_b_resp_win_v1700_e_ = enumerated<msg_b_resp_win_v1700_opts>;

  // member variables
  bool                              ext                                         = false;
  bool                              msg_a_prach_cfg_idx_r16_present             = false;
  bool                              msg_a_ro_fdm_r16_present                    = false;
  bool                              msg_a_ro_freq_start_r16_present             = false;
  bool                              msg_a_zero_correlation_zone_cfg_r16_present = false;
  bool                              msg_a_preamb_pwr_ramp_step_r16_present      = false;
  bool                              msg_a_preamb_rx_target_pwr_r16_present      = false;
  bool                              msg_b_resp_win_r16_present                  = false;
  bool                              preamb_trans_max_r16_present                = false;
  uint16_t                          msg_a_prach_cfg_idx_r16                     = 0;
  msg_a_ro_fdm_r16_e_               msg_a_ro_fdm_r16;
  uint16_t                          msg_a_ro_freq_start_r16             = 0;
  uint8_t                           msg_a_zero_correlation_zone_cfg_r16 = 0;
  msg_a_preamb_pwr_ramp_step_r16_e_ msg_a_preamb_pwr_ramp_step_r16;
  int16_t                           msg_a_preamb_rx_target_pwr_r16 = -202;
  msg_b_resp_win_r16_e_             msg_b_resp_win_r16;
  preamb_trans_max_r16_e_           preamb_trans_max_r16;
  // ...
  // group 0
  bool                    msg_b_resp_win_v1700_present = false;
  msg_b_resp_win_v1700_e_ msg_b_resp_win_v1700;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SlotBased-r16 ::= SEQUENCE
struct slot_based_r16_s {
  struct tci_map_r16_opts {
    enum options { cyclic_map, sequential_map, nulltype } value;

    const char* to_string() const;
  };
  using tci_map_r16_e_ = enumerated<tci_map_r16_opts>;

  // member variables
  tci_map_r16_e_ tci_map_r16;
  uint8_t        seq_offset_for_rv_r16 = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SlotBased-v1630 ::= SEQUENCE
struct slot_based_v1630_s {
  struct tci_map_r16_opts {
    enum options { cyclic_map, sequential_map, nulltype } value;

    const char* to_string() const;
  };
  using tci_map_r16_e_ = enumerated<tci_map_r16_opts>;

  // member variables
  tci_map_r16_e_ tci_map_r16;
  uint8_t        seq_offset_for_rv_r16 = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SPS-PUCCH-AN-r16 ::= SEQUENCE
struct sps_pucch_an_r16_s {
  bool     max_payload_size_r16_present = false;
  uint8_t  sps_pucch_an_res_id_r16      = 0;
  uint16_t max_payload_size_r16         = 4;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SSB-Configuration-r16 ::= SEQUENCE
struct ssb_cfg_r16_s {
  struct half_frame_idx_r16_opts {
    enum options { zero, one, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using half_frame_idx_r16_e_ = enumerated<half_frame_idx_r16_opts>;
  struct ssb_periodicity_r16_opts {
    enum options { ms5, ms10, ms20, ms40, ms80, ms160, spare2, spare1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ssb_periodicity_r16_e_ = enumerated<ssb_periodicity_r16_opts>;
  struct sfn0_offset_r16_s_ {
    bool     integer_sf_offset_r16_present = false;
    uint16_t sfn_offset_r16                = 0;
    uint8_t  integer_sf_offset_r16         = 0;
  };

  // member variables
  bool                   ssb_periodicity_r16_present   = false;
  bool                   sfn0_offset_r16_present       = false;
  bool                   ss_pbch_block_pwr_r16_present = false;
  uint32_t               ssb_freq_r16                  = 0;
  half_frame_idx_r16_e_  half_frame_idx_r16;
  subcarrier_spacing_e   ssb_subcarrier_spacing_r16;
  ssb_periodicity_r16_e_ ssb_periodicity_r16;
  sfn0_offset_r16_s_     sfn0_offset_r16;
  uint8_t                sfn_ssb_offset_r16    = 0;
  int8_t                 ss_pbch_block_pwr_r16 = -60;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// Alpha ::= ENUMERATED
struct alpha_opts {
  enum options { alpha0, alpha04, alpha05, alpha06, alpha07, alpha08, alpha09, alpha1, nulltype } value;
  typedef float number_type;

  const char* to_string() const;
  float       to_number() const;
  const char* to_number_string() const;
};
using alpha_e = enumerated<alpha_opts>;

// BetaOffsetsCrossPri-r17 ::= SEQUENCE (SIZE (3)) OF INTEGER (0..31)
using beta_offsets_cross_pri_r17_l = std::array<uint8_t, 3>;

// CI-ConfigurationPerServingCell-r16 ::= SEQUENCE
struct ci_cfg_per_serving_cell_r16_s {
  struct ci_payload_size_r16_opts {
    enum options { n1, n2, n4, n5, n7, n8, n10, n14, n16, n20, n28, n32, n35, n42, n56, n112, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ci_payload_size_r16_e_ = enumerated<ci_payload_size_r16_opts>;
  struct time_freq_region_r16_s_ {
    struct time_dur_for_ci_r16_opts {
      enum options { n2, n4, n7, n14, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using time_dur_for_ci_r16_e_ = enumerated<time_dur_for_ci_r16_opts>;
    struct time_granularity_for_ci_r16_opts {
      enum options { n1, n2, n4, n7, n14, n28, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using time_granularity_for_ci_r16_e_ = enumerated<time_granularity_for_ci_r16_opts>;

    // member variables
    bool                           ext                         = false;
    bool                           time_dur_for_ci_r16_present = false;
    time_dur_for_ci_r16_e_         time_dur_for_ci_r16;
    time_granularity_for_ci_r16_e_ time_granularity_for_ci_r16;
    uint16_t                       freq_region_for_ci_r16 = 0;
    uint8_t                        delta_offset_r16       = 0;
    // ...
  };

  // member variables
  bool                    position_in_dci_for_sul_r16_present = false;
  bool                    ul_cancellation_prio_v1610_present  = false;
  uint8_t                 serving_cell_id                     = 0;
  uint8_t                 position_in_dci_r16                 = 0;
  uint8_t                 position_in_dci_for_sul_r16         = 0;
  ci_payload_size_r16_e_  ci_payload_size_r16;
  time_freq_region_r16_s_ time_freq_region_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// CSI-FrequencyOccupation ::= SEQUENCE
struct csi_freq_occupation_s {
  bool     ext      = false;
  uint16_t start_rb = 0;
  uint16_t nrof_rbs = 24;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DL-DataToUL-ACK-DCI-1-2-r16 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (0..15)
using dl_data_to_ul_ack_dci_1_2_r16_l = bounded_array<uint8_t, 8>;

// DL-DataToUL-ACK-DCI-1-2-r17 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (0..127)
using dl_data_to_ul_ack_dci_1_2_r17_l = bounded_array<uint8_t, 8>;

// DL-DataToUL-ACK-MulticastDCI-Format4-1-r17 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (0..15)
using dl_data_to_ul_ack_multicast_dci_format4_1_r17_l = bounded_array<uint8_t, 8>;

// DL-DataToUL-ACK-r16 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (-1..15)
using dl_data_to_ul_ack_r16_l = bounded_array<int8_t, 8>;

// DL-DataToUL-ACK-r17 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (-1..127)
using dl_data_to_ul_ack_r17_l = bounded_array<int16_t, 8>;

// DL-DataToUL-ACK-v1700 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (16..31)
using dl_data_to_ul_ack_v1700_l = bounded_array<uint8_t, 8>;

// DL-PRS-Info-r16 ::= SEQUENCE
struct dl_prs_info_r16_s {
  bool     dl_prs_res_id_r16_present = false;
  uint16_t dl_prs_id_r16             = 0;
  uint8_t  dl_prs_res_set_id_r16     = 0;
  uint8_t  dl_prs_res_id_r16         = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DMRS-BundlingPUCCH-Config-r17 ::= SEQUENCE
struct dmrs_bundling_pucch_cfg_r17_s {
  struct pucch_freq_hop_interv_r17_opts {
    enum options { s2, s4, s5, s10, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pucch_freq_hop_interv_r17_e_ = enumerated<pucch_freq_hop_interv_r17_opts>;

  // member variables
  bool                         ext                                   = false;
  bool                         pucch_dmrs_bundling_r17_present       = false;
  bool                         pucch_time_domain_win_len_r17_present = false;
  bool                         pucch_win_restart_r17_present         = false;
  bool                         pucch_freq_hop_interv_r17_present     = false;
  uint8_t                      pucch_time_domain_win_len_r17         = 2;
  pucch_freq_hop_interv_r17_e_ pucch_freq_hop_interv_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DummyPathlossReferenceRS-v1710 ::= SEQUENCE
struct dummy_pathloss_ref_rs_v1710_s {
  bool    add_pci_r17_present          = false;
  uint8_t pusch_pathloss_ref_rs_id_r17 = 0;
  uint8_t add_pci_r17                  = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// INT-ConfigurationPerServingCell ::= SEQUENCE
struct int_cfg_per_serving_cell_s {
  uint8_t serving_cell_id = 0;
  uint8_t position_in_dci = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// MultiPDSCH-TDRA-r17 ::= SEQUENCE
struct multi_pdsch_tdra_r17_s {
  using pdsch_tdra_list_r17_l_ = dyn_array<pdsch_time_domain_res_alloc_r16_s>;

  // member variables
  bool                   ext = false;
  pdsch_tdra_list_r17_l_ pdsch_tdra_list_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// P0-PUCCH ::= SEQUENCE
struct p0_pucch_s {
  uint8_t p0_pucch_id    = 1;
  int8_t  p0_pucch_value = -16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// P0-PUSCH-Set-r16 ::= SEQUENCE
struct p0_pusch_set_r16_s {
  using p0_list_r16_l_ = bounded_array<int8_t, 2>;

  // member variables
  bool           ext                 = false;
  uint8_t        p0_pusch_set_id_r16 = 0;
  p0_list_r16_l_ p0_list_r16;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PathlossReferenceRSList-r16 ::= SEQUENCE (SIZE (1..64)) OF PathlossReferenceRS-r16
using pathloss_ref_rs_list_r16_l = dyn_array<pathloss_ref_rs_r16_s>;

// PDSCH-TimeDomainResourceAllocation ::= SEQUENCE
struct pdsch_time_domain_res_alloc_s {
  struct map_type_opts {
    enum options { type_a, type_b, nulltype } value;

    const char* to_string() const;
  };
  using map_type_e_ = enumerated<map_type_opts>;

  // member variables
  bool        k0_present = false;
  uint8_t     k0         = 0;
  map_type_e_ map_type;
  uint8_t     start_symbol_and_len = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PRACH-ResourceDedicatedBFR ::= CHOICE
struct prach_res_ded_bfr_c {
  struct types_opts {
    enum options { ssb, csi_rs, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  prach_res_ded_bfr_c() = default;
  prach_res_ded_bfr_c(const prach_res_ded_bfr_c& other);
  prach_res_ded_bfr_c& operator=(const prach_res_ded_bfr_c& other);
  ~prach_res_ded_bfr_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  bfr_ssb_res_s& ssb()
  {
    assert_choice_type(types::ssb, type_, "PRACH-ResourceDedicatedBFR");
    return c.get<bfr_ssb_res_s>();
  }
  bfr_csi_rs_res_s& csi_rs()
  {
    assert_choice_type(types::csi_rs, type_, "PRACH-ResourceDedicatedBFR");
    return c.get<bfr_csi_rs_res_s>();
  }
  const bfr_ssb_res_s& ssb() const
  {
    assert_choice_type(types::ssb, type_, "PRACH-ResourceDedicatedBFR");
    return c.get<bfr_ssb_res_s>();
  }
  const bfr_csi_rs_res_s& csi_rs() const
  {
    assert_choice_type(types::csi_rs, type_, "PRACH-ResourceDedicatedBFR");
    return c.get<bfr_csi_rs_res_s>();
  }
  bfr_ssb_res_s&    set_ssb();
  bfr_csi_rs_res_s& set_csi_rs();

private:
  types                                            type_;
  choice_buffer_t<bfr_csi_rs_res_s, bfr_ssb_res_s> c;

  void destroy_();
};

// PUCCH-format0 ::= SEQUENCE
struct pucch_format0_s {
  uint8_t init_cyclic_shift = 0;
  uint8_t nrof_symbols      = 1;
  uint8_t start_symbol_idx  = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-format1 ::= SEQUENCE
struct pucch_format1_s {
  uint8_t init_cyclic_shift = 0;
  uint8_t nrof_symbols      = 4;
  uint8_t start_symbol_idx  = 0;
  uint8_t time_domain_occ   = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-format2 ::= SEQUENCE
struct pucch_format2_s {
  uint8_t nrof_prbs        = 1;
  uint8_t nrof_symbols     = 1;
  uint8_t start_symbol_idx = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-format3 ::= SEQUENCE
struct pucch_format3_s {
  uint8_t nrof_prbs        = 1;
  uint8_t nrof_symbols     = 4;
  uint8_t start_symbol_idx = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-format4 ::= SEQUENCE
struct pucch_format4_s {
  struct occ_len_opts {
    enum options { n2, n4, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using occ_len_e_ = enumerated<occ_len_opts>;
  struct occ_idx_opts {
    enum options { n0, n1, n2, n3, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using occ_idx_e_ = enumerated<occ_idx_opts>;

  // member variables
  uint8_t    nrof_symbols = 4;
  occ_len_e_ occ_len;
  occ_idx_e_ occ_idx;
  uint8_t    start_symbol_idx = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-FormatConfigExt-r17 ::= SEQUENCE
struct pucch_format_cfg_ext_r17_s {
  bool                  ext                          = false;
  bool                  max_code_rate_lp_r17_present = false;
  pucch_max_code_rate_e max_code_rate_lp_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-FormatConfig ::= SEQUENCE
struct pucch_format_cfg_s {
  struct nrof_slots_opts {
    enum options { n2, n4, n8, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using nrof_slots_e_ = enumerated<nrof_slots_opts>;

  // member variables
  bool                  interslot_freq_hop_present = false;
  bool                  add_dmrs_present           = false;
  bool                  max_code_rate_present      = false;
  bool                  nrof_slots_present         = false;
  bool                  pi2_bpsk_present           = false;
  bool                  simul_harq_ack_csi_present = false;
  pucch_max_code_rate_e max_code_rate;
  nrof_slots_e_         nrof_slots;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-PathlossReferenceRS ::= SEQUENCE
struct pucch_pathloss_ref_rs_s {
  struct ref_sig_c_ {
    struct types_opts {
      enum options { ssb_idx, csi_rs_idx, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ref_sig_c_() = default;
    ref_sig_c_(const ref_sig_c_& other);
    ref_sig_c_& operator=(const ref_sig_c_& other);
    ~ref_sig_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx()
    {
      assert_choice_type(types::ssb_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx()
    {
      assert_choice_type(types::csi_rs_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_idx() const
    {
      assert_choice_type(types::ssb_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx() const
    {
      assert_choice_type(types::csi_rs_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_idx();
    uint8_t& set_csi_rs_idx();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  uint8_t    pucch_pathloss_ref_rs_id = 0;
  ref_sig_c_ ref_sig;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-SRS ::= SEQUENCE
struct pucch_srs_s {
  uint8_t res    = 0;
  uint8_t ul_bwp = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUSCH-PathlossReferenceRS-r16 ::= SEQUENCE
struct pusch_pathloss_ref_rs_r16_s {
  struct ref_sig_r16_c_ {
    struct types_opts {
      enum options { ssb_idx_r16, csi_rs_idx_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ref_sig_r16_c_() = default;
    ref_sig_r16_c_(const ref_sig_r16_c_& other);
    ref_sig_r16_c_& operator=(const ref_sig_r16_c_& other);
    ~ref_sig_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx_r16()
    {
      assert_choice_type(types::ssb_idx_r16, type_, "referenceSignal-r16");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx_r16()
    {
      assert_choice_type(types::csi_rs_idx_r16, type_, "referenceSignal-r16");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_idx_r16() const
    {
      assert_choice_type(types::ssb_idx_r16, type_, "referenceSignal-r16");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx_r16() const
    {
      assert_choice_type(types::csi_rs_idx_r16, type_, "referenceSignal-r16");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_idx_r16();
    uint8_t& set_csi_rs_idx_r16();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  uint8_t        pusch_pathloss_ref_rs_id_r16 = 4;
  ref_sig_r16_c_ ref_sig_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUSCH-TimeDomainResourceAllocation-r16 ::= SEQUENCE
struct pusch_time_domain_res_alloc_r16_s {
  using pusch_alloc_list_r16_l_ = dyn_array<pusch_alloc_r16_s>;

  // member variables
  bool                    ext            = false;
  bool                    k2_r16_present = false;
  uint8_t                 k2_r16         = 0;
  pusch_alloc_list_r16_l_ pusch_alloc_list_r16;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUSCH-TimeDomainResourceAllocation ::= SEQUENCE
struct pusch_time_domain_res_alloc_s {
  struct map_type_opts {
    enum options { type_a, type_b, nulltype } value;

    const char* to_string() const;
  };
  using map_type_e_ = enumerated<map_type_opts>;

  // member variables
  bool        k2_present = false;
  uint8_t     k2         = 0;
  map_type_e_ map_type;
  uint8_t     start_symbol_and_len = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RACH-ConfigCommonTwoStepRA-r16 ::= SEQUENCE
struct rach_cfg_common_two_step_ra_r16_s {
  struct msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_c_ {
    struct one_eighth_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, n36, n40, n44, n48, n52, n56, n60, n64, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using one_eighth_e_ = enumerated<one_eighth_opts>;
    struct one_fourth_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, n36, n40, n44, n48, n52, n56, n60, n64, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using one_fourth_e_ = enumerated<one_fourth_opts>;
    struct one_half_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, n36, n40, n44, n48, n52, n56, n60, n64, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using one_half_e_ = enumerated<one_half_opts>;
    struct one_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, n36, n40, n44, n48, n52, n56, n60, n64, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using one_e_ = enumerated<one_opts>;
    struct two_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using two_e_ = enumerated<two_opts>;
    struct types_opts {
      enum options { one_eighth, one_fourth, one_half, one, two, four, eight, sixteen, nulltype } value;
      typedef float number_type;

      const char* to_string() const;
      float       to_number() const;
      const char* to_number_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_c_() = default;
    msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_c_(
        const msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_c_& other);
    msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_c_&
    operator=(const msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_c_& other);
    ~msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    one_eighth_e_& one_eighth()
    {
      assert_choice_type(types::one_eighth, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<one_eighth_e_>();
    }
    one_fourth_e_& one_fourth()
    {
      assert_choice_type(types::one_fourth, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<one_fourth_e_>();
    }
    one_half_e_& one_half()
    {
      assert_choice_type(types::one_half, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<one_half_e_>();
    }
    one_e_& one()
    {
      assert_choice_type(types::one, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<one_e_>();
    }
    two_e_& two()
    {
      assert_choice_type(types::two, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<two_e_>();
    }
    uint8_t& four()
    {
      assert_choice_type(types::four, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<uint8_t>();
    }
    uint8_t& eight()
    {
      assert_choice_type(types::eight, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<uint8_t>();
    }
    uint8_t& sixteen()
    {
      assert_choice_type(types::sixteen, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<uint8_t>();
    }
    const one_eighth_e_& one_eighth() const
    {
      assert_choice_type(types::one_eighth, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<one_eighth_e_>();
    }
    const one_fourth_e_& one_fourth() const
    {
      assert_choice_type(types::one_fourth, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<one_fourth_e_>();
    }
    const one_half_e_& one_half() const
    {
      assert_choice_type(types::one_half, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<one_half_e_>();
    }
    const one_e_& one() const
    {
      assert_choice_type(types::one, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<one_e_>();
    }
    const two_e_& two() const
    {
      assert_choice_type(types::two, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<two_e_>();
    }
    const uint8_t& four() const
    {
      assert_choice_type(types::four, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<uint8_t>();
    }
    const uint8_t& eight() const
    {
      assert_choice_type(types::eight, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<uint8_t>();
    }
    const uint8_t& sixteen() const
    {
      assert_choice_type(types::sixteen, type_, "msgA-SSB-PerRACH-OccasionAndCB-PreamblesPerSSB-r16");
      return c.get<uint8_t>();
    }
    one_eighth_e_& set_one_eighth();
    one_fourth_e_& set_one_fourth();
    one_half_e_&   set_one_half();
    one_e_&        set_one();
    two_e_&        set_two();
    uint8_t&       set_four();
    uint8_t&       set_eight();
    uint8_t&       set_sixteen();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct msg_a_prach_root_seq_idx_r16_c_ {
    struct types_opts {
      enum options { l839, l139, l571, l1151, nulltype } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    msg_a_prach_root_seq_idx_r16_c_() = default;
    msg_a_prach_root_seq_idx_r16_c_(const msg_a_prach_root_seq_idx_r16_c_& other);
    msg_a_prach_root_seq_idx_r16_c_& operator=(const msg_a_prach_root_seq_idx_r16_c_& other);
    ~msg_a_prach_root_seq_idx_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint16_t& l839()
    {
      assert_choice_type(types::l839, type_, "msgA-PRACH-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    uint8_t& l139()
    {
      assert_choice_type(types::l139, type_, "msgA-PRACH-RootSequenceIndex-r16");
      return c.get<uint8_t>();
    }
    uint16_t& l571()
    {
      assert_choice_type(types::l571, type_, "msgA-PRACH-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    uint16_t& l1151()
    {
      assert_choice_type(types::l1151, type_, "msgA-PRACH-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    const uint16_t& l839() const
    {
      assert_choice_type(types::l839, type_, "msgA-PRACH-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    const uint8_t& l139() const
    {
      assert_choice_type(types::l139, type_, "msgA-PRACH-RootSequenceIndex-r16");
      return c.get<uint8_t>();
    }
    const uint16_t& l571() const
    {
      assert_choice_type(types::l571, type_, "msgA-PRACH-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    const uint16_t& l1151() const
    {
      assert_choice_type(types::l1151, type_, "msgA-PRACH-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    uint16_t& set_l839();
    uint8_t&  set_l139();
    uint16_t& set_l571();
    uint16_t& set_l1151();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct msg_a_trans_max_r16_opts {
    enum options { n1, n2, n4, n6, n8, n10, n20, n50, n100, n200, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using msg_a_trans_max_r16_e_ = enumerated<msg_a_trans_max_r16_opts>;
  struct msg_a_restricted_set_cfg_r16_opts {
    enum options { unrestricted_set, restricted_set_type_a, restricted_set_type_b, nulltype } value;

    const char* to_string() const;
  };
  using msg_a_restricted_set_cfg_r16_e_ = enumerated<msg_a_restricted_set_cfg_r16_opts>;
  struct ra_prioritization_for_access_id_two_step_r16_s_ {
    ra_prioritization_s ra_prioritization_r16;
    fixed_bitstring<2>  ra_prioritization_for_ai_r16;
  };
  struct ra_contention_resolution_timer_r16_opts {
    enum options { sf8, sf16, sf24, sf32, sf40, sf48, sf56, sf64, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ra_contention_resolution_timer_r16_e_   = enumerated<ra_contention_resolution_timer_r16_opts>;
  using feature_combination_preambs_list_r17_l_ = dyn_array<feature_combination_preambs_r17_s>;

  // member variables
  bool                               ext                                                            = false;
  bool                               msg_a_total_nof_ra_preambs_r16_present                         = false;
  bool                               msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_present = false;
  bool                               msg_a_cb_preambs_per_ssb_per_shared_ro_r16_present             = false;
  bool                               msg_a_ssb_shared_ro_mask_idx_r16_present                       = false;
  bool                               group_b_cfg_two_step_ra_r16_present                            = false;
  bool                               msg_a_prach_root_seq_idx_r16_present                           = false;
  bool                               msg_a_trans_max_r16_present                                    = false;
  bool                               msg_a_rsrp_thres_r16_present                                   = false;
  bool                               msg_a_rsrp_thres_ssb_r16_present                               = false;
  bool                               msg_a_subcarrier_spacing_r16_present                           = false;
  bool                               msg_a_restricted_set_cfg_r16_present                           = false;
  bool                               ra_prioritization_for_access_id_two_step_r16_present           = false;
  bool                               ra_contention_resolution_timer_r16_present                     = false;
  rach_cfg_generic_two_step_ra_r16_s rach_cfg_generic_two_step_ra_r16;
  uint8_t                            msg_a_total_nof_ra_preambs_r16 = 1;
  msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16_c_ msg_a_ssb_per_rach_occasion_and_cb_preambs_per_ssb_r16;
  uint8_t                                                   msg_a_cb_preambs_per_ssb_per_shared_ro_r16 = 1;
  uint8_t                                                   msg_a_ssb_shared_ro_mask_idx_r16           = 1;
  group_b_cfg_two_step_ra_r16_s                             group_b_cfg_two_step_ra_r16;
  msg_a_prach_root_seq_idx_r16_c_                           msg_a_prach_root_seq_idx_r16;
  msg_a_trans_max_r16_e_                                    msg_a_trans_max_r16;
  uint8_t                                                   msg_a_rsrp_thres_r16     = 0;
  uint8_t                                                   msg_a_rsrp_thres_ssb_r16 = 0;
  subcarrier_spacing_e                                      msg_a_subcarrier_spacing_r16;
  msg_a_restricted_set_cfg_r16_e_                           msg_a_restricted_set_cfg_r16;
  ra_prioritization_for_access_id_two_step_r16_s_           ra_prioritization_for_access_id_two_step_r16;
  ra_contention_resolution_timer_r16_e_                     ra_contention_resolution_timer_r16;
  // ...
  // group 0
  copy_ptr<ra_prioritization_for_slicing_r17_s>     ra_prioritization_for_slicing_two_step_r17;
  copy_ptr<feature_combination_preambs_list_r17_l_> feature_combination_preambs_list_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RACH-ConfigGeneric ::= SEQUENCE
struct rach_cfg_generic_s {
  struct msg1_fdm_opts {
    enum options { one, two, four, eight, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using msg1_fdm_e_ = enumerated<msg1_fdm_opts>;
  struct preamb_trans_max_opts {
    enum options { n3, n4, n5, n6, n7, n8, n10, n20, n50, n100, n200, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using preamb_trans_max_e_ = enumerated<preamb_trans_max_opts>;
  struct pwr_ramp_step_opts {
    enum options { db0, db2, db4, db6, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pwr_ramp_step_e_ = enumerated<pwr_ramp_step_opts>;
  struct ra_resp_win_opts {
    enum options { sl1, sl2, sl4, sl8, sl10, sl20, sl40, sl80, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ra_resp_win_e_ = enumerated<ra_resp_win_opts>;
  struct prach_cfg_period_scaling_iab_r16_opts {
    enum options { scf1, scf2, scf4, scf8, scf16, scf32, scf64, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using prach_cfg_period_scaling_iab_r16_e_ = enumerated<prach_cfg_period_scaling_iab_r16_opts>;
  struct ra_resp_win_v1610_opts {
    enum options { sl60, sl160, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ra_resp_win_v1610_e_ = enumerated<ra_resp_win_v1610_opts>;
  struct ra_resp_win_v1700_opts {
    enum options { sl240, sl320, sl640, sl960, sl1280, sl1920, sl2560, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using ra_resp_win_v1700_e_ = enumerated<ra_resp_win_v1700_opts>;

  // member variables
  bool                ext           = false;
  uint16_t            prach_cfg_idx = 0;
  msg1_fdm_e_         msg1_fdm;
  uint16_t            msg1_freq_start           = 0;
  uint8_t             zero_correlation_zone_cfg = 0;
  int16_t             preamb_rx_target_pwr      = -202;
  preamb_trans_max_e_ preamb_trans_max;
  pwr_ramp_step_e_    pwr_ramp_step;
  ra_resp_win_e_      ra_resp_win;
  // ...
  // group 0
  bool                                prach_cfg_period_scaling_iab_r16_present = false;
  bool                                prach_cfg_frame_offset_iab_r16_present   = false;
  bool                                prach_cfg_s_offset_iab_r16_present       = false;
  bool                                ra_resp_win_v1610_present                = false;
  bool                                prach_cfg_idx_v1610_present              = false;
  prach_cfg_period_scaling_iab_r16_e_ prach_cfg_period_scaling_iab_r16;
  uint8_t                             prach_cfg_frame_offset_iab_r16 = 0;
  uint8_t                             prach_cfg_s_offset_iab_r16     = 0;
  ra_resp_win_v1610_e_                ra_resp_win_v1610;
  uint16_t                            prach_cfg_idx_v1610 = 256;
  // group 1
  bool                 ra_resp_win_v1700_present = false;
  ra_resp_win_v1700_e_ ra_resp_win_v1700;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SPS-PUCCH-AN-List-r16 ::= SEQUENCE (SIZE (1..4)) OF SPS-PUCCH-AN-r16
using sps_pucch_an_list_r16_l = dyn_array<sps_pucch_an_r16_s>;

// SRI-PUSCH-PowerControl ::= SEQUENCE
struct sri_pusch_pwr_ctrl_s {
  struct sri_pusch_closed_loop_idx_opts {
    enum options { i0, i1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using sri_pusch_closed_loop_idx_e_ = enumerated<sri_pusch_closed_loop_idx_opts>;

  // member variables
  uint8_t                      sri_pusch_pwr_ctrl_id        = 0;
  uint8_t                      sri_pusch_pathloss_ref_rs_id = 0;
  uint8_t                      sri_p0_pusch_alpha_set_id    = 0;
  sri_pusch_closed_loop_idx_e_ sri_pusch_closed_loop_idx;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SSB-InfoNcell-r16 ::= SEQUENCE
struct ssb_info_ncell_r16_s {
  bool          ssb_idx_ncell_r16_present = false;
  bool          ssb_cfg_r16_present       = false;
  uint16_t      phys_cell_id_r16          = 0;
  uint8_t       ssb_idx_ncell_r16         = 0;
  ssb_cfg_r16_s ssb_cfg_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// UCI-OnPUSCH-DCI-0-2-r16 ::= SEQUENCE
struct uci_on_pusch_dci_0_2_r16_s {
  struct beta_offsets_dci_0_2_r16_c_ {
    struct dyn_dci_0_2_r16_c_ {
      using one_bit_r16_l_  = std::array<beta_offsets_s, 2>;
      using two_bits_r16_l_ = std::array<beta_offsets_s, 4>;
      struct types_opts {
        enum options { one_bit_r16, two_bits_r16, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using types = enumerated<types_opts>;

      // choice methods
      dyn_dci_0_2_r16_c_() = default;
      dyn_dci_0_2_r16_c_(const dyn_dci_0_2_r16_c_& other);
      dyn_dci_0_2_r16_c_& operator=(const dyn_dci_0_2_r16_c_& other);
      ~dyn_dci_0_2_r16_c_() { destroy_(); }
      void        set(types::options e = types::nulltype);
      types       type() const { return type_; }
      SRSASN_CODE pack(bit_ref& bref) const;
      SRSASN_CODE unpack(cbit_ref& bref);
      void        to_json(json_writer& j) const;
      // getters
      one_bit_r16_l_& one_bit_r16()
      {
        assert_choice_type(types::one_bit_r16, type_, "dynamicDCI-0-2-r16");
        return c.get<one_bit_r16_l_>();
      }
      two_bits_r16_l_& two_bits_r16()
      {
        assert_choice_type(types::two_bits_r16, type_, "dynamicDCI-0-2-r16");
        return c.get<two_bits_r16_l_>();
      }
      const one_bit_r16_l_& one_bit_r16() const
      {
        assert_choice_type(types::one_bit_r16, type_, "dynamicDCI-0-2-r16");
        return c.get<one_bit_r16_l_>();
      }
      const two_bits_r16_l_& two_bits_r16() const
      {
        assert_choice_type(types::two_bits_r16, type_, "dynamicDCI-0-2-r16");
        return c.get<two_bits_r16_l_>();
      }
      one_bit_r16_l_&  set_one_bit_r16();
      two_bits_r16_l_& set_two_bits_r16();

    private:
      types                                            type_;
      choice_buffer_t<one_bit_r16_l_, two_bits_r16_l_> c;

      void destroy_();
    };
    struct types_opts {
      enum options { dyn_dci_0_2_r16, semi_static_dci_0_2_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    beta_offsets_dci_0_2_r16_c_() = default;
    beta_offsets_dci_0_2_r16_c_(const beta_offsets_dci_0_2_r16_c_& other);
    beta_offsets_dci_0_2_r16_c_& operator=(const beta_offsets_dci_0_2_r16_c_& other);
    ~beta_offsets_dci_0_2_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    dyn_dci_0_2_r16_c_& dyn_dci_0_2_r16()
    {
      assert_choice_type(types::dyn_dci_0_2_r16, type_, "betaOffsetsDCI-0-2-r16");
      return c.get<dyn_dci_0_2_r16_c_>();
    }
    beta_offsets_s& semi_static_dci_0_2_r16()
    {
      assert_choice_type(types::semi_static_dci_0_2_r16, type_, "betaOffsetsDCI-0-2-r16");
      return c.get<beta_offsets_s>();
    }
    const dyn_dci_0_2_r16_c_& dyn_dci_0_2_r16() const
    {
      assert_choice_type(types::dyn_dci_0_2_r16, type_, "betaOffsetsDCI-0-2-r16");
      return c.get<dyn_dci_0_2_r16_c_>();
    }
    const beta_offsets_s& semi_static_dci_0_2_r16() const
    {
      assert_choice_type(types::semi_static_dci_0_2_r16, type_, "betaOffsetsDCI-0-2-r16");
      return c.get<beta_offsets_s>();
    }
    dyn_dci_0_2_r16_c_& set_dyn_dci_0_2_r16();
    beta_offsets_s&     set_semi_static_dci_0_2_r16();

  private:
    types                                               type_;
    choice_buffer_t<beta_offsets_s, dyn_dci_0_2_r16_c_> c;

    void destroy_();
  };
  struct scaling_dci_0_2_r16_opts {
    enum options { f0p5, f0p65, f0p8, f1, nulltype } value;

    const char* to_string() const;
  };
  using scaling_dci_0_2_r16_e_ = enumerated<scaling_dci_0_2_r16_opts>;

  // member variables
  bool                        beta_offsets_dci_0_2_r16_present = false;
  beta_offsets_dci_0_2_r16_c_ beta_offsets_dci_0_2_r16;
  scaling_dci_0_2_r16_e_      scaling_dci_0_2_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// UCI-OnPUSCH ::= SEQUENCE
struct uci_on_pusch_s {
  struct beta_offsets_c_ {
    using dyn_l_ = std::array<beta_offsets_s, 4>;
    struct types_opts {
      enum options { dyn, semi_static, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    beta_offsets_c_() = default;
    beta_offsets_c_(const beta_offsets_c_& other);
    beta_offsets_c_& operator=(const beta_offsets_c_& other);
    ~beta_offsets_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    dyn_l_& dyn()
    {
      assert_choice_type(types::dyn, type_, "betaOffsets");
      return c.get<dyn_l_>();
    }
    beta_offsets_s& semi_static()
    {
      assert_choice_type(types::semi_static, type_, "betaOffsets");
      return c.get<beta_offsets_s>();
    }
    const dyn_l_& dyn() const
    {
      assert_choice_type(types::dyn, type_, "betaOffsets");
      return c.get<dyn_l_>();
    }
    const beta_offsets_s& semi_static() const
    {
      assert_choice_type(types::semi_static, type_, "betaOffsets");
      return c.get<beta_offsets_s>();
    }
    dyn_l_&         set_dyn();
    beta_offsets_s& set_semi_static();

  private:
    types                                   type_;
    choice_buffer_t<beta_offsets_s, dyn_l_> c;

    void destroy_();
  };
  struct scaling_opts {
    enum options { f0p5, f0p65, f0p8, f1, nulltype } value;

    const char* to_string() const;
  };
  using scaling_e_ = enumerated<scaling_opts>;

  // member variables
  bool            beta_offsets_present = false;
  beta_offsets_c_ beta_offsets;
  scaling_e_      scaling;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// UL-AccessConfigListDCI-1-1-r16 ::= SEQUENCE (SIZE (1..16)) OF INTEGER (0..15)
using ul_access_cfg_list_dci_1_1_r16_l = bounded_array<uint8_t, 16>;

// UL-AccessConfigListDCI-1-1-r17 ::= SEQUENCE (SIZE (1..3)) OF INTEGER (0..2)
using ul_access_cfg_list_dci_1_1_r17_l = bounded_array<uint8_t, 3>;

// UL-AccessConfigListDCI-1-2-r17 ::= SEQUENCE (SIZE (1..16)) OF INTEGER (0..15)
using ul_access_cfg_list_dci_1_2_r17_l = bounded_array<uint8_t, 16>;

// BeamLinkMonitoringRS-r17 ::= SEQUENCE
struct beam_link_monitoring_rs_r17_s {
  struct detection_res_r17_c_ {
    struct types_opts {
      enum options { ssb_idx, csi_rs_idx, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    detection_res_r17_c_() = default;
    detection_res_r17_c_(const detection_res_r17_c_& other);
    detection_res_r17_c_& operator=(const detection_res_r17_c_& other);
    ~detection_res_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx()
    {
      assert_choice_type(types::ssb_idx, type_, "detectionResource-r17");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx()
    {
      assert_choice_type(types::csi_rs_idx, type_, "detectionResource-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_idx() const
    {
      assert_choice_type(types::ssb_idx, type_, "detectionResource-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx() const
    {
      assert_choice_type(types::csi_rs_idx, type_, "detectionResource-r17");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_idx();
    uint8_t& set_csi_rs_idx();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  bool                 ext                            = false;
  uint8_t              beam_link_monitoring_rs_id_r17 = 0;
  detection_res_r17_c_ detection_res_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BetaOffsetsCrossPriSelCG-r17 ::= CHOICE
struct beta_offsets_cross_pri_sel_cg_r17_c {
  using dyn_r17_l_ = dyn_array<beta_offsets_cross_pri_r17_l>;
  struct types_opts {
    enum options { dyn_r17, semi_static_r17, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  beta_offsets_cross_pri_sel_cg_r17_c() = default;
  beta_offsets_cross_pri_sel_cg_r17_c(const beta_offsets_cross_pri_sel_cg_r17_c& other);
  beta_offsets_cross_pri_sel_cg_r17_c& operator=(const beta_offsets_cross_pri_sel_cg_r17_c& other);
  ~beta_offsets_cross_pri_sel_cg_r17_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  dyn_r17_l_& dyn_r17()
  {
    assert_choice_type(types::dyn_r17, type_, "BetaOffsetsCrossPriSelCG-r17");
    return c.get<dyn_r17_l_>();
  }
  beta_offsets_cross_pri_r17_l& semi_static_r17()
  {
    assert_choice_type(types::semi_static_r17, type_, "BetaOffsetsCrossPriSelCG-r17");
    return c.get<beta_offsets_cross_pri_r17_l>();
  }
  const dyn_r17_l_& dyn_r17() const
  {
    assert_choice_type(types::dyn_r17, type_, "BetaOffsetsCrossPriSelCG-r17");
    return c.get<dyn_r17_l_>();
  }
  const beta_offsets_cross_pri_r17_l& semi_static_r17() const
  {
    assert_choice_type(types::semi_static_r17, type_, "BetaOffsetsCrossPriSelCG-r17");
    return c.get<beta_offsets_cross_pri_r17_l>();
  }
  dyn_r17_l_&                   set_dyn_r17();
  beta_offsets_cross_pri_r17_l& set_semi_static_r17();

private:
  types                                                     type_;
  choice_buffer_t<beta_offsets_cross_pri_r17_l, dyn_r17_l_> c;

  void destroy_();
};

// BetaOffsetsCrossPriSelDCI-0-2-r17 ::= CHOICE
struct beta_offsets_cross_pri_sel_dci_0_2_r17_c {
  struct dyn_dci_0_2_r17_c_ {
    using one_bit_r17_l_  = std::array<beta_offsets_cross_pri_r17_l, 2>;
    using two_bits_r17_l_ = std::array<beta_offsets_cross_pri_r17_l, 4>;
    struct types_opts {
      enum options { one_bit_r17, two_bits_r17, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    dyn_dci_0_2_r17_c_() = default;
    dyn_dci_0_2_r17_c_(const dyn_dci_0_2_r17_c_& other);
    dyn_dci_0_2_r17_c_& operator=(const dyn_dci_0_2_r17_c_& other);
    ~dyn_dci_0_2_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    one_bit_r17_l_& one_bit_r17()
    {
      assert_choice_type(types::one_bit_r17, type_, "dynamicDCI-0-2-r17");
      return c.get<one_bit_r17_l_>();
    }
    two_bits_r17_l_& two_bits_r17()
    {
      assert_choice_type(types::two_bits_r17, type_, "dynamicDCI-0-2-r17");
      return c.get<two_bits_r17_l_>();
    }
    const one_bit_r17_l_& one_bit_r17() const
    {
      assert_choice_type(types::one_bit_r17, type_, "dynamicDCI-0-2-r17");
      return c.get<one_bit_r17_l_>();
    }
    const two_bits_r17_l_& two_bits_r17() const
    {
      assert_choice_type(types::two_bits_r17, type_, "dynamicDCI-0-2-r17");
      return c.get<two_bits_r17_l_>();
    }
    one_bit_r17_l_&  set_one_bit_r17();
    two_bits_r17_l_& set_two_bits_r17();

  private:
    types                                            type_;
    choice_buffer_t<one_bit_r17_l_, two_bits_r17_l_> c;

    void destroy_();
  };
  struct types_opts {
    enum options { dyn_dci_0_2_r17, semi_static_dci_0_2_r17, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  beta_offsets_cross_pri_sel_dci_0_2_r17_c() = default;
  beta_offsets_cross_pri_sel_dci_0_2_r17_c(const beta_offsets_cross_pri_sel_dci_0_2_r17_c& other);
  beta_offsets_cross_pri_sel_dci_0_2_r17_c& operator=(const beta_offsets_cross_pri_sel_dci_0_2_r17_c& other);
  ~beta_offsets_cross_pri_sel_dci_0_2_r17_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  dyn_dci_0_2_r17_c_& dyn_dci_0_2_r17()
  {
    assert_choice_type(types::dyn_dci_0_2_r17, type_, "BetaOffsetsCrossPriSelDCI-0-2-r17");
    return c.get<dyn_dci_0_2_r17_c_>();
  }
  beta_offsets_cross_pri_r17_l& semi_static_dci_0_2_r17()
  {
    assert_choice_type(types::semi_static_dci_0_2_r17, type_, "BetaOffsetsCrossPriSelDCI-0-2-r17");
    return c.get<beta_offsets_cross_pri_r17_l>();
  }
  const dyn_dci_0_2_r17_c_& dyn_dci_0_2_r17() const
  {
    assert_choice_type(types::dyn_dci_0_2_r17, type_, "BetaOffsetsCrossPriSelDCI-0-2-r17");
    return c.get<dyn_dci_0_2_r17_c_>();
  }
  const beta_offsets_cross_pri_r17_l& semi_static_dci_0_2_r17() const
  {
    assert_choice_type(types::semi_static_dci_0_2_r17, type_, "BetaOffsetsCrossPriSelDCI-0-2-r17");
    return c.get<beta_offsets_cross_pri_r17_l>();
  }
  dyn_dci_0_2_r17_c_&           set_dyn_dci_0_2_r17();
  beta_offsets_cross_pri_r17_l& set_semi_static_dci_0_2_r17();

private:
  types                                                             type_;
  choice_buffer_t<beta_offsets_cross_pri_r17_l, dyn_dci_0_2_r17_c_> c;

  void destroy_();
};

// BetaOffsetsCrossPriSel-r17 ::= CHOICE
struct beta_offsets_cross_pri_sel_r17_c {
  using dyn_r17_l_ = std::array<beta_offsets_cross_pri_r17_l, 4>;
  struct types_opts {
    enum options { dyn_r17, semi_static_r17, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  beta_offsets_cross_pri_sel_r17_c() = default;
  beta_offsets_cross_pri_sel_r17_c(const beta_offsets_cross_pri_sel_r17_c& other);
  beta_offsets_cross_pri_sel_r17_c& operator=(const beta_offsets_cross_pri_sel_r17_c& other);
  ~beta_offsets_cross_pri_sel_r17_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  dyn_r17_l_& dyn_r17()
  {
    assert_choice_type(types::dyn_r17, type_, "BetaOffsetsCrossPriSel-r17");
    return c.get<dyn_r17_l_>();
  }
  beta_offsets_cross_pri_r17_l& semi_static_r17()
  {
    assert_choice_type(types::semi_static_r17, type_, "BetaOffsetsCrossPriSel-r17");
    return c.get<beta_offsets_cross_pri_r17_l>();
  }
  const dyn_r17_l_& dyn_r17() const
  {
    assert_choice_type(types::dyn_r17, type_, "BetaOffsetsCrossPriSel-r17");
    return c.get<dyn_r17_l_>();
  }
  const beta_offsets_cross_pri_r17_l& semi_static_r17() const
  {
    assert_choice_type(types::semi_static_r17, type_, "BetaOffsetsCrossPriSel-r17");
    return c.get<beta_offsets_cross_pri_r17_l>();
  }
  dyn_r17_l_&                   set_dyn_r17();
  beta_offsets_cross_pri_r17_l& set_semi_static_r17();

private:
  types                                                     type_;
  choice_buffer_t<beta_offsets_cross_pri_r17_l, dyn_r17_l_> c;

  void destroy_();
};

// CandidateBeamRSListExt-r16 ::= SEQUENCE (SIZE (1..48)) OF PRACH-ResourceDedicatedBFR
using candidate_beam_rs_list_ext_r16_l = dyn_array<prach_res_ded_bfr_c>;

// CellGroupForSwitch-r16 ::= SEQUENCE (SIZE (1..16)) OF INTEGER (0..31)
using cell_group_for_switch_r16_l = bounded_array<uint8_t, 16>;

// CG-UCI-OnPUSCH ::= CHOICE
struct cg_uci_on_pusch_c {
  using dyn_l_ = dyn_array<beta_offsets_s>;
  struct types_opts {
    enum options { dyn, semi_static, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  cg_uci_on_pusch_c() = default;
  cg_uci_on_pusch_c(const cg_uci_on_pusch_c& other);
  cg_uci_on_pusch_c& operator=(const cg_uci_on_pusch_c& other);
  ~cg_uci_on_pusch_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  dyn_l_& dyn()
  {
    assert_choice_type(types::dyn, type_, "CG-UCI-OnPUSCH");
    return c.get<dyn_l_>();
  }
  beta_offsets_s& semi_static()
  {
    assert_choice_type(types::semi_static, type_, "CG-UCI-OnPUSCH");
    return c.get<beta_offsets_s>();
  }
  const dyn_l_& dyn() const
  {
    assert_choice_type(types::dyn, type_, "CG-UCI-OnPUSCH");
    return c.get<dyn_l_>();
  }
  const beta_offsets_s& semi_static() const
  {
    assert_choice_type(types::semi_static, type_, "CG-UCI-OnPUSCH");
    return c.get<beta_offsets_s>();
  }
  dyn_l_&         set_dyn();
  beta_offsets_s& set_semi_static();

private:
  types                                   type_;
  choice_buffer_t<beta_offsets_s, dyn_l_> c;

  void destroy_();
};

// CSI-ResourcePeriodicityAndOffset ::= CHOICE
struct csi_res_periodicity_and_offset_c {
  struct types_opts {
    enum options {
      slots4,
      slots5,
      slots8,
      slots10,
      slots16,
      slots20,
      slots32,
      slots40,
      slots64,
      slots80,
      slots160,
      slots320,
      slots640,
      nulltype
    } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  csi_res_periodicity_and_offset_c() = default;
  csi_res_periodicity_and_offset_c(const csi_res_periodicity_and_offset_c& other);
  csi_res_periodicity_and_offset_c& operator=(const csi_res_periodicity_and_offset_c& other);
  ~csi_res_periodicity_and_offset_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  uint8_t& slots4()
  {
    assert_choice_type(types::slots4, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots5()
  {
    assert_choice_type(types::slots5, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots8()
  {
    assert_choice_type(types::slots8, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots10()
  {
    assert_choice_type(types::slots10, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots16()
  {
    assert_choice_type(types::slots16, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots20()
  {
    assert_choice_type(types::slots20, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots32()
  {
    assert_choice_type(types::slots32, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots40()
  {
    assert_choice_type(types::slots40, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots64()
  {
    assert_choice_type(types::slots64, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots80()
  {
    assert_choice_type(types::slots80, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint8_t& slots160()
  {
    assert_choice_type(types::slots160, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  uint16_t& slots320()
  {
    assert_choice_type(types::slots320, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint16_t>();
  }
  uint16_t& slots640()
  {
    assert_choice_type(types::slots640, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint16_t>();
  }
  const uint8_t& slots4() const
  {
    assert_choice_type(types::slots4, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots5() const
  {
    assert_choice_type(types::slots5, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots8() const
  {
    assert_choice_type(types::slots8, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots10() const
  {
    assert_choice_type(types::slots10, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots16() const
  {
    assert_choice_type(types::slots16, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots20() const
  {
    assert_choice_type(types::slots20, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots32() const
  {
    assert_choice_type(types::slots32, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots40() const
  {
    assert_choice_type(types::slots40, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots64() const
  {
    assert_choice_type(types::slots64, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots80() const
  {
    assert_choice_type(types::slots80, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint8_t& slots160() const
  {
    assert_choice_type(types::slots160, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint8_t>();
  }
  const uint16_t& slots320() const
  {
    assert_choice_type(types::slots320, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint16_t>();
  }
  const uint16_t& slots640() const
  {
    assert_choice_type(types::slots640, type_, "CSI-ResourcePeriodicityAndOffset");
    return c.get<uint16_t>();
  }
  uint8_t&  set_slots4();
  uint8_t&  set_slots5();
  uint8_t&  set_slots8();
  uint8_t&  set_slots10();
  uint8_t&  set_slots16();
  uint8_t&  set_slots20();
  uint8_t&  set_slots32();
  uint8_t&  set_slots40();
  uint8_t&  set_slots64();
  uint8_t&  set_slots80();
  uint8_t&  set_slots160();
  uint16_t& set_slots320();
  uint16_t& set_slots640();

private:
  types               type_;
  pod_choice_buffer_t c;

  void destroy_();
};

// CSI-RS-ResourceMapping ::= SEQUENCE
struct csi_rs_res_map_s {
  struct freq_domain_alloc_c_ {
    struct types_opts {
      enum options { row1, row2, row4, other, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    freq_domain_alloc_c_() = default;
    freq_domain_alloc_c_(const freq_domain_alloc_c_& other);
    freq_domain_alloc_c_& operator=(const freq_domain_alloc_c_& other);
    ~freq_domain_alloc_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    fixed_bitstring<4>& row1()
    {
      assert_choice_type(types::row1, type_, "frequencyDomainAllocation");
      return c.get<fixed_bitstring<4>>();
    }
    fixed_bitstring<12>& row2()
    {
      assert_choice_type(types::row2, type_, "frequencyDomainAllocation");
      return c.get<fixed_bitstring<12>>();
    }
    fixed_bitstring<3>& row4()
    {
      assert_choice_type(types::row4, type_, "frequencyDomainAllocation");
      return c.get<fixed_bitstring<3>>();
    }
    fixed_bitstring<6>& other()
    {
      assert_choice_type(types::other, type_, "frequencyDomainAllocation");
      return c.get<fixed_bitstring<6>>();
    }
    const fixed_bitstring<4>& row1() const
    {
      assert_choice_type(types::row1, type_, "frequencyDomainAllocation");
      return c.get<fixed_bitstring<4>>();
    }
    const fixed_bitstring<12>& row2() const
    {
      assert_choice_type(types::row2, type_, "frequencyDomainAllocation");
      return c.get<fixed_bitstring<12>>();
    }
    const fixed_bitstring<3>& row4() const
    {
      assert_choice_type(types::row4, type_, "frequencyDomainAllocation");
      return c.get<fixed_bitstring<3>>();
    }
    const fixed_bitstring<6>& other() const
    {
      assert_choice_type(types::other, type_, "frequencyDomainAllocation");
      return c.get<fixed_bitstring<6>>();
    }
    fixed_bitstring<4>&  set_row1();
    fixed_bitstring<12>& set_row2();
    fixed_bitstring<3>&  set_row4();
    fixed_bitstring<6>&  set_other();

  private:
    types                                type_;
    choice_buffer_t<fixed_bitstring<12>> c;

    void destroy_();
  };
  struct nrof_ports_opts {
    enum options { p1, p2, p4, p8, p12, p16, p24, p32, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using nrof_ports_e_ = enumerated<nrof_ports_opts>;
  struct cdm_type_opts {
    enum options { no_cdm, fd_cdm2, cdm4_fd2_td2, cdm8_fd2_td4, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using cdm_type_e_ = enumerated<cdm_type_opts>;
  struct density_c_ {
    struct dot5_opts {
      enum options { even_prbs, odd_prbs, nulltype } value;

      const char* to_string() const;
    };
    using dot5_e_ = enumerated<dot5_opts>;
    struct types_opts {
      enum options { dot5, one, three, spare, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    density_c_() = default;
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    dot5_e_& dot5()
    {
      assert_choice_type(types::dot5, type_, "density");
      return c;
    }
    const dot5_e_& dot5() const
    {
      assert_choice_type(types::dot5, type_, "density");
      return c;
    }
    dot5_e_& set_dot5();
    void     set_one();
    void     set_three();
    void     set_spare();

  private:
    types   type_;
    dot5_e_ c;
  };

  // member variables
  bool                  ext                                       = false;
  bool                  first_ofdm_symbol_in_time_domain2_present = false;
  freq_domain_alloc_c_  freq_domain_alloc;
  nrof_ports_e_         nrof_ports;
  uint8_t               first_ofdm_symbol_in_time_domain  = 0;
  uint8_t               first_ofdm_symbol_in_time_domain2 = 2;
  cdm_type_e_           cdm_type;
  density_c_            density;
  csi_freq_occupation_s freq_band;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DownlinkPreemption ::= SEQUENCE
struct dl_preemption_s {
  struct time_freq_set_opts {
    enum options { set0, set1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using time_freq_set_e_            = enumerated<time_freq_set_opts>;
  using int_cfg_per_serving_cell_l_ = dyn_array<int_cfg_per_serving_cell_s>;

  // member variables
  bool                        ext      = false;
  uint32_t                    int_rnti = 0;
  time_freq_set_e_            time_freq_set;
  uint8_t                     dci_payload_size = 0;
  int_cfg_per_serving_cell_l_ int_cfg_per_serving_cell;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DMRS-BundlingPUSCH-Config-r17 ::= SEQUENCE
struct dmrs_bundling_pusch_cfg_r17_s {
  struct pusch_freq_hop_interv_r17_opts {
    enum options { s2, s4, s5, s6, s8, s10, s12, s14, s16, s20, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pusch_freq_hop_interv_r17_e_ = enumerated<pusch_freq_hop_interv_r17_opts>;

  // member variables
  bool                         ext                                   = false;
  bool                         pusch_dmrs_bundling_r17_present       = false;
  bool                         pusch_time_domain_win_len_r17_present = false;
  bool                         pusch_win_restart_r17_present         = false;
  bool                         pusch_freq_hop_interv_r17_present     = false;
  uint8_t                      pusch_time_domain_win_len_r17         = 2;
  pusch_freq_hop_interv_r17_e_ pusch_freq_hop_interv_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DMRS-DownlinkConfig ::= SEQUENCE
struct dmrs_dl_cfg_s {
  struct dmrs_add_position_opts {
    enum options { pos0, pos1, pos3, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using dmrs_add_position_e_ = enumerated<dmrs_add_position_opts>;

  // member variables
  bool                           ext                       = false;
  bool                           dmrs_type_present         = false;
  bool                           dmrs_add_position_present = false;
  bool                           max_len_present           = false;
  bool                           scrambling_id0_present    = false;
  bool                           scrambling_id1_present    = false;
  bool                           phase_tracking_rs_present = false;
  dmrs_add_position_e_           dmrs_add_position;
  uint32_t                       scrambling_id0 = 0;
  uint32_t                       scrambling_id1 = 0;
  setup_release_c<ptrs_dl_cfg_s> phase_tracking_rs;
  // ...
  // group 0
  bool dmrs_dl_r16_present = false;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DMRS-UplinkConfig ::= SEQUENCE
struct dmrs_ul_cfg_s {
  struct dmrs_add_position_opts {
    enum options { pos0, pos1, pos3, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using dmrs_add_position_e_ = enumerated<dmrs_add_position_opts>;
  struct transform_precoding_disabled_s_ {
    bool     ext                    = false;
    bool     scrambling_id0_present = false;
    bool     scrambling_id1_present = false;
    uint32_t scrambling_id0         = 0;
    uint32_t scrambling_id1         = 0;
    // ...
    // group 0
    bool dmrs_ul_r16_present = false;

    // sequence methods
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
  };
  struct transform_precoding_enabled_s_ {
    bool     ext                   = false;
    bool     npusch_id_present     = false;
    bool     seq_group_hop_present = false;
    bool     seq_hop_present       = false;
    uint16_t npusch_id             = 0;
    // ...
    // group 0
    copy_ptr<setup_release_c<dmrs_ul_transform_precoding_r16_s>> dmrs_ul_transform_precoding_r16;

    // sequence methods
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
  };

  // member variables
  bool                            ext                                  = false;
  bool                            dmrs_type_present                    = false;
  bool                            dmrs_add_position_present            = false;
  bool                            phase_tracking_rs_present            = false;
  bool                            max_len_present                      = false;
  bool                            transform_precoding_disabled_present = false;
  bool                            transform_precoding_enabled_present  = false;
  dmrs_add_position_e_            dmrs_add_position;
  setup_release_c<ptrs_ul_cfg_s>  phase_tracking_rs;
  transform_precoding_disabled_s_ transform_precoding_disabled;
  transform_precoding_enabled_s_  transform_precoding_enabled;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// Dummy-TDRA-List ::= SEQUENCE (SIZE (1..16)) OF MultiPDSCH-TDRA-r17
using dummy_tdra_list_l = dyn_array<multi_pdsch_tdra_r17_s>;

// FrequencyHoppingOffsetListsDCI-0-2-r16 ::= SEQUENCE (SIZE (1..4)) OF INTEGER (1..274)
using freq_hop_offset_lists_dci_0_2_r16_l = bounded_array<uint16_t, 4>;

// MinSchedulingOffsetK0-Values-r16 ::= SEQUENCE (SIZE (1..2)) OF INTEGER (0..16)
using min_sched_offset_k0_values_r16_l = bounded_array<uint8_t, 2>;

// MinSchedulingOffsetK0-Values-r17 ::= SEQUENCE (SIZE (1..2)) OF INTEGER (0..64)
using min_sched_offset_k0_values_r17_l = bounded_array<uint8_t, 2>;

// MinSchedulingOffsetK2-Values-r16 ::= SEQUENCE (SIZE (1..2)) OF INTEGER (0..16)
using min_sched_offset_k2_values_r16_l = bounded_array<uint8_t, 2>;

// MinSchedulingOffsetK2-Values-r17 ::= SEQUENCE (SIZE (1..2)) OF INTEGER (0..64)
using min_sched_offset_k2_values_r17_l = bounded_array<uint8_t, 2>;

// MsgA-ConfigCommon-r16 ::= SEQUENCE
struct msg_a_cfg_common_r16_s {
  bool                              msg_a_pusch_cfg_r16_present = false;
  rach_cfg_common_two_step_ra_r16_s rach_cfg_common_two_step_ra_r16;
  msg_a_pusch_cfg_r16_s             msg_a_pusch_cfg_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// MultiPDSCH-TDRA-List-r17 ::= SEQUENCE (SIZE (1..64)) OF MultiPDSCH-TDRA-r17
using multi_pdsch_tdra_list_r17_l = dyn_array<multi_pdsch_tdra_r17_s>;

// P0-PUSCH-AlphaSet ::= SEQUENCE
struct p0_pusch_alpha_set_s {
  bool    p0_present            = false;
  bool    alpha_present         = false;
  uint8_t p0_pusch_alpha_set_id = 0;
  int8_t  p0                    = -16;
  alpha_e alpha;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PDSCH-TimeDomainResourceAllocationList ::= SEQUENCE (SIZE (1..16)) OF PDSCH-TimeDomainResourceAllocation
using pdsch_time_domain_res_alloc_list_l = dyn_array<pdsch_time_domain_res_alloc_s>;

// PDSCH-TimeDomainResourceAllocationList-r16 ::= SEQUENCE (SIZE (1..16)) OF PDSCH-TimeDomainResourceAllocation-r16
using pdsch_time_domain_res_alloc_list_r16_l = dyn_array<pdsch_time_domain_res_alloc_r16_s>;

// PUCCH-PowerControl ::= SEQUENCE
struct pucch_pwr_ctrl_s {
  using p0_set_l_           = dyn_array<p0_pucch_s>;
  using pathloss_ref_rss_l_ = dyn_array<pucch_pathloss_ref_rs_s>;

  // member variables
  bool                ext                                    = false;
  bool                delta_f_pucch_f0_present               = false;
  bool                delta_f_pucch_f1_present               = false;
  bool                delta_f_pucch_f2_present               = false;
  bool                delta_f_pucch_f3_present               = false;
  bool                delta_f_pucch_f4_present               = false;
  bool                two_pucch_pc_adjustment_states_present = false;
  int8_t              delta_f_pucch_f0                       = -16;
  int8_t              delta_f_pucch_f1                       = -16;
  int8_t              delta_f_pucch_f2                       = -16;
  int8_t              delta_f_pucch_f3                       = -16;
  int8_t              delta_f_pucch_f4                       = -16;
  p0_set_l_           p0_set;
  pathloss_ref_rss_l_ pathloss_ref_rss;
  // ...
  // group 0
  copy_ptr<setup_release_c<dyn_seq_of<pucch_pathloss_ref_rs_r16_s, 1, 60>>> pathloss_ref_rss_v1610;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-PowerControlSetInfo-r17 ::= SEQUENCE
struct pucch_pwr_ctrl_set_info_r17_s {
  struct pucch_closed_loop_idx_r17_opts {
    enum options { i0, i1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pucch_closed_loop_idx_r17_e_ = enumerated<pucch_closed_loop_idx_r17_opts>;

  // member variables
  uint8_t                      pucch_pwr_ctrl_set_info_id_r17 = 1;
  uint8_t                      p0_pucch_id_r17                = 1;
  pucch_closed_loop_idx_r17_e_ pucch_closed_loop_idx_r17;
  uint8_t                      pucch_pathloss_ref_rs_id_r17 = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-ResourceExt-v1610 ::= SEQUENCE
struct pucch_res_ext_v1610_s {
  struct interlace_alloc_r16_s_ {
    struct interlace0_r16_c_ {
      struct types_opts {
        enum options { scs15, scs30, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using types = enumerated<types_opts>;

      // choice methods
      interlace0_r16_c_() = default;
      interlace0_r16_c_(const interlace0_r16_c_& other);
      interlace0_r16_c_& operator=(const interlace0_r16_c_& other);
      ~interlace0_r16_c_() { destroy_(); }
      void        set(types::options e = types::nulltype);
      types       type() const { return type_; }
      SRSASN_CODE pack(bit_ref& bref) const;
      SRSASN_CODE unpack(cbit_ref& bref);
      void        to_json(json_writer& j) const;
      // getters
      uint8_t& scs15()
      {
        assert_choice_type(types::scs15, type_, "interlace0-r16");
        return c.get<uint8_t>();
      }
      uint8_t& scs30()
      {
        assert_choice_type(types::scs30, type_, "interlace0-r16");
        return c.get<uint8_t>();
      }
      const uint8_t& scs15() const
      {
        assert_choice_type(types::scs15, type_, "interlace0-r16");
        return c.get<uint8_t>();
      }
      const uint8_t& scs30() const
      {
        assert_choice_type(types::scs30, type_, "interlace0-r16");
        return c.get<uint8_t>();
      }
      uint8_t& set_scs15();
      uint8_t& set_scs30();

    private:
      types               type_;
      pod_choice_buffer_t c;

      void destroy_();
    };

    // member variables
    uint8_t           rb_set_idx_r16 = 0;
    interlace0_r16_c_ interlace0_r16;
  };
  struct format_v1610_c_ {
    struct occ_v1610_s_ {
      struct occ_len_v1610_opts {
        enum options { n2, n4, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using occ_len_v1610_e_ = enumerated<occ_len_v1610_opts>;
      struct occ_idx_v1610_opts {
        enum options { n0, n1, n2, n3, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using occ_idx_v1610_e_ = enumerated<occ_idx_v1610_opts>;

      // member variables
      bool             occ_len_v1610_present = false;
      bool             occ_idx_v1610_present = false;
      occ_len_v1610_e_ occ_len_v1610;
      occ_idx_v1610_e_ occ_idx_v1610;
    };
    struct types_opts {
      enum options { interlace1_v1610, occ_v1610, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    format_v1610_c_() = default;
    format_v1610_c_(const format_v1610_c_& other);
    format_v1610_c_& operator=(const format_v1610_c_& other);
    ~format_v1610_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& interlace1_v1610()
    {
      assert_choice_type(types::interlace1_v1610, type_, "format-v1610");
      return c.get<uint8_t>();
    }
    occ_v1610_s_& occ_v1610()
    {
      assert_choice_type(types::occ_v1610, type_, "format-v1610");
      return c.get<occ_v1610_s_>();
    }
    const uint8_t& interlace1_v1610() const
    {
      assert_choice_type(types::interlace1_v1610, type_, "format-v1610");
      return c.get<uint8_t>();
    }
    const occ_v1610_s_& occ_v1610() const
    {
      assert_choice_type(types::occ_v1610, type_, "format-v1610");
      return c.get<occ_v1610_s_>();
    }
    uint8_t&      set_interlace1_v1610();
    occ_v1610_s_& set_occ_v1610();

  private:
    types                         type_;
    choice_buffer_t<occ_v1610_s_> c;

    void destroy_();
  };
  struct format_v1700_s_ {
    uint8_t nrof_prbs_r17 = 1;
  };
  struct pucch_repeat_nrof_slots_r17_opts {
    enum options { n1, n2, n4, n8, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pucch_repeat_nrof_slots_r17_e_ = enumerated<pucch_repeat_nrof_slots_r17_opts>;

  // member variables
  bool                   ext                         = false;
  bool                   interlace_alloc_r16_present = false;
  bool                   format_v1610_present        = false;
  interlace_alloc_r16_s_ interlace_alloc_r16;
  format_v1610_c_        format_v1610;
  // ...
  // group 0
  bool                           pucch_repeat_nrof_slots_r17_present = false;
  copy_ptr<format_v1700_s_>      format_v1700;
  pucch_repeat_nrof_slots_r17_e_ pucch_repeat_nrof_slots_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-ResourceGroup-r16 ::= SEQUENCE
struct pucch_res_group_r16_s {
  using res_per_group_list_r16_l_ = dyn_array<uint8_t>;

  // member variables
  uint8_t                   pucch_res_group_id_r16 = 0;
  res_per_group_list_r16_l_ res_per_group_list_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-Resource ::= SEQUENCE
struct pucch_res_s {
  struct format_c_ {
    struct types_opts {
      enum options { format0, format1, format2, format3, format4, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    format_c_() = default;
    format_c_(const format_c_& other);
    format_c_& operator=(const format_c_& other);
    ~format_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    pucch_format0_s& format0()
    {
      assert_choice_type(types::format0, type_, "format");
      return c.get<pucch_format0_s>();
    }
    pucch_format1_s& format1()
    {
      assert_choice_type(types::format1, type_, "format");
      return c.get<pucch_format1_s>();
    }
    pucch_format2_s& format2()
    {
      assert_choice_type(types::format2, type_, "format");
      return c.get<pucch_format2_s>();
    }
    pucch_format3_s& format3()
    {
      assert_choice_type(types::format3, type_, "format");
      return c.get<pucch_format3_s>();
    }
    pucch_format4_s& format4()
    {
      assert_choice_type(types::format4, type_, "format");
      return c.get<pucch_format4_s>();
    }
    const pucch_format0_s& format0() const
    {
      assert_choice_type(types::format0, type_, "format");
      return c.get<pucch_format0_s>();
    }
    const pucch_format1_s& format1() const
    {
      assert_choice_type(types::format1, type_, "format");
      return c.get<pucch_format1_s>();
    }
    const pucch_format2_s& format2() const
    {
      assert_choice_type(types::format2, type_, "format");
      return c.get<pucch_format2_s>();
    }
    const pucch_format3_s& format3() const
    {
      assert_choice_type(types::format3, type_, "format");
      return c.get<pucch_format3_s>();
    }
    const pucch_format4_s& format4() const
    {
      assert_choice_type(types::format4, type_, "format");
      return c.get<pucch_format4_s>();
    }
    pucch_format0_s& set_format0();
    pucch_format1_s& set_format1();
    pucch_format2_s& set_format2();
    pucch_format3_s& set_format3();
    pucch_format4_s& set_format4();

  private:
    types                                                                                                type_;
    choice_buffer_t<pucch_format0_s, pucch_format1_s, pucch_format2_s, pucch_format3_s, pucch_format4_s> c;

    void destroy_();
  };

  // member variables
  bool      intra_slot_freq_hop_present = false;
  bool      second_hop_prb_present      = false;
  uint8_t   pucch_res_id                = 0;
  uint16_t  start_prb                   = 0;
  uint16_t  second_hop_prb              = 0;
  format_c_ format;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-ResourceSet ::= SEQUENCE
struct pucch_res_set_s {
  using res_list_l_ = bounded_array<uint8_t, 32>;

  // member variables
  bool        max_payload_size_present = false;
  uint8_t     pucch_res_set_id         = 0;
  res_list_l_ res_list;
  uint16_t    max_payload_size = 4;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-SpatialRelationInfoExt-r16 ::= SEQUENCE
struct pucch_spatial_relation_info_ext_r16_s {
  bool    ext                                          = false;
  bool    pucch_spatial_relation_info_id_v1610_present = false;
  bool    pucch_pathloss_ref_rs_id_v1610_present       = false;
  uint8_t pucch_spatial_relation_info_id_v1610         = 9;
  uint8_t pucch_pathloss_ref_rs_id_v1610               = 4;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-SpatialRelationInfo ::= SEQUENCE
struct pucch_spatial_relation_info_s {
  struct ref_sig_c_ {
    struct types_opts {
      enum options { ssb_idx, csi_rs_idx, srs, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ref_sig_c_() = default;
    ref_sig_c_(const ref_sig_c_& other);
    ref_sig_c_& operator=(const ref_sig_c_& other);
    ~ref_sig_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx()
    {
      assert_choice_type(types::ssb_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx()
    {
      assert_choice_type(types::csi_rs_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    pucch_srs_s& srs()
    {
      assert_choice_type(types::srs, type_, "referenceSignal");
      return c.get<pucch_srs_s>();
    }
    const uint8_t& ssb_idx() const
    {
      assert_choice_type(types::ssb_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx() const
    {
      assert_choice_type(types::csi_rs_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    const pucch_srs_s& srs() const
    {
      assert_choice_type(types::srs, type_, "referenceSignal");
      return c.get<pucch_srs_s>();
    }
    uint8_t&     set_ssb_idx();
    uint8_t&     set_csi_rs_idx();
    pucch_srs_s& set_srs();

  private:
    types                        type_;
    choice_buffer_t<pucch_srs_s> c;

    void destroy_();
  };
  struct closed_loop_idx_opts {
    enum options { i0, i1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using closed_loop_idx_e_ = enumerated<closed_loop_idx_opts>;

  // member variables
  bool               serving_cell_id_present        = false;
  uint8_t            pucch_spatial_relation_info_id = 1;
  uint8_t            serving_cell_id                = 0;
  ref_sig_c_         ref_sig;
  uint8_t            pucch_pathloss_ref_rs_id = 0;
  uint8_t            p0_pucch_id              = 1;
  closed_loop_idx_e_ closed_loop_idx;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-TPC-CommandConfig ::= SEQUENCE
struct pucch_tpc_cmd_cfg_s {
  bool    ext                         = false;
  bool    tpc_idx_pcell_present       = false;
  bool    tpc_idx_pucch_scell_present = false;
  uint8_t tpc_idx_pcell               = 1;
  uint8_t tpc_idx_pucch_scell         = 1;
  // ...
  // group 0
  bool    tpc_idx_pucch_s_scell_r17_present                       = false;
  bool    tpc_idx_pucch_s_scell_secondary_pucch_group_r17_present = false;
  uint8_t tpc_idx_pucch_s_scell_r17                               = 1;
  uint8_t tpc_idx_pucch_s_scell_secondary_pucch_group_r17         = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUSCH-PathlossReferenceRS ::= SEQUENCE
struct pusch_pathloss_ref_rs_s {
  struct ref_sig_c_ {
    struct types_opts {
      enum options { ssb_idx, csi_rs_idx, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ref_sig_c_() = default;
    ref_sig_c_(const ref_sig_c_& other);
    ref_sig_c_& operator=(const ref_sig_c_& other);
    ~ref_sig_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx()
    {
      assert_choice_type(types::ssb_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx()
    {
      assert_choice_type(types::csi_rs_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_idx() const
    {
      assert_choice_type(types::ssb_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx() const
    {
      assert_choice_type(types::csi_rs_idx, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_idx();
    uint8_t& set_csi_rs_idx();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  uint8_t    pusch_pathloss_ref_rs_id = 0;
  ref_sig_c_ ref_sig;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUSCH-PowerControl-v1610 ::= SEQUENCE
struct pusch_pwr_ctrl_v1610_s {
  using pathloss_ref_rs_to_add_mod_list_size_ext_v1610_l_ = dyn_array<pusch_pathloss_ref_rs_r16_s>;
  using pathloss_ref_rs_to_release_list_size_ext_v1610_l_ = dyn_array<uint8_t>;
  using p0_pusch_set_list_r16_l_                          = dyn_array<p0_pusch_set_r16_s>;
  struct olpc_param_set_s_ {
    bool    olpc_param_set_dci_0_1_r16_present = false;
    bool    olpc_param_set_dci_0_2_r16_present = false;
    uint8_t olpc_param_set_dci_0_1_r16         = 1;
    uint8_t olpc_param_set_dci_0_2_r16         = 1;
  };
  using sri_pusch_map_to_add_mod_list2_r17_l_ = dyn_array<sri_pusch_pwr_ctrl_s>;
  using sri_pusch_map_to_release_list2_r17_l_ = bounded_array<uint8_t, 16>;
  using p0_pusch_set_list2_r17_l_             = dyn_array<p0_pusch_set_r16_s>;
  using dummy_l_                              = dyn_array<dummy_pathloss_ref_rs_v1710_s>;

  // member variables
  bool                                              ext                    = false;
  bool                                              olpc_param_set_present = false;
  pathloss_ref_rs_to_add_mod_list_size_ext_v1610_l_ pathloss_ref_rs_to_add_mod_list_size_ext_v1610;
  pathloss_ref_rs_to_release_list_size_ext_v1610_l_ pathloss_ref_rs_to_release_list_size_ext_v1610;
  p0_pusch_set_list_r16_l_                          p0_pusch_set_list_r16;
  olpc_param_set_s_                                 olpc_param_set;
  // ...
  // group 0
  copy_ptr<sri_pusch_map_to_add_mod_list2_r17_l_> sri_pusch_map_to_add_mod_list2_r17;
  copy_ptr<sri_pusch_map_to_release_list2_r17_l_> sri_pusch_map_to_release_list2_r17;
  copy_ptr<p0_pusch_set_list2_r17_l_>             p0_pusch_set_list2_r17;
  copy_ptr<dummy_l_>                              dummy;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUSCH-TimeDomainResourceAllocationList ::= SEQUENCE (SIZE (1..16)) OF PUSCH-TimeDomainResourceAllocation
using pusch_time_domain_res_alloc_list_l = dyn_array<pusch_time_domain_res_alloc_s>;

// PUSCH-TimeDomainResourceAllocationList-r16 ::= SEQUENCE (SIZE (1..64)) OF PUSCH-TimeDomainResourceAllocation-r16
using pusch_time_domain_res_alloc_list_r16_l = dyn_array<pusch_time_domain_res_alloc_r16_s>;

// PUSCH-TPC-CommandConfig ::= SEQUENCE
struct pusch_tpc_cmd_cfg_s {
  bool    ext                 = false;
  bool    tpc_idx_present     = false;
  bool    tpc_idx_sul_present = false;
  bool    target_cell_present = false;
  uint8_t tpc_idx             = 1;
  uint8_t tpc_idx_sul         = 1;
  uint8_t target_cell         = 0;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// QCL-Info ::= SEQUENCE
struct qcl_info_s {
  struct ref_sig_c_ {
    struct types_opts {
      enum options { csi_rs, ssb, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ref_sig_c_() = default;
    ref_sig_c_(const ref_sig_c_& other);
    ref_sig_c_& operator=(const ref_sig_c_& other);
    ~ref_sig_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& csi_rs()
    {
      assert_choice_type(types::csi_rs, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    uint8_t& ssb()
    {
      assert_choice_type(types::ssb, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs() const
    {
      assert_choice_type(types::csi_rs, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb() const
    {
      assert_choice_type(types::ssb, type_, "referenceSignal");
      return c.get<uint8_t>();
    }
    uint8_t& set_csi_rs();
    uint8_t& set_ssb();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct qcl_type_opts {
    enum options { type_a, type_b, type_c, type_d, nulltype } value;

    const char* to_string() const;
  };
  using qcl_type_e_ = enumerated<qcl_type_opts>;

  // member variables
  bool        ext            = false;
  bool        cell_present   = false;
  bool        bwp_id_present = false;
  uint8_t     cell           = 0;
  uint8_t     bwp_id         = 0;
  ref_sig_c_  ref_sig;
  qcl_type_e_ qcl_type;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RACH-ConfigCommon ::= SEQUENCE
struct rach_cfg_common_s {
  struct ssb_per_rach_occasion_and_cb_preambs_per_ssb_c_ {
    struct one_eighth_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, n36, n40, n44, n48, n52, n56, n60, n64, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using one_eighth_e_ = enumerated<one_eighth_opts>;
    struct one_fourth_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, n36, n40, n44, n48, n52, n56, n60, n64, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using one_fourth_e_ = enumerated<one_fourth_opts>;
    struct one_half_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, n36, n40, n44, n48, n52, n56, n60, n64, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using one_half_e_ = enumerated<one_half_opts>;
    struct one_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, n36, n40, n44, n48, n52, n56, n60, n64, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using one_e_ = enumerated<one_opts>;
    struct two_opts {
      enum options { n4, n8, n12, n16, n20, n24, n28, n32, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using two_e_ = enumerated<two_opts>;
    struct types_opts {
      enum options { one_eighth, one_fourth, one_half, one, two, four, eight, sixteen, nulltype } value;
      typedef float number_type;

      const char* to_string() const;
      float       to_number() const;
      const char* to_number_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ssb_per_rach_occasion_and_cb_preambs_per_ssb_c_() = default;
    ssb_per_rach_occasion_and_cb_preambs_per_ssb_c_(const ssb_per_rach_occasion_and_cb_preambs_per_ssb_c_& other);
    ssb_per_rach_occasion_and_cb_preambs_per_ssb_c_&
    operator=(const ssb_per_rach_occasion_and_cb_preambs_per_ssb_c_& other);
    ~ssb_per_rach_occasion_and_cb_preambs_per_ssb_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    one_eighth_e_& one_eighth()
    {
      assert_choice_type(types::one_eighth, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<one_eighth_e_>();
    }
    one_fourth_e_& one_fourth()
    {
      assert_choice_type(types::one_fourth, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<one_fourth_e_>();
    }
    one_half_e_& one_half()
    {
      assert_choice_type(types::one_half, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<one_half_e_>();
    }
    one_e_& one()
    {
      assert_choice_type(types::one, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<one_e_>();
    }
    two_e_& two()
    {
      assert_choice_type(types::two, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<two_e_>();
    }
    uint8_t& four()
    {
      assert_choice_type(types::four, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<uint8_t>();
    }
    uint8_t& eight()
    {
      assert_choice_type(types::eight, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<uint8_t>();
    }
    uint8_t& sixteen()
    {
      assert_choice_type(types::sixteen, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<uint8_t>();
    }
    const one_eighth_e_& one_eighth() const
    {
      assert_choice_type(types::one_eighth, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<one_eighth_e_>();
    }
    const one_fourth_e_& one_fourth() const
    {
      assert_choice_type(types::one_fourth, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<one_fourth_e_>();
    }
    const one_half_e_& one_half() const
    {
      assert_choice_type(types::one_half, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<one_half_e_>();
    }
    const one_e_& one() const
    {
      assert_choice_type(types::one, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<one_e_>();
    }
    const two_e_& two() const
    {
      assert_choice_type(types::two, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<two_e_>();
    }
    const uint8_t& four() const
    {
      assert_choice_type(types::four, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<uint8_t>();
    }
    const uint8_t& eight() const
    {
      assert_choice_type(types::eight, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<uint8_t>();
    }
    const uint8_t& sixteen() const
    {
      assert_choice_type(types::sixteen, type_, "ssb-perRACH-OccasionAndCB-PreamblesPerSSB");
      return c.get<uint8_t>();
    }
    one_eighth_e_& set_one_eighth();
    one_fourth_e_& set_one_fourth();
    one_half_e_&   set_one_half();
    one_e_&        set_one();
    two_e_&        set_two();
    uint8_t&       set_four();
    uint8_t&       set_eight();
    uint8_t&       set_sixteen();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct group_bcfg_s_ {
    struct ra_msg3_size_group_a_opts {
      enum options {
        b56,
        b144,
        b208,
        b256,
        b282,
        b480,
        b640,
        b800,
        b1000,
        b72,
        spare6,
        spare5,
        spare4,
        spare3,
        spare2,
        spare1,
        nulltype
      } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using ra_msg3_size_group_a_e_ = enumerated<ra_msg3_size_group_a_opts>;
    struct msg_pwr_offset_group_b_opts {
      enum options { minusinfinity, db0, db5, db8, db10, db12, db15, db18, nulltype } value;
      typedef int8_t number_type;

      const char* to_string() const;
      int8_t      to_number() const;
    };
    using msg_pwr_offset_group_b_e_ = enumerated<msg_pwr_offset_group_b_opts>;

    // member variables
    ra_msg3_size_group_a_e_   ra_msg3_size_group_a;
    msg_pwr_offset_group_b_e_ msg_pwr_offset_group_b;
    uint8_t                   nof_ra_preambs_group_a = 1;
  };
  struct ra_contention_resolution_timer_opts {
    enum options { sf8, sf16, sf24, sf32, sf40, sf48, sf56, sf64, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ra_contention_resolution_timer_e_ = enumerated<ra_contention_resolution_timer_opts>;
  struct prach_root_seq_idx_c_ {
    struct types_opts {
      enum options { l839, l139, nulltype } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    prach_root_seq_idx_c_() = default;
    prach_root_seq_idx_c_(const prach_root_seq_idx_c_& other);
    prach_root_seq_idx_c_& operator=(const prach_root_seq_idx_c_& other);
    ~prach_root_seq_idx_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint16_t& l839()
    {
      assert_choice_type(types::l839, type_, "prach-RootSequenceIndex");
      return c.get<uint16_t>();
    }
    uint8_t& l139()
    {
      assert_choice_type(types::l139, type_, "prach-RootSequenceIndex");
      return c.get<uint8_t>();
    }
    const uint16_t& l839() const
    {
      assert_choice_type(types::l839, type_, "prach-RootSequenceIndex");
      return c.get<uint16_t>();
    }
    const uint8_t& l139() const
    {
      assert_choice_type(types::l139, type_, "prach-RootSequenceIndex");
      return c.get<uint8_t>();
    }
    uint16_t& set_l839();
    uint8_t&  set_l139();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct restricted_set_cfg_opts {
    enum options { unrestricted_set, restricted_set_type_a, restricted_set_type_b, nulltype } value;

    const char* to_string() const;
  };
  using restricted_set_cfg_e_ = enumerated<restricted_set_cfg_opts>;
  struct ra_prioritization_for_access_id_r16_s_ {
    ra_prioritization_s ra_prioritization_r16;
    fixed_bitstring<2>  ra_prioritization_for_ai_r16;
  };
  struct prach_root_seq_idx_r16_c_ {
    struct types_opts {
      enum options { l571, l1151, nulltype } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    prach_root_seq_idx_r16_c_() = default;
    prach_root_seq_idx_r16_c_(const prach_root_seq_idx_r16_c_& other);
    prach_root_seq_idx_r16_c_& operator=(const prach_root_seq_idx_r16_c_& other);
    ~prach_root_seq_idx_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint16_t& l571()
    {
      assert_choice_type(types::l571, type_, "prach-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    uint16_t& l1151()
    {
      assert_choice_type(types::l1151, type_, "prach-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    const uint16_t& l571() const
    {
      assert_choice_type(types::l571, type_, "prach-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    const uint16_t& l1151() const
    {
      assert_choice_type(types::l1151, type_, "prach-RootSequenceIndex-r16");
      return c.get<uint16_t>();
    }
    uint16_t& set_l571();
    uint16_t& set_l1151();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  using feature_combination_preambs_list_r17_l_ = dyn_array<feature_combination_preambs_r17_s>;

  // member variables
  bool                                            ext                                                  = false;
  bool                                            total_nof_ra_preambs_present                         = false;
  bool                                            ssb_per_rach_occasion_and_cb_preambs_per_ssb_present = false;
  bool                                            group_bcfg_present                                   = false;
  bool                                            rsrp_thres_ssb_present                               = false;
  bool                                            rsrp_thres_ssb_sul_present                           = false;
  bool                                            msg1_subcarrier_spacing_present                      = false;
  bool                                            msg3_transform_precoder_present                      = false;
  rach_cfg_generic_s                              rach_cfg_generic;
  uint8_t                                         total_nof_ra_preambs = 1;
  ssb_per_rach_occasion_and_cb_preambs_per_ssb_c_ ssb_per_rach_occasion_and_cb_preambs_per_ssb;
  group_bcfg_s_                                   group_bcfg;
  ra_contention_resolution_timer_e_               ra_contention_resolution_timer;
  uint8_t                                         rsrp_thres_ssb     = 0;
  uint8_t                                         rsrp_thres_ssb_sul = 0;
  prach_root_seq_idx_c_                           prach_root_seq_idx;
  subcarrier_spacing_e                            msg1_subcarrier_spacing;
  restricted_set_cfg_e_                           restricted_set_cfg;
  // ...
  // group 0
  copy_ptr<ra_prioritization_for_access_id_r16_s_> ra_prioritization_for_access_id_r16;
  copy_ptr<prach_root_seq_idx_r16_c_>              prach_root_seq_idx_r16;
  // group 1
  copy_ptr<ra_prioritization_for_slicing_r17_s>     ra_prioritization_for_slicing_r17;
  copy_ptr<feature_combination_preambs_list_r17_l_> feature_combination_preambs_list_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

struct rate_match_pattern_group_item_c_ {
  struct types_opts {
    enum options { cell_level, bwp_level, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  rate_match_pattern_group_item_c_() = default;
  rate_match_pattern_group_item_c_(const rate_match_pattern_group_item_c_& other);
  rate_match_pattern_group_item_c_& operator=(const rate_match_pattern_group_item_c_& other);
  ~rate_match_pattern_group_item_c_() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  uint8_t& cell_level()
  {
    assert_choice_type(types::cell_level, type_, "RateMatchPatternGroup-item");
    return c.get<uint8_t>();
  }
  uint8_t& bwp_level()
  {
    assert_choice_type(types::bwp_level, type_, "RateMatchPatternGroup-item");
    return c.get<uint8_t>();
  }
  const uint8_t& cell_level() const
  {
    assert_choice_type(types::cell_level, type_, "RateMatchPatternGroup-item");
    return c.get<uint8_t>();
  }
  const uint8_t& bwp_level() const
  {
    assert_choice_type(types::bwp_level, type_, "RateMatchPatternGroup-item");
    return c.get<uint8_t>();
  }
  uint8_t& set_cell_level();
  uint8_t& set_bwp_level();

private:
  types               type_;
  pod_choice_buffer_t c;

  void destroy_();
};

// RepetitionSchemeConfig-r16 ::= CHOICE
struct repeat_scheme_cfg_r16_c {
  struct types_opts {
    enum options { fdm_tdm_r16, slot_based_r16, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  repeat_scheme_cfg_r16_c() = default;
  repeat_scheme_cfg_r16_c(const repeat_scheme_cfg_r16_c& other);
  repeat_scheme_cfg_r16_c& operator=(const repeat_scheme_cfg_r16_c& other);
  ~repeat_scheme_cfg_r16_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  setup_release_c<fdm_tdm_r16_s>& fdm_tdm_r16()
  {
    assert_choice_type(types::fdm_tdm_r16, type_, "RepetitionSchemeConfig-r16");
    return c.get<setup_release_c<fdm_tdm_r16_s>>();
  }
  setup_release_c<slot_based_r16_s>& slot_based_r16()
  {
    assert_choice_type(types::slot_based_r16, type_, "RepetitionSchemeConfig-r16");
    return c.get<setup_release_c<slot_based_r16_s>>();
  }
  const setup_release_c<fdm_tdm_r16_s>& fdm_tdm_r16() const
  {
    assert_choice_type(types::fdm_tdm_r16, type_, "RepetitionSchemeConfig-r16");
    return c.get<setup_release_c<fdm_tdm_r16_s>>();
  }
  const setup_release_c<slot_based_r16_s>& slot_based_r16() const
  {
    assert_choice_type(types::slot_based_r16, type_, "RepetitionSchemeConfig-r16");
    return c.get<setup_release_c<slot_based_r16_s>>();
  }
  setup_release_c<fdm_tdm_r16_s>&    set_fdm_tdm_r16();
  setup_release_c<slot_based_r16_s>& set_slot_based_r16();

private:
  types                                                                              type_;
  choice_buffer_t<setup_release_c<fdm_tdm_r16_s>, setup_release_c<slot_based_r16_s>> c;

  void destroy_();
};

// RepetitionSchemeConfig-v1630 ::= SEQUENCE
struct repeat_scheme_cfg_v1630_s {
  setup_release_c<slot_based_v1630_s> slot_based_v1630;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SchedulingRequestResourceConfigExt-v1610 ::= SEQUENCE
struct sched_request_res_cfg_ext_v1610_s {
  struct phy_prio_idx_r16_opts {
    enum options { p0, p1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using phy_prio_idx_r16_e_ = enumerated<phy_prio_idx_r16_opts>;

  // member variables
  bool                ext                      = false;
  bool                phy_prio_idx_r16_present = false;
  phy_prio_idx_r16_e_ phy_prio_idx_r16;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SchedulingRequestResourceConfigExt-v1700 ::= SEQUENCE
struct sched_request_res_cfg_ext_v1700_s {
  struct periodicity_and_offset_r17_c_ {
    struct types_opts {
      enum options { sl1280, sl2560, sl5120, nulltype } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    periodicity_and_offset_r17_c_() = default;
    periodicity_and_offset_r17_c_(const periodicity_and_offset_r17_c_& other);
    periodicity_and_offset_r17_c_& operator=(const periodicity_and_offset_r17_c_& other);
    ~periodicity_and_offset_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint16_t& sl1280()
    {
      assert_choice_type(types::sl1280, type_, "periodicityAndOffset-r17");
      return c.get<uint16_t>();
    }
    uint16_t& sl2560()
    {
      assert_choice_type(types::sl2560, type_, "periodicityAndOffset-r17");
      return c.get<uint16_t>();
    }
    uint16_t& sl5120()
    {
      assert_choice_type(types::sl5120, type_, "periodicityAndOffset-r17");
      return c.get<uint16_t>();
    }
    const uint16_t& sl1280() const
    {
      assert_choice_type(types::sl1280, type_, "periodicityAndOffset-r17");
      return c.get<uint16_t>();
    }
    const uint16_t& sl2560() const
    {
      assert_choice_type(types::sl2560, type_, "periodicityAndOffset-r17");
      return c.get<uint16_t>();
    }
    const uint16_t& sl5120() const
    {
      assert_choice_type(types::sl5120, type_, "periodicityAndOffset-r17");
      return c.get<uint16_t>();
    }
    uint16_t& set_sl1280();
    uint16_t& set_sl2560();
    uint16_t& set_sl5120();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  bool                          periodicity_and_offset_r17_present = false;
  periodicity_and_offset_r17_c_ periodicity_and_offset_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SchedulingRequestResourceConfig ::= SEQUENCE
struct sched_request_res_cfg_s {
  struct periodicity_and_offset_c_ {
    struct types_opts {
      enum options {
        sym2,
        sym6or7,
        sl1,
        sl2,
        sl4,
        sl5,
        sl8,
        sl10,
        sl16,
        sl20,
        sl40,
        sl80,
        sl160,
        sl320,
        sl640,
        nulltype
      } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    periodicity_and_offset_c_() = default;
    periodicity_and_offset_c_(const periodicity_and_offset_c_& other);
    periodicity_and_offset_c_& operator=(const periodicity_and_offset_c_& other);
    ~periodicity_and_offset_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& sl2()
    {
      assert_choice_type(types::sl2, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl4()
    {
      assert_choice_type(types::sl4, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl5()
    {
      assert_choice_type(types::sl5, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl8()
    {
      assert_choice_type(types::sl8, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl10()
    {
      assert_choice_type(types::sl10, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl16()
    {
      assert_choice_type(types::sl16, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl20()
    {
      assert_choice_type(types::sl20, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl40()
    {
      assert_choice_type(types::sl40, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl80()
    {
      assert_choice_type(types::sl80, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl160()
    {
      assert_choice_type(types::sl160, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint16_t& sl320()
    {
      assert_choice_type(types::sl320, type_, "periodicityAndOffset");
      return c.get<uint16_t>();
    }
    uint16_t& sl640()
    {
      assert_choice_type(types::sl640, type_, "periodicityAndOffset");
      return c.get<uint16_t>();
    }
    const uint8_t& sl2() const
    {
      assert_choice_type(types::sl2, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl4() const
    {
      assert_choice_type(types::sl4, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl5() const
    {
      assert_choice_type(types::sl5, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl8() const
    {
      assert_choice_type(types::sl8, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl10() const
    {
      assert_choice_type(types::sl10, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl16() const
    {
      assert_choice_type(types::sl16, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl20() const
    {
      assert_choice_type(types::sl20, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl40() const
    {
      assert_choice_type(types::sl40, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl80() const
    {
      assert_choice_type(types::sl80, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl160() const
    {
      assert_choice_type(types::sl160, type_, "periodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint16_t& sl320() const
    {
      assert_choice_type(types::sl320, type_, "periodicityAndOffset");
      return c.get<uint16_t>();
    }
    const uint16_t& sl640() const
    {
      assert_choice_type(types::sl640, type_, "periodicityAndOffset");
      return c.get<uint16_t>();
    }
    void      set_sym2();
    void      set_sym6or7();
    void      set_sl1();
    uint8_t&  set_sl2();
    uint8_t&  set_sl4();
    uint8_t&  set_sl5();
    uint8_t&  set_sl8();
    uint8_t&  set_sl10();
    uint8_t&  set_sl16();
    uint8_t&  set_sl20();
    uint8_t&  set_sl40();
    uint8_t&  set_sl80();
    uint8_t&  set_sl160();
    uint16_t& set_sl320();
    uint16_t& set_sl640();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  bool                      periodicity_and_offset_present = false;
  bool                      res_present                    = false;
  uint8_t                   sched_request_res_id           = 1;
  uint8_t                   sched_request_id               = 0;
  periodicity_and_offset_c_ periodicity_and_offset;
  uint8_t                   res = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SRS-PeriodicityAndOffsetExt-r16 ::= CHOICE
struct srs_periodicity_and_offset_ext_r16_c {
  struct types_opts {
    enum options { sl128, sl256, sl512, sl20480, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  srs_periodicity_and_offset_ext_r16_c() = default;
  srs_periodicity_and_offset_ext_r16_c(const srs_periodicity_and_offset_ext_r16_c& other);
  srs_periodicity_and_offset_ext_r16_c& operator=(const srs_periodicity_and_offset_ext_r16_c& other);
  ~srs_periodicity_and_offset_ext_r16_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  uint8_t& sl128()
  {
    assert_choice_type(types::sl128, type_, "SRS-PeriodicityAndOffsetExt-r16");
    return c.get<uint8_t>();
  }
  uint16_t& sl256()
  {
    assert_choice_type(types::sl256, type_, "SRS-PeriodicityAndOffsetExt-r16");
    return c.get<uint16_t>();
  }
  uint16_t& sl512()
  {
    assert_choice_type(types::sl512, type_, "SRS-PeriodicityAndOffsetExt-r16");
    return c.get<uint16_t>();
  }
  uint16_t& sl20480()
  {
    assert_choice_type(types::sl20480, type_, "SRS-PeriodicityAndOffsetExt-r16");
    return c.get<uint16_t>();
  }
  const uint8_t& sl128() const
  {
    assert_choice_type(types::sl128, type_, "SRS-PeriodicityAndOffsetExt-r16");
    return c.get<uint8_t>();
  }
  const uint16_t& sl256() const
  {
    assert_choice_type(types::sl256, type_, "SRS-PeriodicityAndOffsetExt-r16");
    return c.get<uint16_t>();
  }
  const uint16_t& sl512() const
  {
    assert_choice_type(types::sl512, type_, "SRS-PeriodicityAndOffsetExt-r16");
    return c.get<uint16_t>();
  }
  const uint16_t& sl20480() const
  {
    assert_choice_type(types::sl20480, type_, "SRS-PeriodicityAndOffsetExt-r16");
    return c.get<uint16_t>();
  }
  uint8_t&  set_sl128();
  uint16_t& set_sl256();
  uint16_t& set_sl512();
  uint16_t& set_sl20480();

private:
  types               type_;
  pod_choice_buffer_t c;

  void destroy_();
};

// SRS-PeriodicityAndOffset-r16 ::= CHOICE
struct srs_periodicity_and_offset_r16_c {
  struct types_opts {
    enum options {
      sl1,
      sl2,
      sl4,
      sl5,
      sl8,
      sl10,
      sl16,
      sl20,
      sl32,
      sl40,
      sl64,
      sl80,
      sl160,
      sl320,
      sl640,
      sl1280,
      sl2560,
      sl5120,
      sl10240,
      sl40960,
      sl81920,
      // ...
      nulltype
    } value;
    typedef uint32_t number_type;

    const char* to_string() const;
    uint32_t    to_number() const;
  };
  using types = enumerated<types_opts, true>;

  // choice methods
  srs_periodicity_and_offset_r16_c() = default;
  srs_periodicity_and_offset_r16_c(const srs_periodicity_and_offset_r16_c& other);
  srs_periodicity_and_offset_r16_c& operator=(const srs_periodicity_and_offset_r16_c& other);
  ~srs_periodicity_and_offset_r16_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  uint8_t& sl2()
  {
    assert_choice_type(types::sl2, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl4()
  {
    assert_choice_type(types::sl4, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl5()
  {
    assert_choice_type(types::sl5, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl8()
  {
    assert_choice_type(types::sl8, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl10()
  {
    assert_choice_type(types::sl10, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl16()
  {
    assert_choice_type(types::sl16, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl20()
  {
    assert_choice_type(types::sl20, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl32()
  {
    assert_choice_type(types::sl32, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl40()
  {
    assert_choice_type(types::sl40, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl64()
  {
    assert_choice_type(types::sl64, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl80()
  {
    assert_choice_type(types::sl80, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint8_t& sl160()
  {
    assert_choice_type(types::sl160, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  uint16_t& sl320()
  {
    assert_choice_type(types::sl320, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  uint16_t& sl640()
  {
    assert_choice_type(types::sl640, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  uint16_t& sl1280()
  {
    assert_choice_type(types::sl1280, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  uint16_t& sl2560()
  {
    assert_choice_type(types::sl2560, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  uint16_t& sl5120()
  {
    assert_choice_type(types::sl5120, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  uint16_t& sl10240()
  {
    assert_choice_type(types::sl10240, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  uint16_t& sl40960()
  {
    assert_choice_type(types::sl40960, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  uint32_t& sl81920()
  {
    assert_choice_type(types::sl81920, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint32_t>();
  }
  const uint8_t& sl2() const
  {
    assert_choice_type(types::sl2, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl4() const
  {
    assert_choice_type(types::sl4, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl5() const
  {
    assert_choice_type(types::sl5, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl8() const
  {
    assert_choice_type(types::sl8, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl10() const
  {
    assert_choice_type(types::sl10, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl16() const
  {
    assert_choice_type(types::sl16, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl20() const
  {
    assert_choice_type(types::sl20, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl32() const
  {
    assert_choice_type(types::sl32, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl40() const
  {
    assert_choice_type(types::sl40, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl64() const
  {
    assert_choice_type(types::sl64, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl80() const
  {
    assert_choice_type(types::sl80, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint8_t& sl160() const
  {
    assert_choice_type(types::sl160, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint8_t>();
  }
  const uint16_t& sl320() const
  {
    assert_choice_type(types::sl320, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  const uint16_t& sl640() const
  {
    assert_choice_type(types::sl640, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  const uint16_t& sl1280() const
  {
    assert_choice_type(types::sl1280, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  const uint16_t& sl2560() const
  {
    assert_choice_type(types::sl2560, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  const uint16_t& sl5120() const
  {
    assert_choice_type(types::sl5120, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  const uint16_t& sl10240() const
  {
    assert_choice_type(types::sl10240, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  const uint16_t& sl40960() const
  {
    assert_choice_type(types::sl40960, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint16_t>();
  }
  const uint32_t& sl81920() const
  {
    assert_choice_type(types::sl81920, type_, "SRS-PeriodicityAndOffset-r16");
    return c.get<uint32_t>();
  }
  void      set_sl1();
  uint8_t&  set_sl2();
  uint8_t&  set_sl4();
  uint8_t&  set_sl5();
  uint8_t&  set_sl8();
  uint8_t&  set_sl10();
  uint8_t&  set_sl16();
  uint8_t&  set_sl20();
  uint8_t&  set_sl32();
  uint8_t&  set_sl40();
  uint8_t&  set_sl64();
  uint8_t&  set_sl80();
  uint8_t&  set_sl160();
  uint16_t& set_sl320();
  uint16_t& set_sl640();
  uint16_t& set_sl1280();
  uint16_t& set_sl2560();
  uint16_t& set_sl5120();
  uint16_t& set_sl10240();
  uint16_t& set_sl40960();
  uint32_t& set_sl81920();

private:
  types               type_;
  pod_choice_buffer_t c;

  void destroy_();
};

// SRS-SpatialRelationInfoPos-r16 ::= CHOICE
struct srs_spatial_relation_info_pos_r16_c {
  struct serving_rs_r16_s_ {
    struct ref_sig_r16_c_ {
      struct srs_spatial_relation_r16_s_ {
        struct res_sel_r16_c_ {
          struct types_opts {
            enum options { srs_res_id_r16, srs_pos_res_id_r16, nulltype } value;

            const char* to_string() const;
          };
          using types = enumerated<types_opts>;

          // choice methods
          res_sel_r16_c_() = default;
          res_sel_r16_c_(const res_sel_r16_c_& other);
          res_sel_r16_c_& operator=(const res_sel_r16_c_& other);
          ~res_sel_r16_c_() { destroy_(); }
          void        set(types::options e = types::nulltype);
          types       type() const { return type_; }
          SRSASN_CODE pack(bit_ref& bref) const;
          SRSASN_CODE unpack(cbit_ref& bref);
          void        to_json(json_writer& j) const;
          // getters
          uint8_t& srs_res_id_r16()
          {
            assert_choice_type(types::srs_res_id_r16, type_, "resourceSelection-r16");
            return c.get<uint8_t>();
          }
          uint8_t& srs_pos_res_id_r16()
          {
            assert_choice_type(types::srs_pos_res_id_r16, type_, "resourceSelection-r16");
            return c.get<uint8_t>();
          }
          const uint8_t& srs_res_id_r16() const
          {
            assert_choice_type(types::srs_res_id_r16, type_, "resourceSelection-r16");
            return c.get<uint8_t>();
          }
          const uint8_t& srs_pos_res_id_r16() const
          {
            assert_choice_type(types::srs_pos_res_id_r16, type_, "resourceSelection-r16");
            return c.get<uint8_t>();
          }
          uint8_t& set_srs_res_id_r16();
          uint8_t& set_srs_pos_res_id_r16();

        private:
          types               type_;
          pod_choice_buffer_t c;

          void destroy_();
        };

        // member variables
        res_sel_r16_c_ res_sel_r16;
        uint8_t        ul_bwp_r16 = 0;
      };
      struct types_opts {
        enum options { ssb_idx_serving_r16, csi_rs_idx_serving_r16, srs_spatial_relation_r16, nulltype } value;

        const char* to_string() const;
      };
      using types = enumerated<types_opts>;

      // choice methods
      ref_sig_r16_c_() = default;
      ref_sig_r16_c_(const ref_sig_r16_c_& other);
      ref_sig_r16_c_& operator=(const ref_sig_r16_c_& other);
      ~ref_sig_r16_c_() { destroy_(); }
      void        set(types::options e = types::nulltype);
      types       type() const { return type_; }
      SRSASN_CODE pack(bit_ref& bref) const;
      SRSASN_CODE unpack(cbit_ref& bref);
      void        to_json(json_writer& j) const;
      // getters
      uint8_t& ssb_idx_serving_r16()
      {
        assert_choice_type(types::ssb_idx_serving_r16, type_, "referenceSignal-r16");
        return c.get<uint8_t>();
      }
      uint8_t& csi_rs_idx_serving_r16()
      {
        assert_choice_type(types::csi_rs_idx_serving_r16, type_, "referenceSignal-r16");
        return c.get<uint8_t>();
      }
      srs_spatial_relation_r16_s_& srs_spatial_relation_r16()
      {
        assert_choice_type(types::srs_spatial_relation_r16, type_, "referenceSignal-r16");
        return c.get<srs_spatial_relation_r16_s_>();
      }
      const uint8_t& ssb_idx_serving_r16() const
      {
        assert_choice_type(types::ssb_idx_serving_r16, type_, "referenceSignal-r16");
        return c.get<uint8_t>();
      }
      const uint8_t& csi_rs_idx_serving_r16() const
      {
        assert_choice_type(types::csi_rs_idx_serving_r16, type_, "referenceSignal-r16");
        return c.get<uint8_t>();
      }
      const srs_spatial_relation_r16_s_& srs_spatial_relation_r16() const
      {
        assert_choice_type(types::srs_spatial_relation_r16, type_, "referenceSignal-r16");
        return c.get<srs_spatial_relation_r16_s_>();
      }
      uint8_t&                     set_ssb_idx_serving_r16();
      uint8_t&                     set_csi_rs_idx_serving_r16();
      srs_spatial_relation_r16_s_& set_srs_spatial_relation_r16();

    private:
      types                                        type_;
      choice_buffer_t<srs_spatial_relation_r16_s_> c;

      void destroy_();
    };

    // member variables
    bool           serving_cell_id_present = false;
    uint8_t        serving_cell_id         = 0;
    ref_sig_r16_c_ ref_sig_r16;
  };
  struct types_opts {
    enum options { serving_rs_r16, ssb_ncell_r16, dl_prs_r16, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  srs_spatial_relation_info_pos_r16_c() = default;
  srs_spatial_relation_info_pos_r16_c(const srs_spatial_relation_info_pos_r16_c& other);
  srs_spatial_relation_info_pos_r16_c& operator=(const srs_spatial_relation_info_pos_r16_c& other);
  ~srs_spatial_relation_info_pos_r16_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  serving_rs_r16_s_& serving_rs_r16()
  {
    assert_choice_type(types::serving_rs_r16, type_, "SRS-SpatialRelationInfoPos-r16");
    return c.get<serving_rs_r16_s_>();
  }
  ssb_info_ncell_r16_s& ssb_ncell_r16()
  {
    assert_choice_type(types::ssb_ncell_r16, type_, "SRS-SpatialRelationInfoPos-r16");
    return c.get<ssb_info_ncell_r16_s>();
  }
  dl_prs_info_r16_s& dl_prs_r16()
  {
    assert_choice_type(types::dl_prs_r16, type_, "SRS-SpatialRelationInfoPos-r16");
    return c.get<dl_prs_info_r16_s>();
  }
  const serving_rs_r16_s_& serving_rs_r16() const
  {
    assert_choice_type(types::serving_rs_r16, type_, "SRS-SpatialRelationInfoPos-r16");
    return c.get<serving_rs_r16_s_>();
  }
  const ssb_info_ncell_r16_s& ssb_ncell_r16() const
  {
    assert_choice_type(types::ssb_ncell_r16, type_, "SRS-SpatialRelationInfoPos-r16");
    return c.get<ssb_info_ncell_r16_s>();
  }
  const dl_prs_info_r16_s& dl_prs_r16() const
  {
    assert_choice_type(types::dl_prs_r16, type_, "SRS-SpatialRelationInfoPos-r16");
    return c.get<dl_prs_info_r16_s>();
  }
  serving_rs_r16_s_&    set_serving_rs_r16();
  ssb_info_ncell_r16_s& set_ssb_ncell_r16();
  dl_prs_info_r16_s&    set_dl_prs_r16();

private:
  types                                                                       type_;
  choice_buffer_t<dl_prs_info_r16_s, serving_rs_r16_s_, ssb_info_ncell_r16_s> c;

  void destroy_();
};

// SRS-TPC-CommandConfig ::= SEQUENCE
struct srs_tpc_cmd_cfg_s {
  bool    ext                            = false;
  bool    start_bit_of_format2_3_present = false;
  bool    field_type_format2_3_present   = false;
  uint8_t start_bit_of_format2_3         = 1;
  uint8_t field_type_format2_3           = 0;
  // ...
  // group 0
  bool    start_bit_of_format2_3_sul_present = false;
  uint8_t start_bit_of_format2_3_sul         = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// UCI-OnPUSCH-ListDCI-0-1-r16 ::= SEQUENCE (SIZE (1..2)) OF UCI-OnPUSCH
using uci_on_pusch_list_dci_0_1_r16_l = dyn_array<uci_on_pusch_s>;

// UCI-OnPUSCH-ListDCI-0-2-r16 ::= SEQUENCE (SIZE (1..2)) OF UCI-OnPUSCH-DCI-0-2-r16
using uci_on_pusch_list_dci_0_2_r16_l = dyn_array<uci_on_pusch_dci_0_2_r16_s>;

// UL-AccessConfigListDCI-0-1-r16 ::= SEQUENCE (SIZE (1..64)) OF INTEGER (0..63)
using ul_access_cfg_list_dci_0_1_r16_l = dyn_array<uint8_t>;

// UL-AccessConfigListDCI-0-1-r17 ::= SEQUENCE (SIZE (1..3)) OF INTEGER (0..2)
using ul_access_cfg_list_dci_0_1_r17_l = bounded_array<uint8_t, 3>;

// UL-AccessConfigListDCI-0-2-r17 ::= SEQUENCE (SIZE (1..64)) OF INTEGER (0..63)
using ul_access_cfg_list_dci_0_2_r17_l = dyn_array<uint8_t>;

// UplinkCancellation-r16 ::= SEQUENCE
struct ul_cancellation_r16_s {
  using ci_cfg_per_serving_cell_r16_l_ = dyn_array<ci_cfg_per_serving_cell_r16_s>;

  // member variables
  bool                           ext                         = false;
  uint32_t                       ci_rnti_r16                 = 0;
  uint8_t                        dci_payload_size_for_ci_r16 = 0;
  ci_cfg_per_serving_cell_r16_l_ ci_cfg_per_serving_cell_r16;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// ZP-CSI-RS-ResourceSet ::= SEQUENCE
struct zp_csi_rs_res_set_s {
  using zp_csi_rs_res_id_list_l_ = bounded_array<uint8_t, 16>;

  // member variables
  bool                     ext                  = false;
  uint8_t                  zp_csi_rs_res_set_id = 0;
  zp_csi_rs_res_id_list_l_ zp_csi_rs_res_id_list;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// AdditionalRACH-Config-r17 ::= SEQUENCE
struct add_rach_cfg_r17_s {
  bool                   ext                          = false;
  bool                   rach_cfg_common_r17_present  = false;
  bool                   msg_a_cfg_common_r17_present = false;
  rach_cfg_common_s      rach_cfg_common_r17;
  msg_a_cfg_common_r16_s msg_a_cfg_common_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BeamFailureDetectionSet-r17 ::= SEQUENCE
struct beam_fail_detection_set_r17_s {
  using bfd_res_to_add_mod_list_r17_l_ = dyn_array<beam_link_monitoring_rs_r17_s>;
  using bfd_res_to_release_list_r17_l_ = dyn_array<uint8_t>;
  struct beam_fail_instance_max_count_r17_opts {
    enum options { n1, n2, n3, n4, n5, n6, n8, n10, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using beam_fail_instance_max_count_r17_e_ = enumerated<beam_fail_instance_max_count_r17_opts>;
  struct beam_fail_detection_timer_r17_opts {
    enum options { pbfd1, pbfd2, pbfd3, pbfd4, pbfd5, pbfd6, pbfd8, pbfd10, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using beam_fail_detection_timer_r17_e_ = enumerated<beam_fail_detection_timer_r17_opts>;

  // member variables
  bool                                ext                                      = false;
  bool                                beam_fail_instance_max_count_r17_present = false;
  bool                                beam_fail_detection_timer_r17_present    = false;
  bfd_res_to_add_mod_list_r17_l_      bfd_res_to_add_mod_list_r17;
  bfd_res_to_release_list_r17_l_      bfd_res_to_release_list_r17;
  beam_fail_instance_max_count_r17_e_ beam_fail_instance_max_count_r17;
  beam_fail_detection_timer_r17_e_    beam_fail_detection_timer_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// CG-COT-Sharing-r16 ::= CHOICE
struct cg_cot_sharing_r16_c {
  struct cot_sharing_r16_s_ {
    uint8_t dur_r16            = 1;
    uint8_t offset_r16         = 1;
    uint8_t ch_access_prio_r16 = 1;
  };
  struct types_opts {
    enum options { no_cot_sharing_r16, cot_sharing_r16, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  cg_cot_sharing_r16_c() = default;
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  cot_sharing_r16_s_& cot_sharing_r16()
  {
    assert_choice_type(types::cot_sharing_r16, type_, "CG-COT-Sharing-r16");
    return c;
  }
  const cot_sharing_r16_s_& cot_sharing_r16() const
  {
    assert_choice_type(types::cot_sharing_r16, type_, "CG-COT-Sharing-r16");
    return c;
  }
  void                set_no_cot_sharing_r16();
  cot_sharing_r16_s_& set_cot_sharing_r16();

private:
  types              type_;
  cot_sharing_r16_s_ c;
};

// CG-COT-Sharing-r17 ::= CHOICE
struct cg_cot_sharing_r17_c {
  struct cot_sharing_r17_s_ {
    uint16_t dur_r17    = 1;
    uint16_t offset_r17 = 1;
  };
  struct types_opts {
    enum options { no_cot_sharing_r17, cot_sharing_r17, nulltype } value;

    const char* to_string() const;
  };
  using types = enumerated<types_opts>;

  // choice methods
  cg_cot_sharing_r17_c() = default;
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  cot_sharing_r17_s_& cot_sharing_r17()
  {
    assert_choice_type(types::cot_sharing_r17, type_, "CG-COT-Sharing-r17");
    return c;
  }
  const cot_sharing_r17_s_& cot_sharing_r17() const
  {
    assert_choice_type(types::cot_sharing_r17, type_, "CG-COT-Sharing-r17");
    return c;
  }
  void                set_no_cot_sharing_r17();
  cot_sharing_r17_s_& set_cot_sharing_r17();

private:
  types              type_;
  cot_sharing_r17_s_ c;
};

// CG-SDT-Configuration-r17 ::= SEQUENCE
struct cg_sdt_cfg_r17_s {
  struct sdt_ssb_subset_r17_c_ {
    struct types_opts {
      enum options { short_bitmap_r17, medium_bitmap_r17, long_bitmap_r17, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    sdt_ssb_subset_r17_c_() = default;
    sdt_ssb_subset_r17_c_(const sdt_ssb_subset_r17_c_& other);
    sdt_ssb_subset_r17_c_& operator=(const sdt_ssb_subset_r17_c_& other);
    ~sdt_ssb_subset_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    fixed_bitstring<4>& short_bitmap_r17()
    {
      assert_choice_type(types::short_bitmap_r17, type_, "sdt-SSB-Subset-r17");
      return c.get<fixed_bitstring<4>>();
    }
    fixed_bitstring<8>& medium_bitmap_r17()
    {
      assert_choice_type(types::medium_bitmap_r17, type_, "sdt-SSB-Subset-r17");
      return c.get<fixed_bitstring<8>>();
    }
    fixed_bitstring<64>& long_bitmap_r17()
    {
      assert_choice_type(types::long_bitmap_r17, type_, "sdt-SSB-Subset-r17");
      return c.get<fixed_bitstring<64>>();
    }
    const fixed_bitstring<4>& short_bitmap_r17() const
    {
      assert_choice_type(types::short_bitmap_r17, type_, "sdt-SSB-Subset-r17");
      return c.get<fixed_bitstring<4>>();
    }
    const fixed_bitstring<8>& medium_bitmap_r17() const
    {
      assert_choice_type(types::medium_bitmap_r17, type_, "sdt-SSB-Subset-r17");
      return c.get<fixed_bitstring<8>>();
    }
    const fixed_bitstring<64>& long_bitmap_r17() const
    {
      assert_choice_type(types::long_bitmap_r17, type_, "sdt-SSB-Subset-r17");
      return c.get<fixed_bitstring<64>>();
    }
    fixed_bitstring<4>&  set_short_bitmap_r17();
    fixed_bitstring<8>&  set_medium_bitmap_r17();
    fixed_bitstring<64>& set_long_bitmap_r17();

  private:
    types                                type_;
    choice_buffer_t<fixed_bitstring<64>> c;

    void destroy_();
  };
  struct sdt_ssb_per_cg_pusch_r17_opts {
    enum options { one_eighth, one_fourth, half, one, two, four, eight, sixteen, nulltype } value;
    typedef float number_type;

    const char* to_string() const;
    float       to_number() const;
    const char* to_number_string() const;
  };
  using sdt_ssb_per_cg_pusch_r17_e_ = enumerated<sdt_ssb_per_cg_pusch_r17_opts>;
  struct sdt_alpha_r17_opts {
    enum options { alpha0, alpha04, alpha05, alpha06, alpha07, alpha08, alpha09, alpha1, nulltype } value;
    typedef float number_type;

    const char* to_string() const;
    float       to_number() const;
    const char* to_number_string() const;
  };
  using sdt_alpha_r17_e_ = enumerated<sdt_alpha_r17_opts>;
  struct sdt_dmrs_ports_r17_c_ {
    struct types_opts {
      enum options { dmrs_type1_r17, dmrs_type2_r17, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    sdt_dmrs_ports_r17_c_() = default;
    sdt_dmrs_ports_r17_c_(const sdt_dmrs_ports_r17_c_& other);
    sdt_dmrs_ports_r17_c_& operator=(const sdt_dmrs_ports_r17_c_& other);
    ~sdt_dmrs_ports_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    fixed_bitstring<8>& dmrs_type1_r17()
    {
      assert_choice_type(types::dmrs_type1_r17, type_, "sdt-DMRS-Ports-r17");
      return c.get<fixed_bitstring<8>>();
    }
    fixed_bitstring<12>& dmrs_type2_r17()
    {
      assert_choice_type(types::dmrs_type2_r17, type_, "sdt-DMRS-Ports-r17");
      return c.get<fixed_bitstring<12>>();
    }
    const fixed_bitstring<8>& dmrs_type1_r17() const
    {
      assert_choice_type(types::dmrs_type1_r17, type_, "sdt-DMRS-Ports-r17");
      return c.get<fixed_bitstring<8>>();
    }
    const fixed_bitstring<12>& dmrs_type2_r17() const
    {
      assert_choice_type(types::dmrs_type2_r17, type_, "sdt-DMRS-Ports-r17");
      return c.get<fixed_bitstring<12>>();
    }
    fixed_bitstring<8>&  set_dmrs_type1_r17();
    fixed_bitstring<12>& set_dmrs_type2_r17();

  private:
    types                                type_;
    choice_buffer_t<fixed_bitstring<12>> c;

    void destroy_();
  };

  // member variables
  bool                        cg_sdt_retx_timer_present        = false;
  bool                        sdt_ssb_subset_r17_present       = false;
  bool                        sdt_ssb_per_cg_pusch_r17_present = false;
  bool                        sdt_p0_pusch_r17_present         = false;
  bool                        sdt_alpha_r17_present            = false;
  bool                        sdt_dmrs_ports_r17_present       = false;
  bool                        sdt_nrof_dmrs_seqs_r17_present   = false;
  uint8_t                     cg_sdt_retx_timer                = 1;
  sdt_ssb_subset_r17_c_       sdt_ssb_subset_r17;
  sdt_ssb_per_cg_pusch_r17_e_ sdt_ssb_per_cg_pusch_r17;
  int8_t                      sdt_p0_pusch_r17 = -16;
  sdt_alpha_r17_e_            sdt_alpha_r17;
  sdt_dmrs_ports_r17_c_       sdt_dmrs_ports_r17;
  uint8_t                     sdt_nrof_dmrs_seqs_r17 = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// CG-StartingOffsets-r16 ::= SEQUENCE
struct cg_start_offsets_r16_s {
  using cg_start_full_bw_inside_cot_r16_l_  = bounded_array<uint8_t, 7>;
  using cg_start_full_bw_outside_cot_r16_l_ = bounded_array<uint8_t, 7>;

  // member variables
  bool                                cg_start_partial_bw_inside_cot_r16_present  = false;
  bool                                cg_start_partial_bw_outside_cot_r16_present = false;
  cg_start_full_bw_inside_cot_r16_l_  cg_start_full_bw_inside_cot_r16;
  cg_start_full_bw_outside_cot_r16_l_ cg_start_full_bw_outside_cot_r16;
  uint8_t                             cg_start_partial_bw_inside_cot_r16  = 0;
  uint8_t                             cg_start_partial_bw_outside_cot_r16 = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// ControlResourceSet ::= SEQUENCE
struct coreset_s {
  struct cce_reg_map_type_c_ {
    struct interleaved_s_ {
      struct reg_bundle_size_opts {
        enum options { n2, n3, n6, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using reg_bundle_size_e_ = enumerated<reg_bundle_size_opts>;
      struct interleaver_size_opts {
        enum options { n2, n3, n6, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using interleaver_size_e_ = enumerated<interleaver_size_opts>;

      // member variables
      bool                shift_idx_present = false;
      reg_bundle_size_e_  reg_bundle_size;
      interleaver_size_e_ interleaver_size;
      uint16_t            shift_idx = 0;
    };
    struct types_opts {
      enum options { interleaved, non_interleaved, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    cce_reg_map_type_c_() = default;
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    interleaved_s_& interleaved()
    {
      assert_choice_type(types::interleaved, type_, "cce-REG-MappingType");
      return c;
    }
    const interleaved_s_& interleaved() const
    {
      assert_choice_type(types::interleaved, type_, "cce-REG-MappingType");
      return c;
    }
    interleaved_s_& set_interleaved();
    void            set_non_interleaved();

  private:
    types          type_;
    interleaved_s_ c;
  };
  struct precoder_granularity_opts {
    enum options { same_as_reg_bundle, all_contiguous_rbs, nulltype } value;

    const char* to_string() const;
  };
  using precoder_granularity_e_             = enumerated<precoder_granularity_opts>;
  using tci_states_pdcch_to_add_list_l_     = dyn_array<uint8_t>;
  using tci_states_pdcch_to_release_list_l_ = dyn_array<uint8_t>;

  // member variables
  bool                                ext                              = false;
  bool                                tci_present_in_dci_present       = false;
  bool                                pdcch_dmrs_scrambling_id_present = false;
  uint8_t                             coreset_id                       = 0;
  fixed_bitstring<45>                 freq_domain_res;
  uint8_t                             dur = 1;
  cce_reg_map_type_c_                 cce_reg_map_type;
  precoder_granularity_e_             precoder_granularity;
  tci_states_pdcch_to_add_list_l_     tci_states_pdcch_to_add_list;
  tci_states_pdcch_to_release_list_l_ tci_states_pdcch_to_release_list;
  uint32_t                            pdcch_dmrs_scrambling_id = 0;
  // ...
  // group 0
  bool    rb_offset_r16_present           = false;
  bool    tci_present_dci_1_2_r16_present = false;
  bool    coreset_pool_idx_r16_present    = false;
  bool    coreset_id_v1610_present        = false;
  uint8_t rb_offset_r16                   = 0;
  uint8_t tci_present_dci_1_2_r16         = 1;
  uint8_t coreset_pool_idx_r16            = 0;
  uint8_t coreset_id_v1610                = 12;
  // group 1
  bool follow_unified_tci_state_r17_present = false;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// InvalidSymbolPattern-r16 ::= SEQUENCE
struct invalid_symbol_pattern_r16_s {
  struct symbols_r16_c_ {
    struct types_opts {
      enum options { one_slot, two_slots, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    symbols_r16_c_() = default;
    symbols_r16_c_(const symbols_r16_c_& other);
    symbols_r16_c_& operator=(const symbols_r16_c_& other);
    ~symbols_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    fixed_bitstring<14>& one_slot()
    {
      assert_choice_type(types::one_slot, type_, "symbols-r16");
      return c.get<fixed_bitstring<14>>();
    }
    fixed_bitstring<28>& two_slots()
    {
      assert_choice_type(types::two_slots, type_, "symbols-r16");
      return c.get<fixed_bitstring<28>>();
    }
    const fixed_bitstring<14>& one_slot() const
    {
      assert_choice_type(types::one_slot, type_, "symbols-r16");
      return c.get<fixed_bitstring<14>>();
    }
    const fixed_bitstring<28>& two_slots() const
    {
      assert_choice_type(types::two_slots, type_, "symbols-r16");
      return c.get<fixed_bitstring<28>>();
    }
    fixed_bitstring<14>& set_one_slot();
    fixed_bitstring<28>& set_two_slots();

  private:
    types                                type_;
    choice_buffer_t<fixed_bitstring<28>> c;

    void destroy_();
  };
  struct periodicity_and_pattern_r16_c_ {
    struct types_opts {
      enum options { n2, n4, n5, n8, n10, n20, n40, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    periodicity_and_pattern_r16_c_() = default;
    periodicity_and_pattern_r16_c_(const periodicity_and_pattern_r16_c_& other);
    periodicity_and_pattern_r16_c_& operator=(const periodicity_and_pattern_r16_c_& other);
    ~periodicity_and_pattern_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    fixed_bitstring<2>& n2()
    {
      assert_choice_type(types::n2, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<2>>();
    }
    fixed_bitstring<4>& n4()
    {
      assert_choice_type(types::n4, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<4>>();
    }
    fixed_bitstring<5>& n5()
    {
      assert_choice_type(types::n5, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<5>>();
    }
    fixed_bitstring<8>& n8()
    {
      assert_choice_type(types::n8, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<8>>();
    }
    fixed_bitstring<10>& n10()
    {
      assert_choice_type(types::n10, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<10>>();
    }
    fixed_bitstring<20>& n20()
    {
      assert_choice_type(types::n20, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<20>>();
    }
    fixed_bitstring<40>& n40()
    {
      assert_choice_type(types::n40, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<40>>();
    }
    const fixed_bitstring<2>& n2() const
    {
      assert_choice_type(types::n2, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<2>>();
    }
    const fixed_bitstring<4>& n4() const
    {
      assert_choice_type(types::n4, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<4>>();
    }
    const fixed_bitstring<5>& n5() const
    {
      assert_choice_type(types::n5, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<5>>();
    }
    const fixed_bitstring<8>& n8() const
    {
      assert_choice_type(types::n8, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<8>>();
    }
    const fixed_bitstring<10>& n10() const
    {
      assert_choice_type(types::n10, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<10>>();
    }
    const fixed_bitstring<20>& n20() const
    {
      assert_choice_type(types::n20, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<20>>();
    }
    const fixed_bitstring<40>& n40() const
    {
      assert_choice_type(types::n40, type_, "periodicityAndPattern-r16");
      return c.get<fixed_bitstring<40>>();
    }
    fixed_bitstring<2>&  set_n2();
    fixed_bitstring<4>&  set_n4();
    fixed_bitstring<5>&  set_n5();
    fixed_bitstring<8>&  set_n8();
    fixed_bitstring<10>& set_n10();
    fixed_bitstring<20>& set_n20();
    fixed_bitstring<40>& set_n40();

  private:
    types                                type_;
    choice_buffer_t<fixed_bitstring<40>> c;

    void destroy_();
  };

  // member variables
  bool                           ext                                 = false;
  bool                           periodicity_and_pattern_r16_present = false;
  symbols_r16_c_                 symbols_r16;
  periodicity_and_pattern_r16_c_ periodicity_and_pattern_r16;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// MPE-Resource-r17 ::= SEQUENCE
struct mpe_res_r17_s {
  struct mpe_ref_sig_r17_c_ {
    struct types_opts {
      enum options { csi_rs_res_r17, ssb_res_r17, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    mpe_ref_sig_r17_c_() = default;
    mpe_ref_sig_r17_c_(const mpe_ref_sig_r17_c_& other);
    mpe_ref_sig_r17_c_& operator=(const mpe_ref_sig_r17_c_& other);
    ~mpe_ref_sig_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& csi_rs_res_r17()
    {
      assert_choice_type(types::csi_rs_res_r17, type_, "mpe-ReferenceSignal-r17");
      return c.get<uint8_t>();
    }
    uint8_t& ssb_res_r17()
    {
      assert_choice_type(types::ssb_res_r17, type_, "mpe-ReferenceSignal-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_res_r17() const
    {
      assert_choice_type(types::csi_rs_res_r17, type_, "mpe-ReferenceSignal-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_res_r17() const
    {
      assert_choice_type(types::ssb_res_r17, type_, "mpe-ReferenceSignal-r17");
      return c.get<uint8_t>();
    }
    uint8_t& set_csi_rs_res_r17();
    uint8_t& set_ssb_res_r17();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  bool               cell_r17_present    = false;
  bool               add_pci_r17_present = false;
  uint8_t            mpe_res_id_r17      = 1;
  uint8_t            cell_r17            = 0;
  uint8_t            add_pci_r17         = 1;
  mpe_ref_sig_r17_c_ mpe_ref_sig_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-Config ::= SEQUENCE
struct pucch_cfg_s {
  using res_set_to_add_mod_list_l_               = dyn_array<pucch_res_set_s>;
  using res_set_to_release_list_l_               = bounded_array<uint8_t, 4>;
  using res_to_add_mod_list_l_                   = dyn_array<pucch_res_s>;
  using res_to_release_list_l_                   = dyn_array<uint8_t>;
  using sched_request_res_to_add_mod_list_l_     = dyn_array<sched_request_res_cfg_s>;
  using sched_request_res_to_release_list_l_     = bounded_array<uint8_t, 8>;
  using multi_csi_pucch_res_list_l_              = bounded_array<uint8_t, 2>;
  using dl_data_to_ul_ack_l_                     = bounded_array<uint8_t, 8>;
  using spatial_relation_info_to_add_mod_list_l_ = dyn_array<pucch_spatial_relation_info_s>;
  using spatial_relation_info_to_release_list_l_ = bounded_array<uint8_t, 8>;
  using res_to_add_mod_list_ext_v1610_l_         = dyn_array<pucch_res_ext_v1610_s>;
  struct subslot_len_for_pucch_r16_c_ {
    struct normal_cp_r16_opts {
      enum options { n2, n7, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using normal_cp_r16_e_ = enumerated<normal_cp_r16_opts>;
    struct extended_cp_r16_opts {
      enum options { n2, n6, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using extended_cp_r16_e_ = enumerated<extended_cp_r16_opts>;
    struct types_opts {
      enum options { normal_cp_r16, extended_cp_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    subslot_len_for_pucch_r16_c_() = default;
    subslot_len_for_pucch_r16_c_(const subslot_len_for_pucch_r16_c_& other);
    subslot_len_for_pucch_r16_c_& operator=(const subslot_len_for_pucch_r16_c_& other);
    ~subslot_len_for_pucch_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    normal_cp_r16_e_& normal_cp_r16()
    {
      assert_choice_type(types::normal_cp_r16, type_, "subslotLengthForPUCCH-r16");
      return c.get<normal_cp_r16_e_>();
    }
    extended_cp_r16_e_& extended_cp_r16()
    {
      assert_choice_type(types::extended_cp_r16, type_, "subslotLengthForPUCCH-r16");
      return c.get<extended_cp_r16_e_>();
    }
    const normal_cp_r16_e_& normal_cp_r16() const
    {
      assert_choice_type(types::normal_cp_r16, type_, "subslotLengthForPUCCH-r16");
      return c.get<normal_cp_r16_e_>();
    }
    const extended_cp_r16_e_& extended_cp_r16() const
    {
      assert_choice_type(types::extended_cp_r16, type_, "subslotLengthForPUCCH-r16");
      return c.get<extended_cp_r16_e_>();
    }
    normal_cp_r16_e_&   set_normal_cp_r16();
    extended_cp_r16_e_& set_extended_cp_r16();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  using spatial_relation_info_to_add_mod_list_size_ext_v1610_l_ = dyn_array<pucch_spatial_relation_info_s>;
  using spatial_relation_info_to_release_list_size_ext_v1610_l_ = dyn_array<uint8_t>;
  using spatial_relation_info_to_add_mod_list_ext_v1610_l_      = dyn_array<pucch_spatial_relation_info_ext_r16_s>;
  using spatial_relation_info_to_release_list_ext_v1610_l_      = dyn_array<uint8_t>;
  using res_group_to_add_mod_list_r16_l_                        = dyn_array<pucch_res_group_r16_s>;
  using res_group_to_release_list_r16_l_                        = bounded_array<uint8_t, 4>;
  using sched_request_res_to_add_mod_list_ext_v1610_l_          = dyn_array<sched_request_res_cfg_ext_v1610_s>;
  struct map_pattern_r17_opts {
    enum options { cyclic_map, sequential_map, nulltype } value;

    const char* to_string() const;
  };
  using map_pattern_r17_e_                             = enumerated<map_pattern_r17_opts>;
  using pwr_ctrl_set_info_to_add_mod_list_r17_l_       = dyn_array<pucch_pwr_ctrl_set_info_r17_s>;
  using pwr_ctrl_set_info_to_release_list_r17_l_       = bounded_array<uint8_t, 8>;
  using sched_request_res_to_add_mod_list_ext_v1700_l_ = dyn_array<sched_request_res_cfg_ext_v1700_s>;

  // member variables
  bool                                     ext                    = false;
  bool                                     format1_present        = false;
  bool                                     format2_present        = false;
  bool                                     format3_present        = false;
  bool                                     format4_present        = false;
  bool                                     pucch_pwr_ctrl_present = false;
  res_set_to_add_mod_list_l_               res_set_to_add_mod_list;
  res_set_to_release_list_l_               res_set_to_release_list;
  res_to_add_mod_list_l_                   res_to_add_mod_list;
  res_to_release_list_l_                   res_to_release_list;
  setup_release_c<pucch_format_cfg_s>      format1;
  setup_release_c<pucch_format_cfg_s>      format2;
  setup_release_c<pucch_format_cfg_s>      format3;
  setup_release_c<pucch_format_cfg_s>      format4;
  sched_request_res_to_add_mod_list_l_     sched_request_res_to_add_mod_list;
  sched_request_res_to_release_list_l_     sched_request_res_to_release_list;
  multi_csi_pucch_res_list_l_              multi_csi_pucch_res_list;
  dl_data_to_ul_ack_l_                     dl_data_to_ul_ack;
  spatial_relation_info_to_add_mod_list_l_ spatial_relation_info_to_add_mod_list;
  spatial_relation_info_to_release_list_l_ spatial_relation_info_to_release_list;
  pucch_pwr_ctrl_s                         pucch_pwr_ctrl;
  // ...
  // group 0
  bool                                       nof_bits_for_pucch_res_ind_dci_1_2_r16_present = false;
  bool                                       dmrs_ul_transform_precoding_pucch_r16_present  = false;
  copy_ptr<res_to_add_mod_list_ext_v1610_l_> res_to_add_mod_list_ext_v1610;
  copy_ptr<setup_release_c<dyn_seq_of<integer<int8_t, -1, 15>, 1, 8>>>  dl_data_to_ul_ack_r16;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 15>, 1, 16>>> ul_access_cfg_list_dci_1_1_r16;
  copy_ptr<subslot_len_for_pucch_r16_c_>                                subslot_len_for_pucch_r16;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 15>, 1, 8>>>  dl_data_to_ul_ack_dci_1_2_r16;
  uint8_t                                                               nof_bits_for_pucch_res_ind_dci_1_2_r16 = 0;
  copy_ptr<spatial_relation_info_to_add_mod_list_size_ext_v1610_l_>
      spatial_relation_info_to_add_mod_list_size_ext_v1610;
  copy_ptr<spatial_relation_info_to_release_list_size_ext_v1610_l_>
                                                                  spatial_relation_info_to_release_list_size_ext_v1610;
  copy_ptr<spatial_relation_info_to_add_mod_list_ext_v1610_l_>    spatial_relation_info_to_add_mod_list_ext_v1610;
  copy_ptr<spatial_relation_info_to_release_list_ext_v1610_l_>    spatial_relation_info_to_release_list_ext_v1610;
  copy_ptr<res_group_to_add_mod_list_r16_l_>                      res_group_to_add_mod_list_r16;
  copy_ptr<res_group_to_release_list_r16_l_>                      res_group_to_release_list_r16;
  copy_ptr<setup_release_c<dyn_seq_of<sps_pucch_an_r16_s, 1, 4>>> sps_pucch_an_list_r16;
  copy_ptr<sched_request_res_to_add_mod_list_ext_v1610_l_>        sched_request_res_to_add_mod_list_ext_v1610;
  // group 1
  bool                                                                   map_pattern_r17_present              = false;
  bool                                                                   second_tpc_field_dci_1_1_r17_present = false;
  bool                                                                   second_tpc_field_dci_1_2_r17_present = false;
  copy_ptr<setup_release_c<pucch_format_cfg_s>>                          format0_r17;
  copy_ptr<setup_release_c<pucch_format_cfg_ext_r17_s>>                  format2_ext_r17;
  copy_ptr<setup_release_c<pucch_format_cfg_ext_r17_s>>                  format3_ext_r17;
  copy_ptr<setup_release_c<pucch_format_cfg_ext_r17_s>>                  format4_ext_r17;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 15>, 1, 16>>>  ul_access_cfg_list_dci_1_2_r17;
  map_pattern_r17_e_                                                     map_pattern_r17;
  copy_ptr<pwr_ctrl_set_info_to_add_mod_list_r17_l_>                     pwr_ctrl_set_info_to_add_mod_list_r17;
  copy_ptr<pwr_ctrl_set_info_to_release_list_r17_l_>                     pwr_ctrl_set_info_to_release_list_r17;
  copy_ptr<setup_release_c<dyn_seq_of<integer<int16_t, -1, 127>, 1, 8>>> dl_data_to_ul_ack_r17;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 127>, 1, 8>>>  dl_data_to_ul_ack_dci_1_2_r17;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 2>, 1, 3>>>    ul_access_cfg_list_dci_1_1_r17;
  copy_ptr<sched_request_res_to_add_mod_list_ext_v1700_l_>               sched_request_res_to_add_mod_list_ext_v1700;
  copy_ptr<setup_release_c<dmrs_bundling_pucch_cfg_r17_s>>               dmrs_bundling_pucch_cfg_r17;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 16, 31>, 1, 8>>>  dl_data_to_ul_ack_v1700;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 15>, 1, 8>>>   dl_data_to_ul_ack_multicast_dci_format4_1_r17;
  copy_ptr<setup_release_c<dyn_seq_of<sps_pucch_an_r16_s, 1, 4>>>        sps_pucch_an_list_multicast_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUSCH-PowerControl ::= SEQUENCE
struct pusch_pwr_ctrl_s {
  using p0_alpha_sets_l_                   = dyn_array<p0_pusch_alpha_set_s>;
  using pathloss_ref_rs_to_add_mod_list_l_ = dyn_array<pusch_pathloss_ref_rs_s>;
  using pathloss_ref_rs_to_release_list_l_ = bounded_array<uint8_t, 4>;
  using sri_pusch_map_to_add_mod_list_l_   = dyn_array<sri_pusch_pwr_ctrl_s>;
  using sri_pusch_map_to_release_list_l_   = bounded_array<uint8_t, 16>;

  // member variables
  bool                               tpc_accumulation_present               = false;
  bool                               msg3_alpha_present                     = false;
  bool                               p0_nominal_without_grant_present       = false;
  bool                               two_pusch_pc_adjustment_states_present = false;
  bool                               delta_mcs_present                      = false;
  alpha_e                            msg3_alpha;
  int16_t                            p0_nominal_without_grant = -202;
  p0_alpha_sets_l_                   p0_alpha_sets;
  pathloss_ref_rs_to_add_mod_list_l_ pathloss_ref_rs_to_add_mod_list;
  pathloss_ref_rs_to_release_list_l_ pathloss_ref_rs_to_release_list;
  sri_pusch_map_to_add_mod_list_l_   sri_pusch_map_to_add_mod_list;
  sri_pusch_map_to_release_list_l_   sri_pusch_map_to_release_list;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RateMatchPatternGroup ::= SEQUENCE (SIZE (1..8)) OF RateMatchPatternGroup-item
using rate_match_pattern_group_l = dyn_array<rate_match_pattern_group_item_c_>;

// RateMatchPattern ::= SEQUENCE
struct rate_match_pattern_s {
  struct pattern_type_c_ {
    struct bitmaps_s_ {
      struct symbols_in_res_block_c_ {
        struct types_opts {
          enum options { one_slot, two_slots, nulltype } value;
          typedef uint8_t number_type;

          const char* to_string() const;
          uint8_t     to_number() const;
        };
        using types = enumerated<types_opts>;

        // choice methods
        symbols_in_res_block_c_() = default;
        symbols_in_res_block_c_(const symbols_in_res_block_c_& other);
        symbols_in_res_block_c_& operator=(const symbols_in_res_block_c_& other);
        ~symbols_in_res_block_c_() { destroy_(); }
        void        set(types::options e = types::nulltype);
        types       type() const { return type_; }
        SRSASN_CODE pack(bit_ref& bref) const;
        SRSASN_CODE unpack(cbit_ref& bref);
        void        to_json(json_writer& j) const;
        // getters
        fixed_bitstring<14>& one_slot()
        {
          assert_choice_type(types::one_slot, type_, "symbolsInResourceBlock");
          return c.get<fixed_bitstring<14>>();
        }
        fixed_bitstring<28>& two_slots()
        {
          assert_choice_type(types::two_slots, type_, "symbolsInResourceBlock");
          return c.get<fixed_bitstring<28>>();
        }
        const fixed_bitstring<14>& one_slot() const
        {
          assert_choice_type(types::one_slot, type_, "symbolsInResourceBlock");
          return c.get<fixed_bitstring<14>>();
        }
        const fixed_bitstring<28>& two_slots() const
        {
          assert_choice_type(types::two_slots, type_, "symbolsInResourceBlock");
          return c.get<fixed_bitstring<28>>();
        }
        fixed_bitstring<14>& set_one_slot();
        fixed_bitstring<28>& set_two_slots();

      private:
        types                                type_;
        choice_buffer_t<fixed_bitstring<28>> c;

        void destroy_();
      };
      struct periodicity_and_pattern_c_ {
        struct types_opts {
          enum options { n2, n4, n5, n8, n10, n20, n40, nulltype } value;
          typedef uint8_t number_type;

          const char* to_string() const;
          uint8_t     to_number() const;
        };
        using types = enumerated<types_opts>;

        // choice methods
        periodicity_and_pattern_c_() = default;
        periodicity_and_pattern_c_(const periodicity_and_pattern_c_& other);
        periodicity_and_pattern_c_& operator=(const periodicity_and_pattern_c_& other);
        ~periodicity_and_pattern_c_() { destroy_(); }
        void        set(types::options e = types::nulltype);
        types       type() const { return type_; }
        SRSASN_CODE pack(bit_ref& bref) const;
        SRSASN_CODE unpack(cbit_ref& bref);
        void        to_json(json_writer& j) const;
        // getters
        fixed_bitstring<2>& n2()
        {
          assert_choice_type(types::n2, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<2>>();
        }
        fixed_bitstring<4>& n4()
        {
          assert_choice_type(types::n4, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<4>>();
        }
        fixed_bitstring<5>& n5()
        {
          assert_choice_type(types::n5, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<5>>();
        }
        fixed_bitstring<8>& n8()
        {
          assert_choice_type(types::n8, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<8>>();
        }
        fixed_bitstring<10>& n10()
        {
          assert_choice_type(types::n10, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<10>>();
        }
        fixed_bitstring<20>& n20()
        {
          assert_choice_type(types::n20, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<20>>();
        }
        fixed_bitstring<40>& n40()
        {
          assert_choice_type(types::n40, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<40>>();
        }
        const fixed_bitstring<2>& n2() const
        {
          assert_choice_type(types::n2, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<2>>();
        }
        const fixed_bitstring<4>& n4() const
        {
          assert_choice_type(types::n4, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<4>>();
        }
        const fixed_bitstring<5>& n5() const
        {
          assert_choice_type(types::n5, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<5>>();
        }
        const fixed_bitstring<8>& n8() const
        {
          assert_choice_type(types::n8, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<8>>();
        }
        const fixed_bitstring<10>& n10() const
        {
          assert_choice_type(types::n10, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<10>>();
        }
        const fixed_bitstring<20>& n20() const
        {
          assert_choice_type(types::n20, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<20>>();
        }
        const fixed_bitstring<40>& n40() const
        {
          assert_choice_type(types::n40, type_, "periodicityAndPattern");
          return c.get<fixed_bitstring<40>>();
        }
        fixed_bitstring<2>&  set_n2();
        fixed_bitstring<4>&  set_n4();
        fixed_bitstring<5>&  set_n5();
        fixed_bitstring<8>&  set_n8();
        fixed_bitstring<10>& set_n10();
        fixed_bitstring<20>& set_n20();
        fixed_bitstring<40>& set_n40();

      private:
        types                                type_;
        choice_buffer_t<fixed_bitstring<40>> c;

        void destroy_();
      };

      // member variables
      bool                       ext                             = false;
      bool                       periodicity_and_pattern_present = false;
      fixed_bitstring<275>       res_blocks;
      symbols_in_res_block_c_    symbols_in_res_block;
      periodicity_and_pattern_c_ periodicity_and_pattern;
      // ...
    };
    struct types_opts {
      enum options { bitmaps, coreset, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    pattern_type_c_() = default;
    pattern_type_c_(const pattern_type_c_& other);
    pattern_type_c_& operator=(const pattern_type_c_& other);
    ~pattern_type_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    bitmaps_s_& bitmaps()
    {
      assert_choice_type(types::bitmaps, type_, "patternType");
      return c.get<bitmaps_s_>();
    }
    uint8_t& coreset()
    {
      assert_choice_type(types::coreset, type_, "patternType");
      return c.get<uint8_t>();
    }
    const bitmaps_s_& bitmaps() const
    {
      assert_choice_type(types::bitmaps, type_, "patternType");
      return c.get<bitmaps_s_>();
    }
    const uint8_t& coreset() const
    {
      assert_choice_type(types::coreset, type_, "patternType");
      return c.get<uint8_t>();
    }
    bitmaps_s_& set_bitmaps();
    uint8_t&    set_coreset();

  private:
    types                       type_;
    choice_buffer_t<bitmaps_s_> c;

    void destroy_();
  };
  struct dummy_opts {
    enum options { dyn, semi_static, nulltype } value;

    const char* to_string() const;
  };
  using dummy_e_ = enumerated<dummy_opts>;

  // member variables
  bool                 ext                        = false;
  bool                 subcarrier_spacing_present = false;
  uint8_t              rate_match_pattern_id      = 0;
  pattern_type_c_      pattern_type;
  subcarrier_spacing_e subcarrier_spacing;
  dummy_e_             dummy;
  // ...
  // group 0
  bool    coreset_r16_present = false;
  uint8_t coreset_r16         = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SearchSpaceExt-r16 ::= SEQUENCE
struct search_space_ext_r16_s {
  struct search_space_type_r16_s_ {
    struct common_r16_s_ {
      struct dci_format2_4_r16_s_ {
        struct nrof_candidates_ci_r16_s_ {
          struct aggregation_level1_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level1_r16_e_ = enumerated<aggregation_level1_r16_opts>;
          struct aggregation_level2_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level2_r16_e_ = enumerated<aggregation_level2_r16_opts>;
          struct aggregation_level4_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level4_r16_e_ = enumerated<aggregation_level4_r16_opts>;
          struct aggregation_level8_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level8_r16_e_ = enumerated<aggregation_level8_r16_opts>;
          struct aggregation_level16_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level16_r16_e_ = enumerated<aggregation_level16_r16_opts>;

          // member variables
          bool                       aggregation_level1_r16_present  = false;
          bool                       aggregation_level2_r16_present  = false;
          bool                       aggregation_level4_r16_present  = false;
          bool                       aggregation_level8_r16_present  = false;
          bool                       aggregation_level16_r16_present = false;
          aggregation_level1_r16_e_  aggregation_level1_r16;
          aggregation_level2_r16_e_  aggregation_level2_r16;
          aggregation_level4_r16_e_  aggregation_level4_r16;
          aggregation_level8_r16_e_  aggregation_level8_r16;
          aggregation_level16_r16_e_ aggregation_level16_r16;
        };

        // member variables
        bool                      ext = false;
        nrof_candidates_ci_r16_s_ nrof_candidates_ci_r16;
        // ...
      };
      struct dci_format2_5_r16_s_ {
        struct nrof_candidates_iab_r16_s_ {
          struct aggregation_level1_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level1_r16_e_ = enumerated<aggregation_level1_r16_opts>;
          struct aggregation_level2_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level2_r16_e_ = enumerated<aggregation_level2_r16_opts>;
          struct aggregation_level4_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level4_r16_e_ = enumerated<aggregation_level4_r16_opts>;
          struct aggregation_level8_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level8_r16_e_ = enumerated<aggregation_level8_r16_opts>;
          struct aggregation_level16_r16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level16_r16_e_ = enumerated<aggregation_level16_r16_opts>;

          // member variables
          bool                       aggregation_level1_r16_present  = false;
          bool                       aggregation_level2_r16_present  = false;
          bool                       aggregation_level4_r16_present  = false;
          bool                       aggregation_level8_r16_present  = false;
          bool                       aggregation_level16_r16_present = false;
          aggregation_level1_r16_e_  aggregation_level1_r16;
          aggregation_level2_r16_e_  aggregation_level2_r16;
          aggregation_level4_r16_e_  aggregation_level4_r16;
          aggregation_level8_r16_e_  aggregation_level8_r16;
          aggregation_level16_r16_e_ aggregation_level16_r16;
        };

        // member variables
        bool                       ext = false;
        nrof_candidates_iab_r16_s_ nrof_candidates_iab_r16;
        // ...
      };
      struct dci_format2_6_r16_s_ {
        bool ext = false;
        // ...
      };

      // member variables
      bool                 ext                       = false;
      bool                 dci_format2_4_r16_present = false;
      bool                 dci_format2_5_r16_present = false;
      bool                 dci_format2_6_r16_present = false;
      dci_format2_4_r16_s_ dci_format2_4_r16;
      dci_format2_5_r16_s_ dci_format2_5_r16;
      dci_format2_6_r16_s_ dci_format2_6_r16;
      // ...
    };

    // member variables
    common_r16_s_ common_r16;
  };
  using search_space_group_id_list_r16_l_ = bounded_array<uint8_t, 2>;

  // member variables
  bool                              coreset_id_r16_present             = false;
  bool                              search_space_type_r16_present      = false;
  bool                              freq_monitor_locations_r16_present = false;
  uint8_t                           coreset_id_r16                     = 0;
  search_space_type_r16_s_          search_space_type_r16;
  search_space_group_id_list_r16_l_ search_space_group_id_list_r16;
  fixed_bitstring<5>                freq_monitor_locations_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SearchSpaceExt-v1700 ::= SEQUENCE
struct search_space_ext_v1700_s {
  struct monitoring_slot_periodicity_and_offset_v1710_c_ {
    struct types_opts {
      enum options { sl32, sl64, sl128, sl5120, sl10240, sl20480, nulltype } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    monitoring_slot_periodicity_and_offset_v1710_c_() = default;
    monitoring_slot_periodicity_and_offset_v1710_c_(const monitoring_slot_periodicity_and_offset_v1710_c_& other);
    monitoring_slot_periodicity_and_offset_v1710_c_&
    operator=(const monitoring_slot_periodicity_and_offset_v1710_c_& other);
    ~monitoring_slot_periodicity_and_offset_v1710_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& sl32()
    {
      assert_choice_type(types::sl32, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint8_t>();
    }
    uint8_t& sl64()
    {
      assert_choice_type(types::sl64, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint8_t>();
    }
    uint8_t& sl128()
    {
      assert_choice_type(types::sl128, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint8_t>();
    }
    uint16_t& sl5120()
    {
      assert_choice_type(types::sl5120, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint16_t>();
    }
    uint16_t& sl10240()
    {
      assert_choice_type(types::sl10240, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint16_t>();
    }
    uint16_t& sl20480()
    {
      assert_choice_type(types::sl20480, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint16_t>();
    }
    const uint8_t& sl32() const
    {
      assert_choice_type(types::sl32, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint8_t>();
    }
    const uint8_t& sl64() const
    {
      assert_choice_type(types::sl64, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint8_t>();
    }
    const uint8_t& sl128() const
    {
      assert_choice_type(types::sl128, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint8_t>();
    }
    const uint16_t& sl5120() const
    {
      assert_choice_type(types::sl5120, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint16_t>();
    }
    const uint16_t& sl10240() const
    {
      assert_choice_type(types::sl10240, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint16_t>();
    }
    const uint16_t& sl20480() const
    {
      assert_choice_type(types::sl20480, type_, "monitoringSlotPeriodicityAndOffset-v1710");
      return c.get<uint16_t>();
    }
    uint8_t&  set_sl32();
    uint8_t&  set_sl64();
    uint8_t&  set_sl128();
    uint16_t& set_sl5120();
    uint16_t& set_sl10240();
    uint16_t& set_sl20480();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct monitoring_slots_within_slot_group_r17_c_ {
    struct types_opts {
      enum options { slot_group_len4_r17, slot_group_len8_r17, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    monitoring_slots_within_slot_group_r17_c_() = default;
    monitoring_slots_within_slot_group_r17_c_(const monitoring_slots_within_slot_group_r17_c_& other);
    monitoring_slots_within_slot_group_r17_c_& operator=(const monitoring_slots_within_slot_group_r17_c_& other);
    ~monitoring_slots_within_slot_group_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    fixed_bitstring<4>& slot_group_len4_r17()
    {
      assert_choice_type(types::slot_group_len4_r17, type_, "monitoringSlotsWithinSlotGroup-r17");
      return c.get<fixed_bitstring<4>>();
    }
    fixed_bitstring<8>& slot_group_len8_r17()
    {
      assert_choice_type(types::slot_group_len8_r17, type_, "monitoringSlotsWithinSlotGroup-r17");
      return c.get<fixed_bitstring<8>>();
    }
    const fixed_bitstring<4>& slot_group_len4_r17() const
    {
      assert_choice_type(types::slot_group_len4_r17, type_, "monitoringSlotsWithinSlotGroup-r17");
      return c.get<fixed_bitstring<4>>();
    }
    const fixed_bitstring<8>& slot_group_len8_r17() const
    {
      assert_choice_type(types::slot_group_len8_r17, type_, "monitoringSlotsWithinSlotGroup-r17");
      return c.get<fixed_bitstring<8>>();
    }
    fixed_bitstring<4>& set_slot_group_len4_r17();
    fixed_bitstring<8>& set_slot_group_len8_r17();

  private:
    types                               type_;
    choice_buffer_t<fixed_bitstring<8>> c;

    void destroy_();
  };
  struct search_space_type_r17_s_ {
    struct common_r17_s_ {
      struct dci_format4_0_r17_s_ {
        bool ext = false;
        // ...
      };
      struct dci_format4_1_r17_s_ {
        bool ext = false;
        // ...
      };
      struct dci_format4_2_r17_s_ {
        bool ext = false;
        // ...
      };
      struct dci_format4_1_and_format4_2_r17_s_ {
        bool ext = false;
        // ...
      };
      struct dci_format2_7_r17_s_ {
        struct nrof_candidates_pei_r17_s_ {
          struct aggregation_level4_r17_opts {
            enum options { n0, n1, n2, n3, n4, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level4_r17_e_ = enumerated<aggregation_level4_r17_opts>;
          struct aggregation_level8_r17_opts {
            enum options { n0, n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level8_r17_e_ = enumerated<aggregation_level8_r17_opts>;
          struct aggregation_level16_r17_opts {
            enum options { n0, n1, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level16_r17_e_ = enumerated<aggregation_level16_r17_opts>;

          // member variables
          bool                       aggregation_level4_r17_present  = false;
          bool                       aggregation_level8_r17_present  = false;
          bool                       aggregation_level16_r17_present = false;
          aggregation_level4_r17_e_  aggregation_level4_r17;
          aggregation_level8_r17_e_  aggregation_level8_r17;
          aggregation_level16_r17_e_ aggregation_level16_r17;
        };

        // member variables
        bool                       ext = false;
        nrof_candidates_pei_r17_s_ nrof_candidates_pei_r17;
        // ...
      };

      // member variables
      bool                               dci_format4_0_r17_present               = false;
      bool                               dci_format4_1_r17_present               = false;
      bool                               dci_format4_2_r17_present               = false;
      bool                               dci_format4_1_and_format4_2_r17_present = false;
      bool                               dci_format2_7_r17_present               = false;
      dci_format4_0_r17_s_               dci_format4_0_r17;
      dci_format4_1_r17_s_               dci_format4_1_r17;
      dci_format4_2_r17_s_               dci_format4_2_r17;
      dci_format4_1_and_format4_2_r17_s_ dci_format4_1_and_format4_2_r17;
      dci_format2_7_r17_s_               dci_format2_7_r17;
    };

    // member variables
    common_r17_s_ common_r17;
  };
  using search_space_group_id_list_r17_l_ = bounded_array<uint8_t, 3>;

  // member variables
  bool                                            monitoring_slot_periodicity_and_offset_v1710_present = false;
  bool                                            monitoring_slots_within_slot_group_r17_present       = false;
  bool                                            dur_r17_present                                      = false;
  bool                                            search_space_type_r17_present                        = false;
  bool                                            search_space_linking_id_r17_present                  = false;
  monitoring_slot_periodicity_and_offset_v1710_c_ monitoring_slot_periodicity_and_offset_v1710;
  monitoring_slots_within_slot_group_r17_c_       monitoring_slots_within_slot_group_r17;
  uint16_t                                        dur_r17 = 4;
  search_space_type_r17_s_                        search_space_type_r17;
  search_space_group_id_list_r17_l_               search_space_group_id_list_r17;
  uint8_t                                         search_space_linking_id_r17 = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SearchSpace ::= SEQUENCE
struct search_space_s {
  struct monitoring_slot_periodicity_and_offset_c_ {
    struct types_opts {
      enum options {
        sl1,
        sl2,
        sl4,
        sl5,
        sl8,
        sl10,
        sl16,
        sl20,
        sl40,
        sl80,
        sl160,
        sl320,
        sl640,
        sl1280,
        sl2560,
        nulltype
      } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    monitoring_slot_periodicity_and_offset_c_() = default;
    monitoring_slot_periodicity_and_offset_c_(const monitoring_slot_periodicity_and_offset_c_& other);
    monitoring_slot_periodicity_and_offset_c_& operator=(const monitoring_slot_periodicity_and_offset_c_& other);
    ~monitoring_slot_periodicity_and_offset_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& sl2()
    {
      assert_choice_type(types::sl2, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl4()
    {
      assert_choice_type(types::sl4, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl5()
    {
      assert_choice_type(types::sl5, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl8()
    {
      assert_choice_type(types::sl8, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl10()
    {
      assert_choice_type(types::sl10, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl16()
    {
      assert_choice_type(types::sl16, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl20()
    {
      assert_choice_type(types::sl20, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl40()
    {
      assert_choice_type(types::sl40, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl80()
    {
      assert_choice_type(types::sl80, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint8_t& sl160()
    {
      assert_choice_type(types::sl160, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    uint16_t& sl320()
    {
      assert_choice_type(types::sl320, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint16_t>();
    }
    uint16_t& sl640()
    {
      assert_choice_type(types::sl640, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint16_t>();
    }
    uint16_t& sl1280()
    {
      assert_choice_type(types::sl1280, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint16_t>();
    }
    uint16_t& sl2560()
    {
      assert_choice_type(types::sl2560, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint16_t>();
    }
    const uint8_t& sl2() const
    {
      assert_choice_type(types::sl2, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl4() const
    {
      assert_choice_type(types::sl4, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl5() const
    {
      assert_choice_type(types::sl5, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl8() const
    {
      assert_choice_type(types::sl8, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl10() const
    {
      assert_choice_type(types::sl10, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl16() const
    {
      assert_choice_type(types::sl16, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl20() const
    {
      assert_choice_type(types::sl20, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl40() const
    {
      assert_choice_type(types::sl40, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl80() const
    {
      assert_choice_type(types::sl80, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint8_t& sl160() const
    {
      assert_choice_type(types::sl160, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint8_t>();
    }
    const uint16_t& sl320() const
    {
      assert_choice_type(types::sl320, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint16_t>();
    }
    const uint16_t& sl640() const
    {
      assert_choice_type(types::sl640, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint16_t>();
    }
    const uint16_t& sl1280() const
    {
      assert_choice_type(types::sl1280, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint16_t>();
    }
    const uint16_t& sl2560() const
    {
      assert_choice_type(types::sl2560, type_, "monitoringSlotPeriodicityAndOffset");
      return c.get<uint16_t>();
    }
    void      set_sl1();
    uint8_t&  set_sl2();
    uint8_t&  set_sl4();
    uint8_t&  set_sl5();
    uint8_t&  set_sl8();
    uint8_t&  set_sl10();
    uint8_t&  set_sl16();
    uint8_t&  set_sl20();
    uint8_t&  set_sl40();
    uint8_t&  set_sl80();
    uint8_t&  set_sl160();
    uint16_t& set_sl320();
    uint16_t& set_sl640();
    uint16_t& set_sl1280();
    uint16_t& set_sl2560();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct nrof_candidates_s_ {
    struct aggregation_level1_opts {
      enum options { n0, n1, n2, n3, n4, n5, n6, n8, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using aggregation_level1_e_ = enumerated<aggregation_level1_opts>;
    struct aggregation_level2_opts {
      enum options { n0, n1, n2, n3, n4, n5, n6, n8, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using aggregation_level2_e_ = enumerated<aggregation_level2_opts>;
    struct aggregation_level4_opts {
      enum options { n0, n1, n2, n3, n4, n5, n6, n8, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using aggregation_level4_e_ = enumerated<aggregation_level4_opts>;
    struct aggregation_level8_opts {
      enum options { n0, n1, n2, n3, n4, n5, n6, n8, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using aggregation_level8_e_ = enumerated<aggregation_level8_opts>;
    struct aggregation_level16_opts {
      enum options { n0, n1, n2, n3, n4, n5, n6, n8, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using aggregation_level16_e_ = enumerated<aggregation_level16_opts>;

    // member variables
    aggregation_level1_e_  aggregation_level1;
    aggregation_level2_e_  aggregation_level2;
    aggregation_level4_e_  aggregation_level4;
    aggregation_level8_e_  aggregation_level8;
    aggregation_level16_e_ aggregation_level16;
  };
  struct search_space_type_c_ {
    struct common_s_ {
      struct dci_format0_0_and_format1_0_s_ {
        bool ext = false;
        // ...
      };
      struct dci_format2_0_s_ {
        struct nrof_candidates_sfi_s_ {
          struct aggregation_level1_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level1_e_ = enumerated<aggregation_level1_opts>;
          struct aggregation_level2_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level2_e_ = enumerated<aggregation_level2_opts>;
          struct aggregation_level4_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level4_e_ = enumerated<aggregation_level4_opts>;
          struct aggregation_level8_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level8_e_ = enumerated<aggregation_level8_opts>;
          struct aggregation_level16_opts {
            enum options { n1, n2, nulltype } value;
            typedef uint8_t number_type;

            const char* to_string() const;
            uint8_t     to_number() const;
          };
          using aggregation_level16_e_ = enumerated<aggregation_level16_opts>;

          // member variables
          bool                   aggregation_level1_present  = false;
          bool                   aggregation_level2_present  = false;
          bool                   aggregation_level4_present  = false;
          bool                   aggregation_level8_present  = false;
          bool                   aggregation_level16_present = false;
          aggregation_level1_e_  aggregation_level1;
          aggregation_level2_e_  aggregation_level2;
          aggregation_level4_e_  aggregation_level4;
          aggregation_level8_e_  aggregation_level8;
          aggregation_level16_e_ aggregation_level16;
        };

        // member variables
        bool                   ext = false;
        nrof_candidates_sfi_s_ nrof_candidates_sfi;
        // ...
      };
      struct dci_format2_1_s_ {
        bool ext = false;
        // ...
      };
      struct dci_format2_2_s_ {
        bool ext = false;
        // ...
      };
      struct dci_format2_3_s_ {
        struct dummy1_opts {
          enum options { sl1, sl2, sl4, sl5, sl8, sl10, sl16, sl20, nulltype } value;
          typedef uint8_t number_type;

          const char* to_string() const;
          uint8_t     to_number() const;
        };
        using dummy1_e_ = enumerated<dummy1_opts>;
        struct dummy2_opts {
          enum options { n1, n2, nulltype } value;
          typedef uint8_t number_type;

          const char* to_string() const;
          uint8_t     to_number() const;
        };
        using dummy2_e_ = enumerated<dummy2_opts>;

        // member variables
        bool      ext            = false;
        bool      dummy1_present = false;
        dummy1_e_ dummy1;
        dummy2_e_ dummy2;
        // ...
      };

      // member variables
      bool                           dci_format0_0_and_format1_0_present = false;
      bool                           dci_format2_0_present               = false;
      bool                           dci_format2_1_present               = false;
      bool                           dci_format2_2_present               = false;
      bool                           dci_format2_3_present               = false;
      dci_format0_0_and_format1_0_s_ dci_format0_0_and_format1_0;
      dci_format2_0_s_               dci_format2_0;
      dci_format2_1_s_               dci_format2_1;
      dci_format2_2_s_               dci_format2_2;
      dci_format2_3_s_               dci_format2_3;
    };
    struct ue_specific_s_ {
      struct dci_formats_opts {
        enum options { formats0_neg0_and_neg1_neg0, formats0_neg1_and_neg1_neg1, nulltype } value;

        const char* to_string() const;
      };
      using dci_formats_e_ = enumerated<dci_formats_opts>;
      struct dci_formats_sl_r16_opts {
        enum options {
          formats0_neg0_and_neg1_neg0,
          formats0_neg1_and_neg1_neg1,
          formats3_neg0,
          formats3_neg1,
          formats3_neg0_and_neg3_neg1,
          nulltype
        } value;

        const char* to_string() const;
      };
      using dci_formats_sl_r16_e_ = enumerated<dci_formats_sl_r16_opts>;
      struct dci_formats_ext_r16_opts {
        enum options {
          formats0_neg2_and_neg1_neg2,
          formats0_neg1_and_neg1_neg1_and_neg0_neg2_and_neg1_neg2,
          nulltype
        } value;

        const char* to_string() const;
      };
      using dci_formats_ext_r16_e_ = enumerated<dci_formats_ext_r16_opts>;

      // member variables
      bool           ext = false;
      dci_formats_e_ dci_formats;
      // ...
      // group 0
      bool                   dci_formats_mt_r16_present  = false;
      bool                   dci_formats_sl_r16_present  = false;
      bool                   dci_formats_ext_r16_present = false;
      dci_formats_sl_r16_e_  dci_formats_sl_r16;
      dci_formats_ext_r16_e_ dci_formats_ext_r16;

      // sequence methods
      SRSASN_CODE pack(bit_ref& bref) const;
      SRSASN_CODE unpack(cbit_ref& bref);
      void        to_json(json_writer& j) const;
    };
    struct types_opts {
      enum options { common, ue_specific, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    search_space_type_c_() = default;
    search_space_type_c_(const search_space_type_c_& other);
    search_space_type_c_& operator=(const search_space_type_c_& other);
    ~search_space_type_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    common_s_& common()
    {
      assert_choice_type(types::common, type_, "searchSpaceType");
      return c.get<common_s_>();
    }
    ue_specific_s_& ue_specific()
    {
      assert_choice_type(types::ue_specific, type_, "searchSpaceType");
      return c.get<ue_specific_s_>();
    }
    const common_s_& common() const
    {
      assert_choice_type(types::common, type_, "searchSpaceType");
      return c.get<common_s_>();
    }
    const ue_specific_s_& ue_specific() const
    {
      assert_choice_type(types::ue_specific, type_, "searchSpaceType");
      return c.get<ue_specific_s_>();
    }
    common_s_&      set_common();
    ue_specific_s_& set_ue_specific();

  private:
    types                                      type_;
    choice_buffer_t<common_s_, ue_specific_s_> c;

    void destroy_();
  };

  // member variables
  bool                                      coreset_id_present                             = false;
  bool                                      monitoring_slot_periodicity_and_offset_present = false;
  bool                                      dur_present                                    = false;
  bool                                      monitoring_symbols_within_slot_present         = false;
  bool                                      nrof_candidates_present                        = false;
  bool                                      search_space_type_present                      = false;
  uint8_t                                   search_space_id                                = 0;
  uint8_t                                   coreset_id                                     = 0;
  monitoring_slot_periodicity_and_offset_c_ monitoring_slot_periodicity_and_offset;
  uint16_t                                  dur = 2;
  fixed_bitstring<14>                       monitoring_symbols_within_slot;
  nrof_candidates_s_                        nrof_candidates;
  search_space_type_c_                      search_space_type;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SearchSpaceSwitchConfig-r16 ::= SEQUENCE
struct search_space_switch_cfg_r16_s {
  using cell_groups_for_switch_list_r16_l_ = dyn_array<cell_group_for_switch_r16_l>;

  // member variables
  bool                               search_space_switch_delay_r16_present = false;
  cell_groups_for_switch_list_r16_l_ cell_groups_for_switch_list_r16;
  uint8_t                            search_space_switch_delay_r16 = 10;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SearchSpaceSwitchConfig-r17 ::= SEQUENCE
struct search_space_switch_cfg_r17_s {
  bool    search_space_switch_timer_r17_present = false;
  bool    search_space_switch_delay_r17_present = false;
  uint8_t search_space_switch_timer_r17         = 1;
  uint8_t search_space_switch_delay_r17         = 10;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SPS-Config ::= SEQUENCE
struct sps_cfg_s {
  struct periodicity_opts {
    enum options {
      ms10,
      ms20,
      ms32,
      ms40,
      ms64,
      ms80,
      ms128,
      ms160,
      ms320,
      ms640,
      spare6,
      spare5,
      spare4,
      spare3,
      spare2,
      spare1,
      nulltype
    } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using periodicity_e_ = enumerated<periodicity_opts>;
  struct pdsch_aggregation_factor_r16_opts {
    enum options { n1, n2, n4, n8, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pdsch_aggregation_factor_r16_e_ = enumerated<pdsch_aggregation_factor_r16_opts>;

  // member variables
  bool           ext                 = false;
  bool           n1_pucch_an_present = false;
  bool           mcs_table_present   = false;
  periodicity_e_ periodicity;
  uint8_t        nrof_harq_processes = 1;
  uint8_t        n1_pucch_an         = 0;
  // ...
  // group 0
  bool                            sps_cfg_idx_r16_present              = false;
  bool                            harq_proc_id_offset_r16_present      = false;
  bool                            periodicity_ext_r16_present          = false;
  bool                            harq_codebook_id_r16_present         = false;
  bool                            pdsch_aggregation_factor_r16_present = false;
  uint8_t                         sps_cfg_idx_r16                      = 0;
  uint8_t                         harq_proc_id_offset_r16              = 0;
  uint16_t                        periodicity_ext_r16                  = 1;
  uint8_t                         harq_codebook_id_r16                 = 1;
  pdsch_aggregation_factor_r16_e_ pdsch_aggregation_factor_r16;
  // group 1
  bool     sps_harq_deferral_r17_present         = false;
  bool     n1_pucch_an_pucch_s_scell_r17_present = false;
  bool     periodicity_ext_r17_present           = false;
  bool     nrof_harq_processes_v1710_present     = false;
  bool     harq_proc_id_offset_v1700_present     = false;
  uint8_t  sps_harq_deferral_r17                 = 1;
  uint8_t  n1_pucch_an_pucch_s_scell_r17         = 0;
  uint16_t periodicity_ext_r17                   = 1;
  uint8_t  nrof_harq_processes_v1710             = 9;
  uint8_t  harq_proc_id_offset_v1700             = 16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SRS-PosResource-r16 ::= SEQUENCE
struct srs_pos_res_r16_s {
  struct tx_comb_r16_c_ {
    struct n2_r16_s_ {
      uint8_t comb_offset_n2_r16  = 0;
      uint8_t cyclic_shift_n2_r16 = 0;
    };
    struct n4_r16_s_ {
      uint8_t comb_offset_n4_r16  = 0;
      uint8_t cyclic_shift_n4_r16 = 0;
    };
    struct n8_r16_s_ {
      uint8_t comb_offset_n8_r16  = 0;
      uint8_t cyclic_shift_n8_r16 = 0;
    };
    struct types_opts {
      enum options { n2_r16, n4_r16, n8_r16, /*...*/ nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using types = enumerated<types_opts, true>;

    // choice methods
    tx_comb_r16_c_() = default;
    tx_comb_r16_c_(const tx_comb_r16_c_& other);
    tx_comb_r16_c_& operator=(const tx_comb_r16_c_& other);
    ~tx_comb_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    n2_r16_s_& n2_r16()
    {
      assert_choice_type(types::n2_r16, type_, "transmissionComb-r16");
      return c.get<n2_r16_s_>();
    }
    n4_r16_s_& n4_r16()
    {
      assert_choice_type(types::n4_r16, type_, "transmissionComb-r16");
      return c.get<n4_r16_s_>();
    }
    n8_r16_s_& n8_r16()
    {
      assert_choice_type(types::n8_r16, type_, "transmissionComb-r16");
      return c.get<n8_r16_s_>();
    }
    const n2_r16_s_& n2_r16() const
    {
      assert_choice_type(types::n2_r16, type_, "transmissionComb-r16");
      return c.get<n2_r16_s_>();
    }
    const n4_r16_s_& n4_r16() const
    {
      assert_choice_type(types::n4_r16, type_, "transmissionComb-r16");
      return c.get<n4_r16_s_>();
    }
    const n8_r16_s_& n8_r16() const
    {
      assert_choice_type(types::n8_r16, type_, "transmissionComb-r16");
      return c.get<n8_r16_s_>();
    }
    n2_r16_s_& set_n2_r16();
    n4_r16_s_& set_n4_r16();
    n8_r16_s_& set_n8_r16();

  private:
    types                                            type_;
    choice_buffer_t<n2_r16_s_, n4_r16_s_, n8_r16_s_> c;

    void destroy_();
  };
  struct res_map_r16_s_ {
    struct nrof_symbols_r16_opts {
      enum options { n1, n2, n4, n8, n12, nulltype } value;
      typedef uint8_t number_type;

      const char* to_string() const;
      uint8_t     to_number() const;
    };
    using nrof_symbols_r16_e_ = enumerated<nrof_symbols_r16_opts>;

    // member variables
    uint8_t             start_position_r16 = 0;
    nrof_symbols_r16_e_ nrof_symbols_r16;
  };
  struct freq_hop_r16_s_ {
    bool    ext       = false;
    uint8_t c_srs_r16 = 0;
    // ...
  };
  struct group_or_seq_hop_r16_opts {
    enum options { neither, group_hop, seq_hop, nulltype } value;

    const char* to_string() const;
  };
  using group_or_seq_hop_r16_e_ = enumerated<group_or_seq_hop_r16_opts>;
  struct res_type_r16_c_ {
    struct aperiodic_r16_s_ {
      bool    ext                     = false;
      bool    slot_offset_r16_present = false;
      uint8_t slot_offset_r16         = 1;
      // ...
    };
    struct semi_persistent_r16_s_ {
      bool                             ext = false;
      srs_periodicity_and_offset_r16_c periodicity_and_offset_sp_r16;
      // ...
      // group 0
      copy_ptr<srs_periodicity_and_offset_ext_r16_c> periodicity_and_offset_sp_ext_r16;

      // sequence methods
      SRSASN_CODE pack(bit_ref& bref) const;
      SRSASN_CODE unpack(cbit_ref& bref);
      void        to_json(json_writer& j) const;
    };
    struct periodic_r16_s_ {
      bool                             ext = false;
      srs_periodicity_and_offset_r16_c periodicity_and_offset_p_r16;
      // ...
      // group 0
      copy_ptr<srs_periodicity_and_offset_ext_r16_c> periodicity_and_offset_p_ext_r16;

      // sequence methods
      SRSASN_CODE pack(bit_ref& bref) const;
      SRSASN_CODE unpack(cbit_ref& bref);
      void        to_json(json_writer& j) const;
    };
    struct types_opts {
      enum options { aperiodic_r16, semi_persistent_r16, periodic_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    res_type_r16_c_() = default;
    res_type_r16_c_(const res_type_r16_c_& other);
    res_type_r16_c_& operator=(const res_type_r16_c_& other);
    ~res_type_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    aperiodic_r16_s_& aperiodic_r16()
    {
      assert_choice_type(types::aperiodic_r16, type_, "resourceType-r16");
      return c.get<aperiodic_r16_s_>();
    }
    semi_persistent_r16_s_& semi_persistent_r16()
    {
      assert_choice_type(types::semi_persistent_r16, type_, "resourceType-r16");
      return c.get<semi_persistent_r16_s_>();
    }
    periodic_r16_s_& periodic_r16()
    {
      assert_choice_type(types::periodic_r16, type_, "resourceType-r16");
      return c.get<periodic_r16_s_>();
    }
    const aperiodic_r16_s_& aperiodic_r16() const
    {
      assert_choice_type(types::aperiodic_r16, type_, "resourceType-r16");
      return c.get<aperiodic_r16_s_>();
    }
    const semi_persistent_r16_s_& semi_persistent_r16() const
    {
      assert_choice_type(types::semi_persistent_r16, type_, "resourceType-r16");
      return c.get<semi_persistent_r16_s_>();
    }
    const periodic_r16_s_& periodic_r16() const
    {
      assert_choice_type(types::periodic_r16, type_, "resourceType-r16");
      return c.get<periodic_r16_s_>();
    }
    aperiodic_r16_s_&       set_aperiodic_r16();
    semi_persistent_r16_s_& set_semi_persistent_r16();
    periodic_r16_s_&        set_periodic_r16();

  private:
    types                                                                      type_;
    choice_buffer_t<aperiodic_r16_s_, periodic_r16_s_, semi_persistent_r16_s_> c;

    void destroy_();
  };

  // member variables
  bool                                ext                                   = false;
  bool                                spatial_relation_info_pos_r16_present = false;
  uint8_t                             srs_pos_res_id_r16                    = 0;
  tx_comb_r16_c_                      tx_comb_r16;
  res_map_r16_s_                      res_map_r16;
  uint16_t                            freq_domain_shift_r16 = 0;
  freq_hop_r16_s_                     freq_hop_r16;
  group_or_seq_hop_r16_e_             group_or_seq_hop_r16;
  res_type_r16_c_                     res_type_r16;
  uint32_t                            seq_id_r16 = 0;
  srs_spatial_relation_info_pos_r16_c spatial_relation_info_pos_r16;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SRS-PosResourceSet-r16 ::= SEQUENCE
struct srs_pos_res_set_r16_s {
  using srs_pos_res_id_list_r16_l_ = bounded_array<uint8_t, 16>;
  struct res_type_r16_c_ {
    struct aperiodic_r16_s_ {
      using aperiodic_srs_res_trigger_list_r16_l_ = bounded_array<uint8_t, 3>;

      // member variables
      bool                                  ext = false;
      aperiodic_srs_res_trigger_list_r16_l_ aperiodic_srs_res_trigger_list_r16;
      // ...
    };
    struct semi_persistent_r16_s_ {
      bool ext = false;
      // ...
    };
    struct periodic_r16_s_ {
      bool ext = false;
      // ...
    };
    struct types_opts {
      enum options { aperiodic_r16, semi_persistent_r16, periodic_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    res_type_r16_c_() = default;
    res_type_r16_c_(const res_type_r16_c_& other);
    res_type_r16_c_& operator=(const res_type_r16_c_& other);
    ~res_type_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    aperiodic_r16_s_& aperiodic_r16()
    {
      assert_choice_type(types::aperiodic_r16, type_, "resourceType-r16");
      return c.get<aperiodic_r16_s_>();
    }
    semi_persistent_r16_s_& semi_persistent_r16()
    {
      assert_choice_type(types::semi_persistent_r16, type_, "resourceType-r16");
      return c.get<semi_persistent_r16_s_>();
    }
    periodic_r16_s_& periodic_r16()
    {
      assert_choice_type(types::periodic_r16, type_, "resourceType-r16");
      return c.get<periodic_r16_s_>();
    }
    const aperiodic_r16_s_& aperiodic_r16() const
    {
      assert_choice_type(types::aperiodic_r16, type_, "resourceType-r16");
      return c.get<aperiodic_r16_s_>();
    }
    const semi_persistent_r16_s_& semi_persistent_r16() const
    {
      assert_choice_type(types::semi_persistent_r16, type_, "resourceType-r16");
      return c.get<semi_persistent_r16_s_>();
    }
    const periodic_r16_s_& periodic_r16() const
    {
      assert_choice_type(types::periodic_r16, type_, "resourceType-r16");
      return c.get<periodic_r16_s_>();
    }
    aperiodic_r16_s_&       set_aperiodic_r16();
    semi_persistent_r16_s_& set_semi_persistent_r16();
    periodic_r16_s_&        set_periodic_r16();

  private:
    types                                                                      type_;
    choice_buffer_t<aperiodic_r16_s_, periodic_r16_s_, semi_persistent_r16_s_> c;

    void destroy_();
  };
  struct pathloss_ref_rs_pos_r16_c_ {
    struct types_opts {
      enum options { ssb_idx_serving_r16, ssb_ncell_r16, dl_prs_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    pathloss_ref_rs_pos_r16_c_() = default;
    pathloss_ref_rs_pos_r16_c_(const pathloss_ref_rs_pos_r16_c_& other);
    pathloss_ref_rs_pos_r16_c_& operator=(const pathloss_ref_rs_pos_r16_c_& other);
    ~pathloss_ref_rs_pos_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx_serving_r16()
    {
      assert_choice_type(types::ssb_idx_serving_r16, type_, "pathlossReferenceRS-Pos-r16");
      return c.get<uint8_t>();
    }
    ssb_info_ncell_r16_s& ssb_ncell_r16()
    {
      assert_choice_type(types::ssb_ncell_r16, type_, "pathlossReferenceRS-Pos-r16");
      return c.get<ssb_info_ncell_r16_s>();
    }
    dl_prs_info_r16_s& dl_prs_r16()
    {
      assert_choice_type(types::dl_prs_r16, type_, "pathlossReferenceRS-Pos-r16");
      return c.get<dl_prs_info_r16_s>();
    }
    const uint8_t& ssb_idx_serving_r16() const
    {
      assert_choice_type(types::ssb_idx_serving_r16, type_, "pathlossReferenceRS-Pos-r16");
      return c.get<uint8_t>();
    }
    const ssb_info_ncell_r16_s& ssb_ncell_r16() const
    {
      assert_choice_type(types::ssb_ncell_r16, type_, "pathlossReferenceRS-Pos-r16");
      return c.get<ssb_info_ncell_r16_s>();
    }
    const dl_prs_info_r16_s& dl_prs_r16() const
    {
      assert_choice_type(types::dl_prs_r16, type_, "pathlossReferenceRS-Pos-r16");
      return c.get<dl_prs_info_r16_s>();
    }
    uint8_t&              set_ssb_idx_serving_r16();
    ssb_info_ncell_r16_s& set_ssb_ncell_r16();
    dl_prs_info_r16_s&    set_dl_prs_r16();

  private:
    types                                                    type_;
    choice_buffer_t<dl_prs_info_r16_s, ssb_info_ncell_r16_s> c;

    void destroy_();
  };

  // member variables
  bool                       ext                             = false;
  bool                       alpha_r16_present               = false;
  bool                       p0_r16_present                  = false;
  bool                       pathloss_ref_rs_pos_r16_present = false;
  uint8_t                    srs_pos_res_set_id_r16          = 0;
  srs_pos_res_id_list_r16_l_ srs_pos_res_id_list_r16;
  res_type_r16_c_            res_type_r16;
  alpha_e                    alpha_r16;
  int16_t                    p0_r16 = -202;
  pathloss_ref_rs_pos_r16_c_ pathloss_ref_rs_pos_r16;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SRS-ResourceSet ::= SEQUENCE
struct srs_res_set_s {
  using srs_res_id_list_l_ = bounded_array<uint8_t, 16>;
  struct res_type_c_ {
    struct aperiodic_s_ {
      using aperiodic_srs_res_trigger_list_l_ = bounded_array<uint8_t, 2>;

      // member variables
      bool    ext                       = false;
      bool    csi_rs_present            = false;
      bool    slot_offset_present       = false;
      uint8_t aperiodic_srs_res_trigger = 1;
      uint8_t csi_rs                    = 0;
      uint8_t slot_offset               = 1;
      // ...
      // group 0
      copy_ptr<aperiodic_srs_res_trigger_list_l_> aperiodic_srs_res_trigger_list;

      // sequence methods
      SRSASN_CODE pack(bit_ref& bref) const;
      SRSASN_CODE unpack(cbit_ref& bref);
      void        to_json(json_writer& j) const;
    };
    struct semi_persistent_s_ {
      bool    ext                       = false;
      bool    associated_csi_rs_present = false;
      uint8_t associated_csi_rs         = 0;
      // ...
    };
    struct periodic_s_ {
      bool    ext                       = false;
      bool    associated_csi_rs_present = false;
      uint8_t associated_csi_rs         = 0;
      // ...
    };
    struct types_opts {
      enum options { aperiodic, semi_persistent, periodic, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    res_type_c_() = default;
    res_type_c_(const res_type_c_& other);
    res_type_c_& operator=(const res_type_c_& other);
    ~res_type_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    aperiodic_s_& aperiodic()
    {
      assert_choice_type(types::aperiodic, type_, "resourceType");
      return c.get<aperiodic_s_>();
    }
    semi_persistent_s_& semi_persistent()
    {
      assert_choice_type(types::semi_persistent, type_, "resourceType");
      return c.get<semi_persistent_s_>();
    }
    periodic_s_& periodic()
    {
      assert_choice_type(types::periodic, type_, "resourceType");
      return c.get<periodic_s_>();
    }
    const aperiodic_s_& aperiodic() const
    {
      assert_choice_type(types::aperiodic, type_, "resourceType");
      return c.get<aperiodic_s_>();
    }
    const semi_persistent_s_& semi_persistent() const
    {
      assert_choice_type(types::semi_persistent, type_, "resourceType");
      return c.get<semi_persistent_s_>();
    }
    const periodic_s_& periodic() const
    {
      assert_choice_type(types::periodic, type_, "resourceType");
      return c.get<periodic_s_>();
    }
    aperiodic_s_&       set_aperiodic();
    semi_persistent_s_& set_semi_persistent();
    periodic_s_&        set_periodic();

  private:
    types                                                          type_;
    choice_buffer_t<aperiodic_s_, periodic_s_, semi_persistent_s_> c;

    void destroy_();
  };
  struct usage_opts {
    enum options { beam_management, codebook, non_codebook, ant_switching, nulltype } value;

    const char* to_string() const;
  };
  using usage_e_ = enumerated<usage_opts>;
  struct srs_pwr_ctrl_adjustment_states_opts {
    enum options { same_as_fci2, separate_closed_loop, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using srs_pwr_ctrl_adjustment_states_e_ = enumerated<srs_pwr_ctrl_adjustment_states_opts>;
  using available_slot_offset_list_r17_l_ = bounded_array<uint8_t, 4>;

  // member variables
  bool                              ext                                    = false;
  bool                              alpha_present                          = false;
  bool                              p0_present                             = false;
  bool                              pathloss_ref_rs_present                = false;
  bool                              srs_pwr_ctrl_adjustment_states_present = false;
  uint8_t                           srs_res_set_id                         = 0;
  srs_res_id_list_l_                srs_res_id_list;
  res_type_c_                       res_type;
  usage_e_                          usage;
  alpha_e                           alpha;
  int16_t                           p0 = -202;
  pathloss_ref_rs_cfg_c             pathloss_ref_rs;
  srs_pwr_ctrl_adjustment_states_e_ srs_pwr_ctrl_adjustment_states;
  // ...
  // group 0
  copy_ptr<setup_release_c<dyn_seq_of<pathloss_ref_rs_r16_s, 1, 64>>> pathloss_ref_rs_list_r16;
  // group 1
  bool                                        usage_pdc_r17_present                    = false;
  bool                                        follow_unified_tci_state_srs_r17_present = false;
  copy_ptr<available_slot_offset_list_r17_l_> available_slot_offset_list_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// TCI-State ::= SEQUENCE
struct tci_state_s {
  bool       ext               = false;
  bool       qcl_type2_present = false;
  uint8_t    tci_state_id      = 0;
  qcl_info_s qcl_type1;
  qcl_info_s qcl_type2;
  // ...
  // group 0
  bool    add_pci_r17_present            = false;
  bool    pathloss_ref_rs_id_r17_present = false;
  bool    ul_pwr_ctrl_r17_present        = false;
  uint8_t add_pci_r17                    = 1;
  uint8_t pathloss_ref_rs_id_r17         = 0;
  uint8_t ul_pwr_ctrl_r17                = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// ZP-CSI-RS-Resource ::= SEQUENCE
struct zp_csi_rs_res_s {
  bool                             ext                            = false;
  bool                             periodicity_and_offset_present = false;
  uint8_t                          zp_csi_rs_res_id               = 0;
  csi_rs_res_map_s                 res_map;
  csi_res_periodicity_and_offset_c periodicity_and_offset;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// AdditionalRACH-ConfigList-r17 ::= SEQUENCE (SIZE (1..256)) OF AdditionalRACH-Config-r17
using add_rach_cfg_list_r17_l = dyn_array<add_rach_cfg_r17_s>;

// BeamFailureDetection-r17 ::= SEQUENCE
struct beam_fail_detection_r17_s {
  bool                          fail_detection_set1_r17_present = false;
  bool                          fail_detection_set2_r17_present = false;
  bool                          add_pci_r17_present             = false;
  beam_fail_detection_set_r17_s fail_detection_set1_r17;
  beam_fail_detection_set_r17_s fail_detection_set2_r17;
  uint8_t                       add_pci_r17 = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BeamFailureRecoveryConfig ::= SEQUENCE
struct beam_fail_recovery_cfg_s {
  using candidate_beam_rs_list_l_ = dyn_array<prach_res_ded_bfr_c>;
  struct ssb_per_rach_occasion_opts {
    enum options { one_eighth, one_fourth, one_half, one, two, four, eight, sixteen, nulltype } value;
    typedef float number_type;

    const char* to_string() const;
    float       to_number() const;
    const char* to_number_string() const;
  };
  using ssb_per_rach_occasion_e_ = enumerated<ssb_per_rach_occasion_opts>;
  struct beam_fail_recovery_timer_opts {
    enum options { ms10, ms20, ms40, ms60, ms80, ms100, ms150, ms200, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using beam_fail_recovery_timer_e_ = enumerated<beam_fail_recovery_timer_opts>;

  // member variables
  bool                        ext                              = false;
  bool                        root_seq_idx_bfr_present         = false;
  bool                        rach_cfg_bfr_present             = false;
  bool                        rsrp_thres_ssb_present           = false;
  bool                        ssb_per_rach_occasion_present    = false;
  bool                        ra_ssb_occasion_mask_idx_present = false;
  bool                        recovery_search_space_id_present = false;
  bool                        ra_prioritization_present        = false;
  bool                        beam_fail_recovery_timer_present = false;
  uint8_t                     root_seq_idx_bfr                 = 0;
  rach_cfg_generic_s          rach_cfg_bfr;
  uint8_t                     rsrp_thres_ssb = 0;
  candidate_beam_rs_list_l_   candidate_beam_rs_list;
  ssb_per_rach_occasion_e_    ssb_per_rach_occasion;
  uint8_t                     ra_ssb_occasion_mask_idx = 0;
  uint8_t                     recovery_search_space_id = 0;
  ra_prioritization_s         ra_prioritization;
  beam_fail_recovery_timer_e_ beam_fail_recovery_timer;
  // ...
  // group 0
  bool                 msg1_subcarrier_spacing_present = false;
  subcarrier_spacing_e msg1_subcarrier_spacing;
  // group 1
  copy_ptr<ra_prioritization_s>                                     ra_prioritization_two_step_r16;
  copy_ptr<setup_release_c<dyn_seq_of<prach_res_ded_bfr_c, 1, 48>>> candidate_beam_rs_list_ext_v1610;
  // group 2
  bool sp_cell_bfr_cbra_r16_present = false;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// CandidateBeamRS-r16 ::= SEQUENCE
struct candidate_beam_rs_r16_s {
  struct candidate_beam_cfg_r16_c_ {
    struct types_opts {
      enum options { ssb_r16, csi_rs_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    candidate_beam_cfg_r16_c_() = default;
    candidate_beam_cfg_r16_c_(const candidate_beam_cfg_r16_c_& other);
    candidate_beam_cfg_r16_c_& operator=(const candidate_beam_cfg_r16_c_& other);
    ~candidate_beam_cfg_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_r16()
    {
      assert_choice_type(types::ssb_r16, type_, "candidateBeamConfig-r16");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_r16()
    {
      assert_choice_type(types::csi_rs_r16, type_, "candidateBeamConfig-r16");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_r16() const
    {
      assert_choice_type(types::ssb_r16, type_, "candidateBeamConfig-r16");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_r16() const
    {
      assert_choice_type(types::csi_rs_r16, type_, "candidateBeamConfig-r16");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_r16();
    uint8_t& set_csi_rs_r16();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  bool                      serving_cell_id_present = false;
  candidate_beam_cfg_r16_c_ candidate_beam_cfg_r16;
  uint8_t                   serving_cell_id = 0;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// ConfiguredGrantConfig ::= SEQUENCE
struct cfg_grant_cfg_s {
  struct freq_hop_opts {
    enum options { intra_slot, inter_slot, nulltype } value;

    const char* to_string() const;
  };
  using freq_hop_e_ = enumerated<freq_hop_opts>;
  struct mcs_table_opts {
    enum options { qam256, qam64_low_se, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using mcs_table_e_ = enumerated<mcs_table_opts>;
  struct mcs_table_transform_precoder_opts {
    enum options { qam256, qam64_low_se, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using mcs_table_transform_precoder_e_ = enumerated<mcs_table_transform_precoder_opts>;
  struct res_alloc_opts {
    enum options { res_alloc_type0, res_alloc_type1, dyn_switch, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using res_alloc_e_ = enumerated<res_alloc_opts>;
  struct pwr_ctrl_loop_to_use_opts {
    enum options { n0, n1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pwr_ctrl_loop_to_use_e_ = enumerated<pwr_ctrl_loop_to_use_opts>;
  struct transform_precoder_opts {
    enum options { enabled, disabled, nulltype } value;

    const char* to_string() const;
  };
  using transform_precoder_e_ = enumerated<transform_precoder_opts>;
  struct rep_k_opts {
    enum options { n1, n2, n4, n8, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using rep_k_e_ = enumerated<rep_k_opts>;
  struct rep_k_rv_opts {
    enum options { s1_neg0231, s2_neg0303, s3_neg0000, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using rep_k_rv_e_ = enumerated<rep_k_rv_opts>;
  struct periodicity_opts {
    enum options {
      sym2,
      sym7,
      sym1x14,
      sym2x14,
      sym4x14,
      sym5x14,
      sym8x14,
      sym10x14,
      sym16x14,
      sym20x14,
      sym32x14,
      sym40x14,
      sym64x14,
      sym80x14,
      sym128x14,
      sym160x14,
      sym256x14,
      sym320x14,
      sym512x14,
      sym640x14,
      sym1024x14,
      sym1280x14,
      sym2560x14,
      sym5120x14,
      sym6,
      sym1x12,
      sym2x12,
      sym4x12,
      sym5x12,
      sym8x12,
      sym10x12,
      sym16x12,
      sym20x12,
      sym32x12,
      sym40x12,
      sym64x12,
      sym80x12,
      sym128x12,
      sym160x12,
      sym256x12,
      sym320x12,
      sym512x12,
      sym640x12,
      sym1280x12,
      sym2560x12,
      nulltype
    } value;

    const char* to_string() const;
  };
  using periodicity_e_ = enumerated<periodicity_opts>;
  struct rrc_cfg_ul_grant_s_ {
    struct pusch_rep_type_ind_r16_opts {
      enum options { pusch_rep_type_a, pusch_rep_type_b, nulltype } value;

      const char* to_string() const;
    };
    using pusch_rep_type_ind_r16_e_ = enumerated<pusch_rep_type_ind_r16_opts>;
    struct freq_hop_pusch_rep_type_b_r16_opts {
      enum options { inter_repeat, inter_slot, nulltype } value;

      const char* to_string() const;
    };
    using freq_hop_pusch_rep_type_b_r16_e_ = enumerated<freq_hop_pusch_rep_type_b_r16_opts>;

    // member variables
    bool                ext                          = false;
    bool                dmrs_seq_initization_present = false;
    bool                srs_res_ind_present          = false;
    bool                freq_hop_offset_present      = false;
    uint16_t            time_domain_offset           = 0;
    uint8_t             time_domain_alloc            = 0;
    fixed_bitstring<18> freq_domain_alloc;
    uint8_t             ant_port                 = 0;
    uint8_t             dmrs_seq_initization     = 0;
    uint8_t             precoding_and_nof_layers = 0;
    uint8_t             srs_res_ind              = 0;
    uint8_t             mcs_and_tbs              = 0;
    uint16_t            freq_hop_offset          = 1;
    uint8_t             pathloss_ref_idx         = 0;
    // ...
    // group 0
    bool                             pusch_rep_type_ind_r16_present        = false;
    bool                             freq_hop_pusch_rep_type_b_r16_present = false;
    bool                             time_ref_sfn_r16_present              = false;
    pusch_rep_type_ind_r16_e_        pusch_rep_type_ind_r16;
    freq_hop_pusch_rep_type_b_r16_e_ freq_hop_pusch_rep_type_b_r16;
    // group 1
    bool                       pathloss_ref_idx2_r17_present         = false;
    bool                       srs_res_ind2_r17_present              = false;
    bool                       precoding_and_nof_layers2_r17_present = false;
    bool                       time_domain_alloc_v1710_present       = false;
    bool                       time_domain_offset_r17_present        = false;
    uint8_t                    pathloss_ref_idx2_r17                 = 0;
    uint8_t                    srs_res_ind2_r17                      = 0;
    uint8_t                    precoding_and_nof_layers2_r17         = 0;
    uint8_t                    time_domain_alloc_v1710               = 16;
    uint16_t                   time_domain_offset_r17                = 0;
    copy_ptr<cg_sdt_cfg_r17_s> cg_sdt_cfg_r17;

    // sequence methods
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
  };
  struct cg_min_dfi_delay_r16_opts {
    enum options {
      sym7,
      sym1x14,
      sym2x14,
      sym3x14,
      sym4x14,
      sym5x14,
      sym6x14,
      sym7x14,
      sym8x14,
      sym9x14,
      sym10x14,
      sym11x14,
      sym12x14,
      sym13x14,
      sym14x14,
      sym15x14,
      sym16x14,
      nulltype
    } value;

    const char* to_string() const;
  };
  using cg_min_dfi_delay_r16_e_    = enumerated<cg_min_dfi_delay_r16_opts>;
  using cg_cot_sharing_list_r16_l_ = dyn_array<cg_cot_sharing_r16_c>;
  struct start_from_rv0_r16_opts {
    enum options { on, off, nulltype } value;

    const char* to_string() const;
  };
  using start_from_rv0_r16_e_ = enumerated<start_from_rv0_r16_opts>;
  struct phy_prio_idx_r16_opts {
    enum options { p0, p1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using phy_prio_idx_r16_e_ = enumerated<phy_prio_idx_r16_opts>;
  struct map_pattern_r17_opts {
    enum options { cyclic_map, sequential_map, nulltype } value;

    const char* to_string() const;
  };
  using map_pattern_r17_e_ = enumerated<map_pattern_r17_opts>;
  struct pwr_ctrl_loop_to_use2_r17_opts {
    enum options { n0, n1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pwr_ctrl_loop_to_use2_r17_e_ = enumerated<pwr_ctrl_loop_to_use2_r17_opts>;
  using cg_cot_sharing_list_r17_l_   = dyn_array<cg_cot_sharing_r17_c>;
  struct rep_k_v1710_opts {
    enum options { n12, n16, n24, n32, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using rep_k_v1710_e_ = enumerated<rep_k_v1710_opts>;

  // member variables
  bool                               ext                                  = false;
  bool                               freq_hop_present                     = false;
  bool                               mcs_table_present                    = false;
  bool                               mcs_table_transform_precoder_present = false;
  bool                               uci_on_pusch_present                 = false;
  bool                               rbg_size_present                     = false;
  bool                               transform_precoder_present           = false;
  bool                               rep_k_rv_present                     = false;
  bool                               cfg_grant_timer_present              = false;
  bool                               rrc_cfg_ul_grant_present             = false;
  freq_hop_e_                        freq_hop;
  dmrs_ul_cfg_s                      cg_dmrs_cfg;
  mcs_table_e_                       mcs_table;
  mcs_table_transform_precoder_e_    mcs_table_transform_precoder;
  setup_release_c<cg_uci_on_pusch_c> uci_on_pusch;
  res_alloc_e_                       res_alloc;
  pwr_ctrl_loop_to_use_e_            pwr_ctrl_loop_to_use;
  uint8_t                            p0_pusch_alpha = 0;
  transform_precoder_e_              transform_precoder;
  uint8_t                            nrof_harq_processes = 1;
  rep_k_e_                           rep_k;
  rep_k_rv_e_                        rep_k_rv;
  periodicity_e_                     periodicity;
  uint8_t                            cfg_grant_timer = 1;
  rrc_cfg_ul_grant_s_                rrc_cfg_ul_grant;
  // ...
  // group 0
  bool                                 cg_retx_timer_r16_present         = false;
  bool                                 cg_min_dfi_delay_r16_present      = false;
  bool                                 cg_nrof_pusch_in_slot_r16_present = false;
  bool                                 cg_nrof_slots_r16_present         = false;
  bool                                 cg_uci_mux_r16_present            = false;
  bool                                 cg_cot_sharing_offset_r16_present = false;
  bool                                 beta_offset_cg_uci_r16_present    = false;
  bool                                 harq_proc_id_offset_r16_present   = false;
  bool                                 harq_proc_id_offset2_r16_present  = false;
  bool                                 cfg_grant_cfg_idx_r16_present     = false;
  bool                                 cfg_grant_cfg_idx_mac_r16_present = false;
  bool                                 periodicity_ext_r16_present       = false;
  bool                                 start_from_rv0_r16_present        = false;
  bool                                 phy_prio_idx_r16_present          = false;
  bool                                 autonomous_tx_r16_present         = false;
  uint8_t                              cg_retx_timer_r16                 = 1;
  cg_min_dfi_delay_r16_e_              cg_min_dfi_delay_r16;
  uint8_t                              cg_nrof_pusch_in_slot_r16 = 1;
  uint8_t                              cg_nrof_slots_r16         = 1;
  copy_ptr<cg_start_offsets_r16_s>     cg_start_offsets_r16;
  uint8_t                              cg_cot_sharing_offset_r16 = 1;
  uint8_t                              beta_offset_cg_uci_r16    = 0;
  copy_ptr<cg_cot_sharing_list_r16_l_> cg_cot_sharing_list_r16;
  uint8_t                              harq_proc_id_offset_r16   = 0;
  uint8_t                              harq_proc_id_offset2_r16  = 0;
  uint8_t                              cfg_grant_cfg_idx_r16     = 0;
  uint8_t                              cfg_grant_cfg_idx_mac_r16 = 0;
  uint16_t                             periodicity_ext_r16       = 1;
  start_from_rv0_r16_e_                start_from_rv0_r16;
  phy_prio_idx_r16_e_                  phy_prio_idx_r16;
  // group 1
  bool                                                           map_pattern_r17_present            = false;
  bool                                                           seq_offset_for_rv_r17_present      = false;
  bool                                                           p0_pusch_alpha2_r17_present        = false;
  bool                                                           pwr_ctrl_loop_to_use2_r17_present  = false;
  bool                                                           periodicity_ext_r17_present        = false;
  bool                                                           rep_k_v1710_present                = false;
  bool                                                           nrof_harq_processes_v1700_present  = false;
  bool                                                           harq_proc_id_offset2_v1700_present = false;
  bool                                                           cfg_grant_timer_v1700_present      = false;
  bool                                                           cg_min_dfi_delay_v1710_present     = false;
  copy_ptr<setup_release_c<beta_offsets_cross_pri_sel_cg_r17_c>> cg_beta_offsets_cross_pri0_r17;
  copy_ptr<setup_release_c<beta_offsets_cross_pri_sel_cg_r17_c>> cg_beta_offsets_cross_pri1_r17;
  map_pattern_r17_e_                                             map_pattern_r17;
  uint8_t                                                        seq_offset_for_rv_r17 = 0;
  uint8_t                                                        p0_pusch_alpha2_r17   = 0;
  pwr_ctrl_loop_to_use2_r17_e_                                   pwr_ctrl_loop_to_use2_r17;
  copy_ptr<cg_cot_sharing_list_r17_l_>                           cg_cot_sharing_list_r17;
  uint16_t                                                       periodicity_ext_r17 = 1;
  rep_k_v1710_e_                                                 rep_k_v1710;
  uint8_t                                                        nrof_harq_processes_v1700  = 17;
  uint8_t                                                        harq_proc_id_offset2_v1700 = 16;
  uint16_t                                                       cfg_grant_timer_v1700      = 33;
  uint16_t                                                       cg_min_dfi_delay_v1710     = 238;
  // group 2
  bool     harq_proc_id_offset_v1730_present = false;
  bool     cg_nrof_slots_r17_present         = false;
  uint8_t  harq_proc_id_offset_v1730         = 16;
  uint16_t cg_nrof_slots_r17                 = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// ConfiguredGrantConfigType2DeactivationState-r16 ::= SEQUENCE (SIZE (1..12)) OF INTEGER (0..11)
using cfg_grant_cfg_type2_deactivation_state_r16_l = bounded_array<uint8_t, 12>;

// DL-PPW-PeriodicityAndStartSlot-r17 ::= CHOICE
struct dl_ppw_periodicity_and_start_slot_r17_c {
  struct scs15_c_ {
    struct types_opts {
      enum options {
        n4,
        n5,
        n8,
        n10,
        n16,
        n20,
        n32,
        n40,
        n64,
        n80,
        n160,
        n320,
        n640,
        n1280,
        n2560,
        n5120,
        n10240,
        // ...
        nulltype
      } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts, true>;

    // choice methods
    scs15_c_() = default;
    scs15_c_(const scs15_c_& other);
    scs15_c_& operator=(const scs15_c_& other);
    ~scs15_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& n4()
    {
      assert_choice_type(types::n4, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n5()
    {
      assert_choice_type(types::n5, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n8()
    {
      assert_choice_type(types::n8, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n10()
    {
      assert_choice_type(types::n10, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n16()
    {
      assert_choice_type(types::n16, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n20()
    {
      assert_choice_type(types::n20, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n32()
    {
      assert_choice_type(types::n32, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n40()
    {
      assert_choice_type(types::n40, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n64()
    {
      assert_choice_type(types::n64, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n80()
    {
      assert_choice_type(types::n80, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint8_t& n160()
    {
      assert_choice_type(types::n160, type_, "scs15");
      return c.get<uint8_t>();
    }
    uint16_t& n320()
    {
      assert_choice_type(types::n320, type_, "scs15");
      return c.get<uint16_t>();
    }
    uint16_t& n640()
    {
      assert_choice_type(types::n640, type_, "scs15");
      return c.get<uint16_t>();
    }
    uint16_t& n1280()
    {
      assert_choice_type(types::n1280, type_, "scs15");
      return c.get<uint16_t>();
    }
    uint16_t& n2560()
    {
      assert_choice_type(types::n2560, type_, "scs15");
      return c.get<uint16_t>();
    }
    uint16_t& n5120()
    {
      assert_choice_type(types::n5120, type_, "scs15");
      return c.get<uint16_t>();
    }
    uint16_t& n10240()
    {
      assert_choice_type(types::n10240, type_, "scs15");
      return c.get<uint16_t>();
    }
    const uint8_t& n4() const
    {
      assert_choice_type(types::n4, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n5() const
    {
      assert_choice_type(types::n5, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n8() const
    {
      assert_choice_type(types::n8, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n10() const
    {
      assert_choice_type(types::n10, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n16() const
    {
      assert_choice_type(types::n16, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n20() const
    {
      assert_choice_type(types::n20, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n32() const
    {
      assert_choice_type(types::n32, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n40() const
    {
      assert_choice_type(types::n40, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n64() const
    {
      assert_choice_type(types::n64, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n80() const
    {
      assert_choice_type(types::n80, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint8_t& n160() const
    {
      assert_choice_type(types::n160, type_, "scs15");
      return c.get<uint8_t>();
    }
    const uint16_t& n320() const
    {
      assert_choice_type(types::n320, type_, "scs15");
      return c.get<uint16_t>();
    }
    const uint16_t& n640() const
    {
      assert_choice_type(types::n640, type_, "scs15");
      return c.get<uint16_t>();
    }
    const uint16_t& n1280() const
    {
      assert_choice_type(types::n1280, type_, "scs15");
      return c.get<uint16_t>();
    }
    const uint16_t& n2560() const
    {
      assert_choice_type(types::n2560, type_, "scs15");
      return c.get<uint16_t>();
    }
    const uint16_t& n5120() const
    {
      assert_choice_type(types::n5120, type_, "scs15");
      return c.get<uint16_t>();
    }
    const uint16_t& n10240() const
    {
      assert_choice_type(types::n10240, type_, "scs15");
      return c.get<uint16_t>();
    }
    uint8_t&  set_n4();
    uint8_t&  set_n5();
    uint8_t&  set_n8();
    uint8_t&  set_n10();
    uint8_t&  set_n16();
    uint8_t&  set_n20();
    uint8_t&  set_n32();
    uint8_t&  set_n40();
    uint8_t&  set_n64();
    uint8_t&  set_n80();
    uint8_t&  set_n160();
    uint16_t& set_n320();
    uint16_t& set_n640();
    uint16_t& set_n1280();
    uint16_t& set_n2560();
    uint16_t& set_n5120();
    uint16_t& set_n10240();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct scs30_c_ {
    struct types_opts {
      enum options {
        n8,
        n10,
        n16,
        n20,
        n32,
        n40,
        n64,
        n80,
        n128,
        n160,
        n320,
        n640,
        n1280,
        n2560,
        n5120,
        n10240,
        n20480,
        // ...
        nulltype
      } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts, true>;

    // choice methods
    scs30_c_() = default;
    scs30_c_(const scs30_c_& other);
    scs30_c_& operator=(const scs30_c_& other);
    ~scs30_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& n8()
    {
      assert_choice_type(types::n8, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n10()
    {
      assert_choice_type(types::n10, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n16()
    {
      assert_choice_type(types::n16, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n20()
    {
      assert_choice_type(types::n20, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n32()
    {
      assert_choice_type(types::n32, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n40()
    {
      assert_choice_type(types::n40, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n64()
    {
      assert_choice_type(types::n64, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n80()
    {
      assert_choice_type(types::n80, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n128()
    {
      assert_choice_type(types::n128, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint8_t& n160()
    {
      assert_choice_type(types::n160, type_, "scs30");
      return c.get<uint8_t>();
    }
    uint16_t& n320()
    {
      assert_choice_type(types::n320, type_, "scs30");
      return c.get<uint16_t>();
    }
    uint16_t& n640()
    {
      assert_choice_type(types::n640, type_, "scs30");
      return c.get<uint16_t>();
    }
    uint16_t& n1280()
    {
      assert_choice_type(types::n1280, type_, "scs30");
      return c.get<uint16_t>();
    }
    uint16_t& n2560()
    {
      assert_choice_type(types::n2560, type_, "scs30");
      return c.get<uint16_t>();
    }
    uint16_t& n5120()
    {
      assert_choice_type(types::n5120, type_, "scs30");
      return c.get<uint16_t>();
    }
    uint16_t& n10240()
    {
      assert_choice_type(types::n10240, type_, "scs30");
      return c.get<uint16_t>();
    }
    uint16_t& n20480()
    {
      assert_choice_type(types::n20480, type_, "scs30");
      return c.get<uint16_t>();
    }
    const uint8_t& n8() const
    {
      assert_choice_type(types::n8, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n10() const
    {
      assert_choice_type(types::n10, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n16() const
    {
      assert_choice_type(types::n16, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n20() const
    {
      assert_choice_type(types::n20, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n32() const
    {
      assert_choice_type(types::n32, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n40() const
    {
      assert_choice_type(types::n40, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n64() const
    {
      assert_choice_type(types::n64, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n80() const
    {
      assert_choice_type(types::n80, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n128() const
    {
      assert_choice_type(types::n128, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint8_t& n160() const
    {
      assert_choice_type(types::n160, type_, "scs30");
      return c.get<uint8_t>();
    }
    const uint16_t& n320() const
    {
      assert_choice_type(types::n320, type_, "scs30");
      return c.get<uint16_t>();
    }
    const uint16_t& n640() const
    {
      assert_choice_type(types::n640, type_, "scs30");
      return c.get<uint16_t>();
    }
    const uint16_t& n1280() const
    {
      assert_choice_type(types::n1280, type_, "scs30");
      return c.get<uint16_t>();
    }
    const uint16_t& n2560() const
    {
      assert_choice_type(types::n2560, type_, "scs30");
      return c.get<uint16_t>();
    }
    const uint16_t& n5120() const
    {
      assert_choice_type(types::n5120, type_, "scs30");
      return c.get<uint16_t>();
    }
    const uint16_t& n10240() const
    {
      assert_choice_type(types::n10240, type_, "scs30");
      return c.get<uint16_t>();
    }
    const uint16_t& n20480() const
    {
      assert_choice_type(types::n20480, type_, "scs30");
      return c.get<uint16_t>();
    }
    uint8_t&  set_n8();
    uint8_t&  set_n10();
    uint8_t&  set_n16();
    uint8_t&  set_n20();
    uint8_t&  set_n32();
    uint8_t&  set_n40();
    uint8_t&  set_n64();
    uint8_t&  set_n80();
    uint8_t&  set_n128();
    uint8_t&  set_n160();
    uint16_t& set_n320();
    uint16_t& set_n640();
    uint16_t& set_n1280();
    uint16_t& set_n2560();
    uint16_t& set_n5120();
    uint16_t& set_n10240();
    uint16_t& set_n20480();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct scs60_c_ {
    struct types_opts {
      enum options {
        n16,
        n20,
        n32,
        n40,
        n64,
        n80,
        n128,
        n160,
        n256,
        n320,
        n640,
        n1280,
        n2560,
        n5120,
        n10240,
        n20480,
        n40960,
        // ...
        nulltype
      } value;
      typedef uint16_t number_type;

      const char* to_string() const;
      uint16_t    to_number() const;
    };
    using types = enumerated<types_opts, true>;

    // choice methods
    scs60_c_() = default;
    scs60_c_(const scs60_c_& other);
    scs60_c_& operator=(const scs60_c_& other);
    ~scs60_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& n16()
    {
      assert_choice_type(types::n16, type_, "scs60");
      return c.get<uint8_t>();
    }
    uint8_t& n20()
    {
      assert_choice_type(types::n20, type_, "scs60");
      return c.get<uint8_t>();
    }
    uint8_t& n32()
    {
      assert_choice_type(types::n32, type_, "scs60");
      return c.get<uint8_t>();
    }
    uint8_t& n40()
    {
      assert_choice_type(types::n40, type_, "scs60");
      return c.get<uint8_t>();
    }
    uint8_t& n64()
    {
      assert_choice_type(types::n64, type_, "scs60");
      return c.get<uint8_t>();
    }
    uint8_t& n80()
    {
      assert_choice_type(types::n80, type_, "scs60");
      return c.get<uint8_t>();
    }
    uint8_t& n128()
    {
      assert_choice_type(types::n128, type_, "scs60");
      return c.get<uint8_t>();
    }
    uint8_t& n160()
    {
      assert_choice_type(types::n160, type_, "scs60");
      return c.get<uint8_t>();
    }
    uint16_t& n256()
    {
      assert_choice_type(types::n256, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint16_t& n320()
    {
      assert_choice_type(types::n320, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint16_t& n640()
    {
      assert_choice_type(types::n640, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint16_t& n1280()
    {
      assert_choice_type(types::n1280, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint16_t& n2560()
    {
      assert_choice_type(types::n2560, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint16_t& n5120()
    {
      assert_choice_type(types::n5120, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint16_t& n10240()
    {
      assert_choice_type(types::n10240, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint16_t& n20480()
    {
      assert_choice_type(types::n20480, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint16_t& n40960()
    {
      assert_choice_type(types::n40960, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint8_t& n16() const
    {
      assert_choice_type(types::n16, type_, "scs60");
      return c.get<uint8_t>();
    }
    const uint8_t& n20() const
    {
      assert_choice_type(types::n20, type_, "scs60");
      return c.get<uint8_t>();
    }
    const uint8_t& n32() const
    {
      assert_choice_type(types::n32, type_, "scs60");
      return c.get<uint8_t>();
    }
    const uint8_t& n40() const
    {
      assert_choice_type(types::n40, type_, "scs60");
      return c.get<uint8_t>();
    }
    const uint8_t& n64() const
    {
      assert_choice_type(types::n64, type_, "scs60");
      return c.get<uint8_t>();
    }
    const uint8_t& n80() const
    {
      assert_choice_type(types::n80, type_, "scs60");
      return c.get<uint8_t>();
    }
    const uint8_t& n128() const
    {
      assert_choice_type(types::n128, type_, "scs60");
      return c.get<uint8_t>();
    }
    const uint8_t& n160() const
    {
      assert_choice_type(types::n160, type_, "scs60");
      return c.get<uint8_t>();
    }
    const uint16_t& n256() const
    {
      assert_choice_type(types::n256, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint16_t& n320() const
    {
      assert_choice_type(types::n320, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint16_t& n640() const
    {
      assert_choice_type(types::n640, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint16_t& n1280() const
    {
      assert_choice_type(types::n1280, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint16_t& n2560() const
    {
      assert_choice_type(types::n2560, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint16_t& n5120() const
    {
      assert_choice_type(types::n5120, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint16_t& n10240() const
    {
      assert_choice_type(types::n10240, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint16_t& n20480() const
    {
      assert_choice_type(types::n20480, type_, "scs60");
      return c.get<uint16_t>();
    }
    const uint16_t& n40960() const
    {
      assert_choice_type(types::n40960, type_, "scs60");
      return c.get<uint16_t>();
    }
    uint8_t&  set_n16();
    uint8_t&  set_n20();
    uint8_t&  set_n32();
    uint8_t&  set_n40();
    uint8_t&  set_n64();
    uint8_t&  set_n80();
    uint8_t&  set_n128();
    uint8_t&  set_n160();
    uint16_t& set_n256();
    uint16_t& set_n320();
    uint16_t& set_n640();
    uint16_t& set_n1280();
    uint16_t& set_n2560();
    uint16_t& set_n5120();
    uint16_t& set_n10240();
    uint16_t& set_n20480();
    uint16_t& set_n40960();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct scs120_c_ {
    struct types_opts {
      enum options {
        n32,
        n40,
        n64,
        n80,
        n128,
        n160,
        n256,
        n320,
        n512,
        n640,
        n1280,
        n2560,
        n5120,
        n10240,
        n20480,
        n40960,
        n81920,
        // ...
        nulltype
      } value;
      typedef uint32_t number_type;

      const char* to_string() const;
      uint32_t    to_number() const;
    };
    using types = enumerated<types_opts, true>;

    // choice methods
    scs120_c_() = default;
    scs120_c_(const scs120_c_& other);
    scs120_c_& operator=(const scs120_c_& other);
    ~scs120_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& n32()
    {
      assert_choice_type(types::n32, type_, "scs120");
      return c.get<uint8_t>();
    }
    uint8_t& n40()
    {
      assert_choice_type(types::n40, type_, "scs120");
      return c.get<uint8_t>();
    }
    uint8_t& n64()
    {
      assert_choice_type(types::n64, type_, "scs120");
      return c.get<uint8_t>();
    }
    uint8_t& n80()
    {
      assert_choice_type(types::n80, type_, "scs120");
      return c.get<uint8_t>();
    }
    uint8_t& n128()
    {
      assert_choice_type(types::n128, type_, "scs120");
      return c.get<uint8_t>();
    }
    uint8_t& n160()
    {
      assert_choice_type(types::n160, type_, "scs120");
      return c.get<uint8_t>();
    }
    uint16_t& n256()
    {
      assert_choice_type(types::n256, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n320()
    {
      assert_choice_type(types::n320, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n512()
    {
      assert_choice_type(types::n512, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n640()
    {
      assert_choice_type(types::n640, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n1280()
    {
      assert_choice_type(types::n1280, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n2560()
    {
      assert_choice_type(types::n2560, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n5120()
    {
      assert_choice_type(types::n5120, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n10240()
    {
      assert_choice_type(types::n10240, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n20480()
    {
      assert_choice_type(types::n20480, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint16_t& n40960()
    {
      assert_choice_type(types::n40960, type_, "scs120");
      return c.get<uint16_t>();
    }
    uint32_t& n81920()
    {
      assert_choice_type(types::n81920, type_, "scs120");
      return c.get<uint32_t>();
    }
    const uint8_t& n32() const
    {
      assert_choice_type(types::n32, type_, "scs120");
      return c.get<uint8_t>();
    }
    const uint8_t& n40() const
    {
      assert_choice_type(types::n40, type_, "scs120");
      return c.get<uint8_t>();
    }
    const uint8_t& n64() const
    {
      assert_choice_type(types::n64, type_, "scs120");
      return c.get<uint8_t>();
    }
    const uint8_t& n80() const
    {
      assert_choice_type(types::n80, type_, "scs120");
      return c.get<uint8_t>();
    }
    const uint8_t& n128() const
    {
      assert_choice_type(types::n128, type_, "scs120");
      return c.get<uint8_t>();
    }
    const uint8_t& n160() const
    {
      assert_choice_type(types::n160, type_, "scs120");
      return c.get<uint8_t>();
    }
    const uint16_t& n256() const
    {
      assert_choice_type(types::n256, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n320() const
    {
      assert_choice_type(types::n320, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n512() const
    {
      assert_choice_type(types::n512, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n640() const
    {
      assert_choice_type(types::n640, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n1280() const
    {
      assert_choice_type(types::n1280, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n2560() const
    {
      assert_choice_type(types::n2560, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n5120() const
    {
      assert_choice_type(types::n5120, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n10240() const
    {
      assert_choice_type(types::n10240, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n20480() const
    {
      assert_choice_type(types::n20480, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint16_t& n40960() const
    {
      assert_choice_type(types::n40960, type_, "scs120");
      return c.get<uint16_t>();
    }
    const uint32_t& n81920() const
    {
      assert_choice_type(types::n81920, type_, "scs120");
      return c.get<uint32_t>();
    }
    uint8_t&  set_n32();
    uint8_t&  set_n40();
    uint8_t&  set_n64();
    uint8_t&  set_n80();
    uint8_t&  set_n128();
    uint8_t&  set_n160();
    uint16_t& set_n256();
    uint16_t& set_n320();
    uint16_t& set_n512();
    uint16_t& set_n640();
    uint16_t& set_n1280();
    uint16_t& set_n2560();
    uint16_t& set_n5120();
    uint16_t& set_n10240();
    uint16_t& set_n20480();
    uint16_t& set_n40960();
    uint32_t& set_n81920();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct types_opts {
    enum options { scs15, scs30, scs60, scs120, /*...*/ nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using types = enumerated<types_opts, true>;

  // choice methods
  dl_ppw_periodicity_and_start_slot_r17_c() = default;
  dl_ppw_periodicity_and_start_slot_r17_c(const dl_ppw_periodicity_and_start_slot_r17_c& other);
  dl_ppw_periodicity_and_start_slot_r17_c& operator=(const dl_ppw_periodicity_and_start_slot_r17_c& other);
  ~dl_ppw_periodicity_and_start_slot_r17_c() { destroy_(); }
  void        set(types::options e = types::nulltype);
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
  // getters
  scs15_c_& scs15()
  {
    assert_choice_type(types::scs15, type_, "DL-PPW-PeriodicityAndStartSlot-r17");
    return c.get<scs15_c_>();
  }
  scs30_c_& scs30()
  {
    assert_choice_type(types::scs30, type_, "DL-PPW-PeriodicityAndStartSlot-r17");
    return c.get<scs30_c_>();
  }
  scs60_c_& scs60()
  {
    assert_choice_type(types::scs60, type_, "DL-PPW-PeriodicityAndStartSlot-r17");
    return c.get<scs60_c_>();
  }
  scs120_c_& scs120()
  {
    assert_choice_type(types::scs120, type_, "DL-PPW-PeriodicityAndStartSlot-r17");
    return c.get<scs120_c_>();
  }
  const scs15_c_& scs15() const
  {
    assert_choice_type(types::scs15, type_, "DL-PPW-PeriodicityAndStartSlot-r17");
    return c.get<scs15_c_>();
  }
  const scs30_c_& scs30() const
  {
    assert_choice_type(types::scs30, type_, "DL-PPW-PeriodicityAndStartSlot-r17");
    return c.get<scs30_c_>();
  }
  const scs60_c_& scs60() const
  {
    assert_choice_type(types::scs60, type_, "DL-PPW-PeriodicityAndStartSlot-r17");
    return c.get<scs60_c_>();
  }
  const scs120_c_& scs120() const
  {
    assert_choice_type(types::scs120, type_, "DL-PPW-PeriodicityAndStartSlot-r17");
    return c.get<scs120_c_>();
  }
  scs15_c_&  set_scs15();
  scs30_c_&  set_scs30();
  scs60_c_&  set_scs60();
  scs120_c_& set_scs120();

private:
  types                                                    type_;
  choice_buffer_t<scs120_c_, scs15_c_, scs30_c_, scs60_c_> c;

  void destroy_();
};

// LBT-FailureRecoveryConfig-r16 ::= SEQUENCE
struct lbt_fail_recovery_cfg_r16_s {
  struct lbt_fail_instance_max_count_r16_opts {
    enum options { n4, n8, n16, n32, n64, n128, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using lbt_fail_instance_max_count_r16_e_ = enumerated<lbt_fail_instance_max_count_r16_opts>;
  struct lbt_fail_detection_timer_r16_opts {
    enum options { ms10, ms20, ms40, ms80, ms160, ms320, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using lbt_fail_detection_timer_r16_e_ = enumerated<lbt_fail_detection_timer_r16_opts>;

  // member variables
  bool                               ext = false;
  lbt_fail_instance_max_count_r16_e_ lbt_fail_instance_max_count_r16;
  lbt_fail_detection_timer_r16_e_    lbt_fail_detection_timer_r16;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PDCCH-ConfigCommon ::= SEQUENCE
struct pdcch_cfg_common_s {
  using common_search_space_list_l_ = dyn_array<search_space_s>;
  struct first_pdcch_monitoring_occasion_of_po_c_ {
    using scs15_kh_zone_t_l_                                                              = bounded_array<uint8_t, 4>;
    using scs30_kh_zone_t_scs15_kh_zhalf_t_l_                                             = bounded_array<uint16_t, 4>;
    using scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_                         = bounded_array<uint16_t, 4>;
    using scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_ = bounded_array<uint16_t, 4>;
    using scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_ =
        bounded_array<uint16_t, 4>;
    using scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_ = bounded_array<uint16_t, 4>;
    using scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_                     = bounded_array<uint16_t, 4>;
    using scs120_kh_zone_sixteenth_t_l_                                            = bounded_array<uint16_t, 4>;
    struct types_opts {
      enum options {
        scs15_kh_zone_t,
        scs30_kh_zone_t_scs15_kh_zhalf_t,
        scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t,
        scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t,
        scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t,
        scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t,
        scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t,
        scs120_kh_zone_sixteenth_t,
        nulltype
      } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    first_pdcch_monitoring_occasion_of_po_c_() = default;
    first_pdcch_monitoring_occasion_of_po_c_(const first_pdcch_monitoring_occasion_of_po_c_& other);
    first_pdcch_monitoring_occasion_of_po_c_& operator=(const first_pdcch_monitoring_occasion_of_po_c_& other);
    ~first_pdcch_monitoring_occasion_of_po_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    scs15_kh_zone_t_l_& scs15_kh_zone_t()
    {
      assert_choice_type(types::scs15_kh_zone_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs15_kh_zone_t_l_>();
    }
    scs30_kh_zone_t_scs15_kh_zhalf_t_l_& scs30_kh_zone_t_scs15_kh_zhalf_t()
    {
      assert_choice_type(types::scs30_kh_zone_t_scs15_kh_zhalf_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs30_kh_zone_t_scs15_kh_zhalf_t_l_>();
    }
    scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_& scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t()
    {
      assert_choice_type(
          types::scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_>();
    }
    scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_&
    scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t()
    {
      assert_choice_type(types::scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t,
                         type_,
                         "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_>();
    }
    scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_&
    scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t()
    {
      assert_choice_type(types::scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t,
                         type_,
                         "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_>();
    }
    scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_&
    scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t()
    {
      assert_choice_type(types::scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t,
                         type_,
                         "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_>();
    }
    scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_& scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t()
    {
      assert_choice_type(
          types::scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_>();
    }
    scs120_kh_zone_sixteenth_t_l_& scs120_kh_zone_sixteenth_t()
    {
      assert_choice_type(types::scs120_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zone_sixteenth_t_l_>();
    }
    const scs15_kh_zone_t_l_& scs15_kh_zone_t() const
    {
      assert_choice_type(types::scs15_kh_zone_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs15_kh_zone_t_l_>();
    }
    const scs30_kh_zone_t_scs15_kh_zhalf_t_l_& scs30_kh_zone_t_scs15_kh_zhalf_t() const
    {
      assert_choice_type(types::scs30_kh_zone_t_scs15_kh_zhalf_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs30_kh_zone_t_scs15_kh_zhalf_t_l_>();
    }
    const scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_&
    scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t() const
    {
      assert_choice_type(
          types::scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_>();
    }
    const scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_&
    scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t() const
    {
      assert_choice_type(types::scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t,
                         type_,
                         "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_>();
    }
    const scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_&
    scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t() const
    {
      assert_choice_type(types::scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t,
                         type_,
                         "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_>();
    }
    const scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_&
    scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t() const
    {
      assert_choice_type(types::scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t,
                         type_,
                         "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_>();
    }
    const scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_&
    scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t() const
    {
      assert_choice_type(
          types::scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_>();
    }
    const scs120_kh_zone_sixteenth_t_l_& scs120_kh_zone_sixteenth_t() const
    {
      assert_choice_type(types::scs120_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPO");
      return c.get<scs120_kh_zone_sixteenth_t_l_>();
    }
    scs15_kh_zone_t_l_&                                      set_scs15_kh_zone_t();
    scs30_kh_zone_t_scs15_kh_zhalf_t_l_&                     set_scs30_kh_zone_t_scs15_kh_zhalf_t();
    scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_& set_scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t();
    scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_&
    set_scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t();
    scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_&
    set_scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t();
    scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_&
    set_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t();
    scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_& set_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t();
    scs120_kh_zone_sixteenth_t_l_&                        set_scs120_kh_zone_sixteenth_t();

  private:
    types type_;
    choice_buffer_t<scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_,
                    scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_,
                    scs120_kh_zone_sixteenth_t_l_,
                    scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_,
                    scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_,
                    scs15_kh_zone_t_l_,
                    scs30_kh_zone_t_scs15_kh_zhalf_t_l_,
                    scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_>
        c;

    void destroy_();
  };
  using common_search_space_list_ext_r16_l_ = dyn_array<search_space_ext_r16_s>;
  struct sdt_search_space_r17_c_ {
    struct types_opts {
      enum options { new_search_space, existing_search_space, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    sdt_search_space_r17_c_() = default;
    sdt_search_space_r17_c_(const sdt_search_space_r17_c_& other);
    sdt_search_space_r17_c_& operator=(const sdt_search_space_r17_c_& other);
    ~sdt_search_space_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    search_space_s& new_search_space()
    {
      assert_choice_type(types::new_search_space, type_, "sdt-SearchSpace-r17");
      return c.get<search_space_s>();
    }
    uint8_t& existing_search_space()
    {
      assert_choice_type(types::existing_search_space, type_, "sdt-SearchSpace-r17");
      return c.get<uint8_t>();
    }
    const search_space_s& new_search_space() const
    {
      assert_choice_type(types::new_search_space, type_, "sdt-SearchSpace-r17");
      return c.get<search_space_s>();
    }
    const uint8_t& existing_search_space() const
    {
      assert_choice_type(types::existing_search_space, type_, "sdt-SearchSpace-r17");
      return c.get<uint8_t>();
    }
    search_space_s& set_new_search_space();
    uint8_t&        set_existing_search_space();

  private:
    types                           type_;
    choice_buffer_t<search_space_s> c;

    void destroy_();
  };
  using common_search_space_list_ext2_r17_l_ = dyn_array<search_space_ext_v1700_s>;
  struct first_pdcch_monitoring_occasion_of_po_v1710_c_ {
    using scs480_kh_zone_eighth_t_l_    = bounded_array<uint16_t, 4>;
    using scs480_kh_zone_sixteenth_t_l_ = bounded_array<uint32_t, 4>;
    struct types_opts {
      enum options { scs480_kh_zone_eighth_t, scs480_kh_zone_sixteenth_t, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    first_pdcch_monitoring_occasion_of_po_v1710_c_() = default;
    first_pdcch_monitoring_occasion_of_po_v1710_c_(const first_pdcch_monitoring_occasion_of_po_v1710_c_& other);
    first_pdcch_monitoring_occasion_of_po_v1710_c_&
    operator=(const first_pdcch_monitoring_occasion_of_po_v1710_c_& other);
    ~first_pdcch_monitoring_occasion_of_po_v1710_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    scs480_kh_zone_eighth_t_l_& scs480_kh_zone_eighth_t()
    {
      assert_choice_type(types::scs480_kh_zone_eighth_t, type_, "firstPDCCH-MonitoringOccasionOfPO-v1710");
      return c.get<scs480_kh_zone_eighth_t_l_>();
    }
    scs480_kh_zone_sixteenth_t_l_& scs480_kh_zone_sixteenth_t()
    {
      assert_choice_type(types::scs480_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPO-v1710");
      return c.get<scs480_kh_zone_sixteenth_t_l_>();
    }
    const scs480_kh_zone_eighth_t_l_& scs480_kh_zone_eighth_t() const
    {
      assert_choice_type(types::scs480_kh_zone_eighth_t, type_, "firstPDCCH-MonitoringOccasionOfPO-v1710");
      return c.get<scs480_kh_zone_eighth_t_l_>();
    }
    const scs480_kh_zone_sixteenth_t_l_& scs480_kh_zone_sixteenth_t() const
    {
      assert_choice_type(types::scs480_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPO-v1710");
      return c.get<scs480_kh_zone_sixteenth_t_l_>();
    }
    scs480_kh_zone_eighth_t_l_&    set_scs480_kh_zone_eighth_t();
    scs480_kh_zone_sixteenth_t_l_& set_scs480_kh_zone_sixteenth_t();

  private:
    types                                                                      type_;
    choice_buffer_t<scs480_kh_zone_eighth_t_l_, scs480_kh_zone_sixteenth_t_l_> c;

    void destroy_();
  };
  struct pei_cfg_bwp_r17_s_ {
    struct first_pdcch_monitoring_occasion_of_pei_o_r17_c_ {
      using scs15_kh_zone_t_l_                                      = bounded_array<uint8_t, 4>;
      using scs30_kh_zone_t_scs15_kh_zhalf_t_l_                     = bounded_array<uint16_t, 4>;
      using scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_ = bounded_array<uint16_t, 4>;
      using scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_ =
          bounded_array<uint16_t, 4>;
      using scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_ =
          bounded_array<uint16_t, 4>;
      using scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_ =
          bounded_array<uint16_t, 4>;
      using scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_ = bounded_array<uint16_t, 4>;
      using scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t_l_                     = bounded_array<uint16_t, 4>;
      using scs480_kh_zone_eighth_t_l_                                             = bounded_array<uint16_t, 4>;
      using scs480_kh_zone_sixteenth_t_l_                                          = bounded_array<uint32_t, 4>;
      struct types_opts {
        enum options {
          scs15_kh_zone_t,
          scs30_kh_zone_t_scs15_kh_zhalf_t,
          scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t,
          scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t,
          scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t,
          scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t,
          scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t,
          scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t,
          scs480_kh_zone_eighth_t,
          scs480_kh_zone_sixteenth_t,
          nulltype
        } value;

        const char* to_string() const;
      };
      using types = enumerated<types_opts>;

      // choice methods
      first_pdcch_monitoring_occasion_of_pei_o_r17_c_() = default;
      first_pdcch_monitoring_occasion_of_pei_o_r17_c_(const first_pdcch_monitoring_occasion_of_pei_o_r17_c_& other);
      first_pdcch_monitoring_occasion_of_pei_o_r17_c_&
      operator=(const first_pdcch_monitoring_occasion_of_pei_o_r17_c_& other);
      ~first_pdcch_monitoring_occasion_of_pei_o_r17_c_() { destroy_(); }
      void        set(types::options e = types::nulltype);
      types       type() const { return type_; }
      SRSASN_CODE pack(bit_ref& bref) const;
      SRSASN_CODE unpack(cbit_ref& bref);
      void        to_json(json_writer& j) const;
      // getters
      scs15_kh_zone_t_l_& scs15_kh_zone_t()
      {
        assert_choice_type(types::scs15_kh_zone_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs15_kh_zone_t_l_>();
      }
      scs30_kh_zone_t_scs15_kh_zhalf_t_l_& scs30_kh_zone_t_scs15_kh_zhalf_t()
      {
        assert_choice_type(types::scs30_kh_zone_t_scs15_kh_zhalf_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs30_kh_zone_t_scs15_kh_zhalf_t_l_>();
      }
      scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_& scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t()
      {
        assert_choice_type(types::scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t,
                           type_,
                           "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_>();
      }
      scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_&
      scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t()
      {
        assert_choice_type(types::scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t,
                           type_,
                           "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_>();
      }
      scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_&
      scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t()
      {
        assert_choice_type(
            types::scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t,
            type_,
            "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_>();
      }
      scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_&
      scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t()
      {
        assert_choice_type(
            types::scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t,
            type_,
            "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_>();
      }
      scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_&
      scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t()
      {
        assert_choice_type(types::scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t,
                           type_,
                           "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_>();
      }
      scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t_l_& scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t()
      {
        assert_choice_type(
            types::scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t_l_>();
      }
      scs480_kh_zone_eighth_t_l_& scs480_kh_zone_eighth_t()
      {
        assert_choice_type(types::scs480_kh_zone_eighth_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zone_eighth_t_l_>();
      }
      scs480_kh_zone_sixteenth_t_l_& scs480_kh_zone_sixteenth_t()
      {
        assert_choice_type(types::scs480_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zone_sixteenth_t_l_>();
      }
      const scs15_kh_zone_t_l_& scs15_kh_zone_t() const
      {
        assert_choice_type(types::scs15_kh_zone_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs15_kh_zone_t_l_>();
      }
      const scs30_kh_zone_t_scs15_kh_zhalf_t_l_& scs30_kh_zone_t_scs15_kh_zhalf_t() const
      {
        assert_choice_type(types::scs30_kh_zone_t_scs15_kh_zhalf_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs30_kh_zone_t_scs15_kh_zhalf_t_l_>();
      }
      const scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_&
      scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t() const
      {
        assert_choice_type(types::scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t,
                           type_,
                           "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_>();
      }
      const scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_&
      scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t() const
      {
        assert_choice_type(types::scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t,
                           type_,
                           "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_>();
      }
      const scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_&
      scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t() const
      {
        assert_choice_type(
            types::scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t,
            type_,
            "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_>();
      }
      const scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_&
      scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t() const
      {
        assert_choice_type(
            types::scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t,
            type_,
            "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_>();
      }
      const scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_&
      scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t() const
      {
        assert_choice_type(types::scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t,
                           type_,
                           "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_>();
      }
      const scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t_l_& scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t() const
      {
        assert_choice_type(
            types::scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t_l_>();
      }
      const scs480_kh_zone_eighth_t_l_& scs480_kh_zone_eighth_t() const
      {
        assert_choice_type(types::scs480_kh_zone_eighth_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zone_eighth_t_l_>();
      }
      const scs480_kh_zone_sixteenth_t_l_& scs480_kh_zone_sixteenth_t() const
      {
        assert_choice_type(types::scs480_kh_zone_sixteenth_t, type_, "firstPDCCH-MonitoringOccasionOfPEI-O-r17");
        return c.get<scs480_kh_zone_sixteenth_t_l_>();
      }
      scs15_kh_zone_t_l_&                  set_scs15_kh_zone_t();
      scs30_kh_zone_t_scs15_kh_zhalf_t_l_& set_scs30_kh_zone_t_scs15_kh_zhalf_t();
      scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_&
      set_scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t();
      scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_&
      set_scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t();
      scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_&
      set_scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t();
      scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_&
      set_scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t();
      scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_&
      set_scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t();
      scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t_l_& set_scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t();
      scs480_kh_zone_eighth_t_l_&                         set_scs480_kh_zone_eighth_t();
      scs480_kh_zone_sixteenth_t_l_&                      set_scs480_kh_zone_sixteenth_t();

    private:
      types type_;
      choice_buffer_t<scs120_kh_zhalf_t_scs60_kh_zquarter_t_scs30_kh_zone_eighth_t_scs15_kh_zone_sixteenth_t_l_,
                      scs120_kh_zone_t_scs60_kh_zhalf_t_scs30_kh_zquarter_t_scs15_kh_zone_eighth_t_l_,
                      scs15_kh_zone_t_l_,
                      scs30_kh_zone_t_scs15_kh_zhalf_t_l_,
                      scs480_kh_zhalf_t_scs120_kh_zone_eighth_t_scs60_kh_zone_sixteenth_t_l_,
                      scs480_kh_zone_eighth_t_l_,
                      scs480_kh_zone_sixteenth_t_l_,
                      scs480_kh_zone_t_scs120_kh_zquarter_t_scs60_kh_zone_eighth_t_scs30_kh_zone_sixteenth_t_l_,
                      scs480_kh_zquarter_t_scs120_kh_zone_sixteenth_t_l_,
                      scs60_kh_zone_t_scs30_kh_zhalf_t_scs15_kh_zquarter_t_l_>
          c;

      void destroy_();
    };

    // member variables
    uint8_t                                         pei_search_space_r17 = 0;
    first_pdcch_monitoring_occasion_of_pei_o_r17_c_ first_pdcch_monitoring_occasion_of_pei_o_r17;
  };

  // member variables
  bool                        ext                                 = false;
  bool                        coreset_zero_present                = false;
  bool                        common_coreset_present              = false;
  bool                        search_space_zero_present           = false;
  bool                        search_space_sib1_present           = false;
  bool                        search_space_other_sys_info_present = false;
  bool                        paging_search_space_present         = false;
  bool                        ra_search_space_present             = false;
  uint8_t                     coreset_zero                        = 0;
  coreset_s                   common_coreset;
  uint8_t                     search_space_zero = 0;
  common_search_space_list_l_ common_search_space_list;
  uint8_t                     search_space_sib1           = 0;
  uint8_t                     search_space_other_sys_info = 0;
  uint8_t                     paging_search_space         = 0;
  uint8_t                     ra_search_space             = 0;
  // ...
  // group 0
  copy_ptr<first_pdcch_monitoring_occasion_of_po_c_> first_pdcch_monitoring_occasion_of_po;
  // group 1
  copy_ptr<common_search_space_list_ext_r16_l_> common_search_space_list_ext_r16;
  // group 2
  bool                                                     search_space_mcch_r17_present = false;
  bool                                                     search_space_mtch_r17_present = false;
  copy_ptr<sdt_search_space_r17_c_>                        sdt_search_space_r17;
  uint8_t                                                  search_space_mcch_r17 = 0;
  uint8_t                                                  search_space_mtch_r17 = 0;
  copy_ptr<common_search_space_list_ext2_r17_l_>           common_search_space_list_ext2_r17;
  copy_ptr<first_pdcch_monitoring_occasion_of_po_v1710_c_> first_pdcch_monitoring_occasion_of_po_v1710;
  copy_ptr<pei_cfg_bwp_r17_s_>                             pei_cfg_bwp_r17;
  // group 3
  bool follow_unified_tci_state_v1720_present = false;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PDCCH-Config ::= SEQUENCE
struct pdcch_cfg_s {
  using coreset_to_add_mod_list_l_                = dyn_array<coreset_s>;
  using coreset_to_release_list_l_                = bounded_array<uint8_t, 3>;
  using search_spaces_to_add_mod_list_l_          = dyn_array<search_space_s>;
  using search_spaces_to_release_list_l_          = bounded_array<uint8_t, 10>;
  using coreset_to_add_mod_list_size_ext_v1610_l_ = dyn_array<coreset_s>;
  using coreset_to_release_list_size_ext_r16_l_   = bounded_array<uint8_t, 5>;
  using search_spaces_to_add_mod_list_ext_r16_l_  = dyn_array<search_space_ext_r16_s>;
  struct monitoring_cap_cfg_r16_opts {
    enum options { r15monitoringcap, r16monitoringcap, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using monitoring_cap_cfg_r16_e_                  = enumerated<monitoring_cap_cfg_r16_opts>;
  using search_spaces_to_add_mod_list_ext_v1700_l_ = dyn_array<search_space_ext_v1700_s>;
  using pdcch_skipping_dur_list_r17_l_             = bounded_array<uint8_t, 3>;

  // member variables
  bool                                 ext                   = false;
  bool                                 dl_preemption_present = false;
  bool                                 tpc_pusch_present     = false;
  bool                                 tpc_pucch_present     = false;
  bool                                 tpc_srs_present       = false;
  coreset_to_add_mod_list_l_           coreset_to_add_mod_list;
  coreset_to_release_list_l_           coreset_to_release_list;
  search_spaces_to_add_mod_list_l_     search_spaces_to_add_mod_list;
  search_spaces_to_release_list_l_     search_spaces_to_release_list;
  setup_release_c<dl_preemption_s>     dl_preemption;
  setup_release_c<pusch_tpc_cmd_cfg_s> tpc_pusch;
  setup_release_c<pucch_tpc_cmd_cfg_s> tpc_pucch;
  setup_release_c<srs_tpc_cmd_cfg_s>   tpc_srs;
  // ...
  // group 0
  bool                                                monitoring_cap_cfg_r16_present = false;
  copy_ptr<coreset_to_add_mod_list_size_ext_v1610_l_> coreset_to_add_mod_list_size_ext_v1610;
  copy_ptr<coreset_to_release_list_size_ext_r16_l_>   coreset_to_release_list_size_ext_r16;
  copy_ptr<search_spaces_to_add_mod_list_ext_r16_l_>  search_spaces_to_add_mod_list_ext_r16;
  copy_ptr<setup_release_c<ul_cancellation_r16_s>>    ul_cancellation_r16;
  monitoring_cap_cfg_r16_e_                           monitoring_cap_cfg_r16;
  copy_ptr<search_space_switch_cfg_r16_s>             search_space_switch_cfg_r16;
  // group 1
  bool                                                 monitoring_cap_cfg_v1710_present = false;
  copy_ptr<search_spaces_to_add_mod_list_ext_v1700_l_> search_spaces_to_add_mod_list_ext_v1700;
  copy_ptr<search_space_switch_cfg_r17_s>              search_space_switch_cfg_r17;
  copy_ptr<pdcch_skipping_dur_list_r17_l_>             pdcch_skipping_dur_list_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PDSCH-ConfigCommon ::= SEQUENCE
struct pdsch_cfg_common_s {
  bool                               ext = false;
  pdsch_time_domain_res_alloc_list_l pdsch_time_domain_alloc_list;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PDSCH-Config ::= SEQUENCE
struct pdsch_cfg_s {
  using tci_states_to_add_mod_list_l_ = dyn_array<tci_state_s>;
  using tci_states_to_release_list_l_ = dyn_array<uint8_t>;
  struct vrb_to_prb_interleaver_opts {
    enum options { n2, n4, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using vrb_to_prb_interleaver_e_ = enumerated<vrb_to_prb_interleaver_opts>;
  struct res_alloc_opts {
    enum options { res_alloc_type0, res_alloc_type1, dyn_switch, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using res_alloc_e_ = enumerated<res_alloc_opts>;
  struct pdsch_aggregation_factor_opts {
    enum options { n2, n4, n8, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pdsch_aggregation_factor_e_           = enumerated<pdsch_aggregation_factor_opts>;
  using rate_match_pattern_to_add_mod_list_l_ = dyn_array<rate_match_pattern_s>;
  using rate_match_pattern_to_release_list_l_ = bounded_array<uint8_t, 4>;
  struct rbg_size_opts {
    enum options { cfg1, cfg2, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using rbg_size_e_ = enumerated<rbg_size_opts>;
  struct mcs_table_opts {
    enum options { qam256, qam64_low_se, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using mcs_table_e_ = enumerated<mcs_table_opts>;
  struct max_nrof_code_words_sched_by_dci_opts {
    enum options { n1, n2, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using max_nrof_code_words_sched_by_dci_e_ = enumerated<max_nrof_code_words_sched_by_dci_opts>;
  struct prb_bundling_type_c_ {
    struct static_bundling_s_ {
      struct bundle_size_opts {
        enum options { n4, wideband, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using bundle_size_e_ = enumerated<bundle_size_opts>;

      // member variables
      bool           bundle_size_present = false;
      bundle_size_e_ bundle_size;
    };
    struct dyn_bundling_s_ {
      struct bundle_size_set1_opts {
        enum options { n4, wideband, n2_wideband, n4_wideband, nulltype } value;

        const char* to_string() const;
      };
      using bundle_size_set1_e_ = enumerated<bundle_size_set1_opts>;
      struct bundle_size_set2_opts {
        enum options { n4, wideband, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using bundle_size_set2_e_ = enumerated<bundle_size_set2_opts>;

      // member variables
      bool                bundle_size_set1_present = false;
      bool                bundle_size_set2_present = false;
      bundle_size_set1_e_ bundle_size_set1;
      bundle_size_set2_e_ bundle_size_set2;
    };
    struct types_opts {
      enum options { static_bundling, dyn_bundling, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    prb_bundling_type_c_() = default;
    prb_bundling_type_c_(const prb_bundling_type_c_& other);
    prb_bundling_type_c_& operator=(const prb_bundling_type_c_& other);
    ~prb_bundling_type_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    static_bundling_s_& static_bundling()
    {
      assert_choice_type(types::static_bundling, type_, "prb-BundlingType");
      return c.get<static_bundling_s_>();
    }
    dyn_bundling_s_& dyn_bundling()
    {
      assert_choice_type(types::dyn_bundling, type_, "prb-BundlingType");
      return c.get<dyn_bundling_s_>();
    }
    const static_bundling_s_& static_bundling() const
    {
      assert_choice_type(types::static_bundling, type_, "prb-BundlingType");
      return c.get<static_bundling_s_>();
    }
    const dyn_bundling_s_& dyn_bundling() const
    {
      assert_choice_type(types::dyn_bundling, type_, "prb-BundlingType");
      return c.get<dyn_bundling_s_>();
    }
    static_bundling_s_& set_static_bundling();
    dyn_bundling_s_&    set_dyn_bundling();

  private:
    types                                                type_;
    choice_buffer_t<dyn_bundling_s_, static_bundling_s_> c;

    void destroy_();
  };
  using zp_csi_rs_res_to_add_mod_list_l_                            = dyn_array<zp_csi_rs_res_s>;
  using zp_csi_rs_res_to_release_list_l_                            = bounded_array<uint8_t, 32>;
  using aperiodic_zp_csi_rs_res_sets_to_add_mod_list_l_             = dyn_array<zp_csi_rs_res_set_s>;
  using aperiodic_zp_csi_rs_res_sets_to_release_list_l_             = bounded_array<uint8_t, 16>;
  using sp_zp_csi_rs_res_sets_to_add_mod_list_l_                    = dyn_array<zp_csi_rs_res_set_s>;
  using sp_zp_csi_rs_res_sets_to_release_list_l_                    = bounded_array<uint8_t, 16>;
  using aperiodic_zp_csi_rs_res_sets_to_add_mod_list_dci_1_2_r16_l_ = dyn_array<zp_csi_rs_res_set_s>;
  using aperiodic_zp_csi_rs_res_sets_to_release_list_dci_1_2_r16_l_ = bounded_array<uint8_t, 16>;
  struct mcs_table_dci_1_2_r16_opts {
    enum options { qam256, qam64_low_se, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using mcs_table_dci_1_2_r16_e_ = enumerated<mcs_table_dci_1_2_r16_opts>;
  struct prb_bundling_type_dci_1_2_r16_c_ {
    struct static_bundling_r16_s_ {
      struct bundle_size_r16_opts {
        enum options { n4, wideband, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using bundle_size_r16_e_ = enumerated<bundle_size_r16_opts>;

      // member variables
      bool               bundle_size_r16_present = false;
      bundle_size_r16_e_ bundle_size_r16;
    };
    struct dyn_bundling_r16_s_ {
      struct bundle_size_set1_r16_opts {
        enum options { n4, wideband, n2_wideband, n4_wideband, nulltype } value;

        const char* to_string() const;
      };
      using bundle_size_set1_r16_e_ = enumerated<bundle_size_set1_r16_opts>;
      struct bundle_size_set2_r16_opts {
        enum options { n4, wideband, nulltype } value;
        typedef uint8_t number_type;

        const char* to_string() const;
        uint8_t     to_number() const;
      };
      using bundle_size_set2_r16_e_ = enumerated<bundle_size_set2_r16_opts>;

      // member variables
      bool                    bundle_size_set1_r16_present = false;
      bool                    bundle_size_set2_r16_present = false;
      bundle_size_set1_r16_e_ bundle_size_set1_r16;
      bundle_size_set2_r16_e_ bundle_size_set2_r16;
    };
    struct types_opts {
      enum options { static_bundling_r16, dyn_bundling_r16, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    prb_bundling_type_dci_1_2_r16_c_() = default;
    prb_bundling_type_dci_1_2_r16_c_(const prb_bundling_type_dci_1_2_r16_c_& other);
    prb_bundling_type_dci_1_2_r16_c_& operator=(const prb_bundling_type_dci_1_2_r16_c_& other);
    ~prb_bundling_type_dci_1_2_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    static_bundling_r16_s_& static_bundling_r16()
    {
      assert_choice_type(types::static_bundling_r16, type_, "prb-BundlingTypeDCI-1-2-r16");
      return c.get<static_bundling_r16_s_>();
    }
    dyn_bundling_r16_s_& dyn_bundling_r16()
    {
      assert_choice_type(types::dyn_bundling_r16, type_, "prb-BundlingTypeDCI-1-2-r16");
      return c.get<dyn_bundling_r16_s_>();
    }
    const static_bundling_r16_s_& static_bundling_r16() const
    {
      assert_choice_type(types::static_bundling_r16, type_, "prb-BundlingTypeDCI-1-2-r16");
      return c.get<static_bundling_r16_s_>();
    }
    const dyn_bundling_r16_s_& dyn_bundling_r16() const
    {
      assert_choice_type(types::dyn_bundling_r16, type_, "prb-BundlingTypeDCI-1-2-r16");
      return c.get<dyn_bundling_r16_s_>();
    }
    static_bundling_r16_s_& set_static_bundling_r16();
    dyn_bundling_r16_s_&    set_dyn_bundling_r16();

  private:
    types                                                        type_;
    choice_buffer_t<dyn_bundling_r16_s_, static_bundling_r16_s_> c;

    void destroy_();
  };
  struct res_alloc_type1_granularity_dci_1_2_r16_opts {
    enum options { n2, n4, n8, n16, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using res_alloc_type1_granularity_dci_1_2_r16_e_ = enumerated<res_alloc_type1_granularity_dci_1_2_r16_opts>;
  struct vrb_to_prb_interleaver_dci_1_2_r16_opts {
    enum options { n2, n4, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using vrb_to_prb_interleaver_dci_1_2_r16_e_ = enumerated<vrb_to_prb_interleaver_dci_1_2_r16_opts>;
  struct res_alloc_dci_1_2_r16_opts {
    enum options { res_alloc_type0, res_alloc_type1, dyn_switch, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using res_alloc_dci_1_2_r16_e_ = enumerated<res_alloc_dci_1_2_r16_opts>;
  struct dl_or_joint_tci_state_list_r17_c_ {
    struct explicitlist_s_ {
      using dl_or_joint_tci_state_to_add_mod_list_r17_l_ = dyn_array<tci_state_s>;
      using dl_or_joint_tci_state_to_release_list_r17_l_ = dyn_array<uint8_t>;

      // member variables
      dl_or_joint_tci_state_to_add_mod_list_r17_l_ dl_or_joint_tci_state_to_add_mod_list_r17;
      dl_or_joint_tci_state_to_release_list_r17_l_ dl_or_joint_tci_state_to_release_list_r17;
    };
    struct types_opts {
      enum options { explicitlist, unified_tci_state_ref_r17, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    dl_or_joint_tci_state_list_r17_c_() = default;
    dl_or_joint_tci_state_list_r17_c_(const dl_or_joint_tci_state_list_r17_c_& other);
    dl_or_joint_tci_state_list_r17_c_& operator=(const dl_or_joint_tci_state_list_r17_c_& other);
    ~dl_or_joint_tci_state_list_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    explicitlist_s_& explicitlist()
    {
      assert_choice_type(types::explicitlist, type_, "dl-OrJointTCI-StateList-r17");
      return c.get<explicitlist_s_>();
    }
    serving_cell_and_bwp_id_r17_s& unified_tci_state_ref_r17()
    {
      assert_choice_type(types::unified_tci_state_ref_r17, type_, "dl-OrJointTCI-StateList-r17");
      return c.get<serving_cell_and_bwp_id_r17_s>();
    }
    const explicitlist_s_& explicitlist() const
    {
      assert_choice_type(types::explicitlist, type_, "dl-OrJointTCI-StateList-r17");
      return c.get<explicitlist_s_>();
    }
    const serving_cell_and_bwp_id_r17_s& unified_tci_state_ref_r17() const
    {
      assert_choice_type(types::unified_tci_state_ref_r17, type_, "dl-OrJointTCI-StateList-r17");
      return c.get<serving_cell_and_bwp_id_r17_s>();
    }
    explicitlist_s_&               set_explicitlist();
    serving_cell_and_bwp_id_r17_s& set_unified_tci_state_ref_r17();

  private:
    types                                                           type_;
    choice_buffer_t<explicitlist_s_, serving_cell_and_bwp_id_r17_s> c;

    void destroy_();
  };
  struct beam_app_time_r17_opts {
    enum options {
      n1,
      n2,
      n4,
      n7,
      n14,
      n28,
      n42,
      n56,
      n70,
      n84,
      n98,
      n112,
      n224,
      n336,
      spare2,
      spare1,
      nulltype
    } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using beam_app_time_r17_e_ = enumerated<beam_app_time_r17_opts>;
  struct xoverhead_multicast_r17_opts {
    enum options { xoh6, xoh12, xoh18, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using xoverhead_multicast_r17_e_ = enumerated<xoverhead_multicast_r17_opts>;

  // member variables
  bool                                                              ext                                      = false;
  bool                                                              data_scrambling_id_pdsch_present         = false;
  bool                                                              dmrs_dl_for_pdsch_map_type_a_present     = false;
  bool                                                              dmrs_dl_for_pdsch_map_type_b_present     = false;
  bool                                                              vrb_to_prb_interleaver_present           = false;
  bool                                                              pdsch_time_domain_alloc_list_present     = false;
  bool                                                              pdsch_aggregation_factor_present         = false;
  bool                                                              mcs_table_present                        = false;
  bool                                                              max_nrof_code_words_sched_by_dci_present = false;
  bool                                                              p_zp_csi_rs_res_set_present              = false;
  uint16_t                                                          data_scrambling_id_pdsch                 = 0;
  setup_release_c<dmrs_dl_cfg_s>                                    dmrs_dl_for_pdsch_map_type_a;
  setup_release_c<dmrs_dl_cfg_s>                                    dmrs_dl_for_pdsch_map_type_b;
  tci_states_to_add_mod_list_l_                                     tci_states_to_add_mod_list;
  tci_states_to_release_list_l_                                     tci_states_to_release_list;
  vrb_to_prb_interleaver_e_                                         vrb_to_prb_interleaver;
  res_alloc_e_                                                      res_alloc;
  setup_release_c<dyn_seq_of<pdsch_time_domain_res_alloc_s, 1, 16>> pdsch_time_domain_alloc_list;
  pdsch_aggregation_factor_e_                                       pdsch_aggregation_factor;
  rate_match_pattern_to_add_mod_list_l_                             rate_match_pattern_to_add_mod_list;
  rate_match_pattern_to_release_list_l_                             rate_match_pattern_to_release_list;
  rate_match_pattern_group_l                                        rate_match_pattern_group1;
  rate_match_pattern_group_l                                        rate_match_pattern_group2;
  rbg_size_e_                                                       rbg_size;
  mcs_table_e_                                                      mcs_table;
  max_nrof_code_words_sched_by_dci_e_                               max_nrof_code_words_sched_by_dci;
  prb_bundling_type_c_                                              prb_bundling_type;
  zp_csi_rs_res_to_add_mod_list_l_                                  zp_csi_rs_res_to_add_mod_list;
  zp_csi_rs_res_to_release_list_l_                                  zp_csi_rs_res_to_release_list;
  aperiodic_zp_csi_rs_res_sets_to_add_mod_list_l_                   aperiodic_zp_csi_rs_res_sets_to_add_mod_list;
  aperiodic_zp_csi_rs_res_sets_to_release_list_l_                   aperiodic_zp_csi_rs_res_sets_to_release_list;
  sp_zp_csi_rs_res_sets_to_add_mod_list_l_                          sp_zp_csi_rs_res_sets_to_add_mod_list;
  sp_zp_csi_rs_res_sets_to_release_list_l_                          sp_zp_csi_rs_res_sets_to_release_list;
  setup_release_c<zp_csi_rs_res_set_s>                              p_zp_csi_rs_res_set;
  // ...
  // group 0
  bool                                              ant_ports_field_presence_dci_1_2_r16_present    = false;
  bool                                              dmrs_seq_initization_dci_1_2_r16_present        = false;
  bool                                              harq_process_num_size_dci_1_2_r16_present       = false;
  bool                                              mcs_table_dci_1_2_r16_present                   = false;
  bool                                              nof_bits_for_rv_dci_1_2_r16_present             = false;
  bool                                              prio_ind_dci_1_2_r16_present                    = false;
  bool                                              res_alloc_type1_granularity_dci_1_2_r16_present = false;
  bool                                              vrb_to_prb_interleaver_dci_1_2_r16_present      = false;
  bool                                              ref_of_slivdci_1_2_r16_present                  = false;
  bool                                              res_alloc_dci_1_2_r16_present                   = false;
  bool                                              prio_ind_dci_1_1_r16_present                    = false;
  bool                                              data_scrambling_id_pdsch2_r16_present           = false;
  copy_ptr<setup_release_c<integer<uint8_t, 1, 8>>> max_mimo_layers_r16;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 16>, 1, 2>>> minimum_sched_offset_k0_r16;
  copy_ptr<aperiodic_zp_csi_rs_res_sets_to_add_mod_list_dci_1_2_r16_l_>
      aperiodic_zp_csi_rs_res_sets_to_add_mod_list_dci_1_2_r16;
  copy_ptr<aperiodic_zp_csi_rs_res_sets_to_release_list_dci_1_2_r16_l_>
                                           aperiodic_zp_csi_rs_res_sets_to_release_list_dci_1_2_r16;
  copy_ptr<setup_release_c<dmrs_dl_cfg_s>> dmrs_dl_for_pdsch_map_type_a_dci_1_2_r16;
  copy_ptr<setup_release_c<dmrs_dl_cfg_s>> dmrs_dl_for_pdsch_map_type_b_dci_1_2_r16;
  uint8_t                                  harq_process_num_size_dci_1_2_r16 = 0;
  mcs_table_dci_1_2_r16_e_                 mcs_table_dci_1_2_r16;
  uint8_t                                  nof_bits_for_rv_dci_1_2_r16 = 0;
  copy_ptr<setup_release_c<dyn_seq_of<pdsch_time_domain_res_alloc_r16_s, 1, 16>>>
                                             pdsch_time_domain_alloc_list_dci_1_2_r16;
  copy_ptr<prb_bundling_type_dci_1_2_r16_c_> prb_bundling_type_dci_1_2_r16;
  copy_ptr<rate_match_pattern_group_l>       rate_match_pattern_group1_dci_1_2_r16;
  copy_ptr<rate_match_pattern_group_l>       rate_match_pattern_group2_dci_1_2_r16;
  res_alloc_type1_granularity_dci_1_2_r16_e_ res_alloc_type1_granularity_dci_1_2_r16;
  vrb_to_prb_interleaver_dci_1_2_r16_e_      vrb_to_prb_interleaver_dci_1_2_r16;
  res_alloc_dci_1_2_r16_e_                   res_alloc_dci_1_2_r16;
  uint16_t                                   data_scrambling_id_pdsch2_r16 = 0;
  copy_ptr<setup_release_c<dyn_seq_of<pdsch_time_domain_res_alloc_r16_s, 1, 16>>> pdsch_time_domain_alloc_list_r16;
  copy_ptr<setup_release_c<repeat_scheme_cfg_r16_c>>                              repeat_scheme_cfg_r16;
  // group 1
  copy_ptr<setup_release_c<repeat_scheme_cfg_v1630_s>> repeat_scheme_cfg_v1630;
  // group 2
  bool                                        pdsch_harq_ack_one_shot_feedback_dci_1_2_r17_present = false;
  bool                                        pdsch_harq_ack_enh_type3_dci_1_2_r17_present         = false;
  bool                                        pdsch_harq_ack_enh_type3_dci_field_1_2_r17_present   = false;
  bool                                        pdsch_harq_ack_retx_dci_1_2_r17_present              = false;
  bool                                        pucch_s_scell_dyn_dci_1_2_r17_present                = false;
  bool                                        beam_app_time_r17_present                            = false;
  bool                                        dmrs_fd_occ_disabled_for_rank1_pdsch_r17_present     = false;
  bool                                        harq_process_num_size_dci_1_2_v1700_present          = false;
  bool                                        harq_process_num_size_dci_1_1_r17_present            = false;
  bool                                        mcs_table_r17_present                                = false;
  bool                                        mcs_table_dci_1_2_r17_present                        = false;
  bool                                        xoverhead_multicast_r17_present                      = false;
  bool                                        prio_ind_dci_4_2_r17_present                         = false;
  bool                                        size_dci_4_2_r17_present                             = false;
  copy_ptr<dl_or_joint_tci_state_list_r17_c_> dl_or_joint_tci_state_list_r17;
  beam_app_time_r17_e_                        beam_app_time_r17;
  copy_ptr<setup_release_c<dyn_seq_of<multi_pdsch_tdra_r17_s, 1, 16>>> dummy;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 64>, 1, 2>>> minimum_sched_offset_k0_r17;
  uint8_t                                                              harq_process_num_size_dci_1_2_v1700 = 0;
  uint8_t                                                              harq_process_num_size_dci_1_1_r17   = 5;
  xoverhead_multicast_r17_e_                                           xoverhead_multicast_r17;
  uint8_t                                                              size_dci_4_2_r17 = 20;
  // group 3
  copy_ptr<setup_release_c<dyn_seq_of<multi_pdsch_tdra_r17_s, 1, 64>>> pdsch_time_domain_alloc_list_for_multi_pdsch_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-ConfigCommon ::= SEQUENCE
struct pucch_cfg_common_s {
  struct pucch_group_hop_opts {
    enum options { neither, enable, disable, nulltype } value;

    const char* to_string() const;
  };
  using pucch_group_hop_e_ = enumerated<pucch_group_hop_opts>;
  struct intra_slot_fh_r17_opts {
    enum options { from_lower_edge, from_upper_edge, nulltype } value;

    const char* to_string() const;
  };
  using intra_slot_fh_r17_e_ = enumerated<intra_slot_fh_r17_opts>;
  struct add_prb_offset_r17_opts {
    enum options { n2, n3, n4, n6, n8, n9, n10, n12, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using add_prb_offset_r17_e_ = enumerated<add_prb_offset_r17_opts>;

  // member variables
  bool               ext                      = false;
  bool               pucch_res_common_present = false;
  bool               hop_id_present           = false;
  bool               p0_nominal_present       = false;
  uint8_t            pucch_res_common         = 0;
  pucch_group_hop_e_ pucch_group_hop;
  uint16_t           hop_id     = 0;
  int16_t            p0_nominal = -202;
  // ...
  // group 0
  bool                  nrof_prbs_present                    = false;
  bool                  intra_slot_fh_r17_present            = false;
  bool                  pucch_res_common_red_cap_r17_present = false;
  bool                  add_prb_offset_r17_present           = false;
  uint8_t               nrof_prbs                            = 1;
  intra_slot_fh_r17_e_  intra_slot_fh_r17;
  uint8_t               pucch_res_common_red_cap_r17 = 0;
  add_prb_offset_r17_e_ add_prb_offset_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUCCH-ConfigurationList-r16 ::= SEQUENCE (SIZE (1..2)) OF PUCCH-Config
using pucch_cfg_list_r16_l = dyn_array<pucch_cfg_s>;

// PUSCH-ConfigCommon ::= SEQUENCE
struct pusch_cfg_common_s {
  bool                               ext                                           = false;
  bool                               group_hop_enabled_transform_precoding_present = false;
  bool                               msg3_delta_preamb_present                     = false;
  bool                               p0_nominal_with_grant_present                 = false;
  pusch_time_domain_res_alloc_list_l pusch_time_domain_alloc_list;
  int8_t                             msg3_delta_preamb     = -1;
  int16_t                            p0_nominal_with_grant = -202;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PUSCH-Config ::= SEQUENCE
struct pusch_cfg_s {
  struct tx_cfg_opts {
    enum options { codebook, non_codebook, nulltype } value;

    const char* to_string() const;
  };
  using tx_cfg_e_ = enumerated<tx_cfg_opts>;
  struct freq_hop_opts {
    enum options { intra_slot, inter_slot, nulltype } value;

    const char* to_string() const;
  };
  using freq_hop_e_              = enumerated<freq_hop_opts>;
  using freq_hop_offset_lists_l_ = bounded_array<uint16_t, 4>;
  struct res_alloc_opts {
    enum options { res_alloc_type0, res_alloc_type1, dyn_switch, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using res_alloc_e_ = enumerated<res_alloc_opts>;
  struct pusch_aggregation_factor_opts {
    enum options { n2, n4, n8, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using pusch_aggregation_factor_e_ = enumerated<pusch_aggregation_factor_opts>;
  struct mcs_table_opts {
    enum options { qam256, qam64_low_se, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using mcs_table_e_ = enumerated<mcs_table_opts>;
  struct mcs_table_transform_precoder_opts {
    enum options { qam256, qam64_low_se, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using mcs_table_transform_precoder_e_ = enumerated<mcs_table_transform_precoder_opts>;
  struct transform_precoder_opts {
    enum options { enabled, disabled, nulltype } value;

    const char* to_string() const;
  };
  using transform_precoder_e_ = enumerated<transform_precoder_opts>;
  struct codebook_subset_opts {
    enum options { fully_and_partial_and_non_coherent, partial_and_non_coherent, non_coherent, nulltype } value;

    const char* to_string() const;
  };
  using codebook_subset_e_ = enumerated<codebook_subset_opts>;
  struct freq_hop_dci_0_2_r16_c_ {
    struct pusch_rep_type_a_opts {
      enum options { intra_slot, inter_slot, nulltype } value;

      const char* to_string() const;
    };
    using pusch_rep_type_a_e_ = enumerated<pusch_rep_type_a_opts>;
    struct pusch_rep_type_b_opts {
      enum options { inter_repeat, inter_slot, nulltype } value;

      const char* to_string() const;
    };
    using pusch_rep_type_b_e_ = enumerated<pusch_rep_type_b_opts>;
    struct types_opts {
      enum options { pusch_rep_type_a, pusch_rep_type_b, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    freq_hop_dci_0_2_r16_c_() = default;
    freq_hop_dci_0_2_r16_c_(const freq_hop_dci_0_2_r16_c_& other);
    freq_hop_dci_0_2_r16_c_& operator=(const freq_hop_dci_0_2_r16_c_& other);
    ~freq_hop_dci_0_2_r16_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    pusch_rep_type_a_e_& pusch_rep_type_a()
    {
      assert_choice_type(types::pusch_rep_type_a, type_, "frequencyHoppingDCI-0-2-r16");
      return c.get<pusch_rep_type_a_e_>();
    }
    pusch_rep_type_b_e_& pusch_rep_type_b()
    {
      assert_choice_type(types::pusch_rep_type_b, type_, "frequencyHoppingDCI-0-2-r16");
      return c.get<pusch_rep_type_b_e_>();
    }
    const pusch_rep_type_a_e_& pusch_rep_type_a() const
    {
      assert_choice_type(types::pusch_rep_type_a, type_, "frequencyHoppingDCI-0-2-r16");
      return c.get<pusch_rep_type_a_e_>();
    }
    const pusch_rep_type_b_e_& pusch_rep_type_b() const
    {
      assert_choice_type(types::pusch_rep_type_b, type_, "frequencyHoppingDCI-0-2-r16");
      return c.get<pusch_rep_type_b_e_>();
    }
    pusch_rep_type_a_e_& set_pusch_rep_type_a();
    pusch_rep_type_b_e_& set_pusch_rep_type_b();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };
  struct codebook_subset_dci_0_2_r16_opts {
    enum options { fully_and_partial_and_non_coherent, partial_and_non_coherent, non_coherent, nulltype } value;

    const char* to_string() const;
  };
  using codebook_subset_dci_0_2_r16_e_ = enumerated<codebook_subset_dci_0_2_r16_opts>;
  struct mcs_table_dci_0_2_r16_opts {
    enum options { qam256, qam64_low_se, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using mcs_table_dci_0_2_r16_e_ = enumerated<mcs_table_dci_0_2_r16_opts>;
  struct mcs_table_transform_precoder_dci_0_2_r16_opts {
    enum options { qam256, qam64_low_se, nulltype } value;
    typedef uint16_t number_type;

    const char* to_string() const;
    uint16_t    to_number() const;
  };
  using mcs_table_transform_precoder_dci_0_2_r16_e_ = enumerated<mcs_table_transform_precoder_dci_0_2_r16_opts>;
  struct pusch_rep_type_ind_dci_0_2_r16_opts {
    enum options { pusch_rep_type_a, pusch_rep_type_b, nulltype } value;

    const char* to_string() const;
  };
  using pusch_rep_type_ind_dci_0_2_r16_e_ = enumerated<pusch_rep_type_ind_dci_0_2_r16_opts>;
  struct res_alloc_dci_0_2_r16_opts {
    enum options { res_alloc_type0, res_alloc_type1, dyn_switch, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using res_alloc_dci_0_2_r16_e_ = enumerated<res_alloc_dci_0_2_r16_opts>;
  struct res_alloc_type1_granularity_dci_0_2_r16_opts {
    enum options { n2, n4, n8, n16, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using res_alloc_type1_granularity_dci_0_2_r16_e_ = enumerated<res_alloc_type1_granularity_dci_0_2_r16_opts>;
  struct pusch_rep_type_ind_dci_0_1_r16_opts {
    enum options { pusch_rep_type_a, pusch_rep_type_b, nulltype } value;

    const char* to_string() const;
  };
  using pusch_rep_type_ind_dci_0_1_r16_e_ = enumerated<pusch_rep_type_ind_dci_0_1_r16_opts>;
  struct freq_hop_dci_0_1_r16_opts {
    enum options { inter_repeat, inter_slot, nulltype } value;

    const char* to_string() const;
  };
  using freq_hop_dci_0_1_r16_e_ = enumerated<freq_hop_dci_0_1_r16_opts>;
  struct ul_full_pwr_tx_r16_opts {
    enum options { fullpwr, fullpwr_mode1, fullpwr_mode2, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ul_full_pwr_tx_r16_e_ = enumerated<ul_full_pwr_tx_r16_opts>;
  struct map_pattern_r17_opts {
    enum options { cyclic_map, sequential_map, nulltype } value;

    const char* to_string() const;
  };
  using map_pattern_r17_e_                  = enumerated<map_pattern_r17_opts>;
  using mpe_res_pool_to_add_mod_list_r17_l_ = dyn_array<mpe_res_r17_s>;
  using mpe_res_pool_to_release_list_r17_l_ = dyn_array<uint8_t>;

  // member variables
  bool                                                              ext                                  = false;
  bool                                                              data_scrambling_id_pusch_present     = false;
  bool                                                              tx_cfg_present                       = false;
  bool                                                              dmrs_ul_for_pusch_map_type_a_present = false;
  bool                                                              dmrs_ul_for_pusch_map_type_b_present = false;
  bool                                                              pusch_pwr_ctrl_present               = false;
  bool                                                              freq_hop_present                     = false;
  bool                                                              pusch_time_domain_alloc_list_present = false;
  bool                                                              pusch_aggregation_factor_present     = false;
  bool                                                              mcs_table_present                    = false;
  bool                                                              mcs_table_transform_precoder_present = false;
  bool                                                              transform_precoder_present           = false;
  bool                                                              codebook_subset_present              = false;
  bool                                                              max_rank_present                     = false;
  bool                                                              rbg_size_present                     = false;
  bool                                                              uci_on_pusch_present                 = false;
  bool                                                              tp_pi2_bpsk_present                  = false;
  uint16_t                                                          data_scrambling_id_pusch             = 0;
  tx_cfg_e_                                                         tx_cfg;
  setup_release_c<dmrs_ul_cfg_s>                                    dmrs_ul_for_pusch_map_type_a;
  setup_release_c<dmrs_ul_cfg_s>                                    dmrs_ul_for_pusch_map_type_b;
  pusch_pwr_ctrl_s                                                  pusch_pwr_ctrl;
  freq_hop_e_                                                       freq_hop;
  freq_hop_offset_lists_l_                                          freq_hop_offset_lists;
  res_alloc_e_                                                      res_alloc;
  setup_release_c<dyn_seq_of<pusch_time_domain_res_alloc_s, 1, 16>> pusch_time_domain_alloc_list;
  pusch_aggregation_factor_e_                                       pusch_aggregation_factor;
  mcs_table_e_                                                      mcs_table;
  mcs_table_transform_precoder_e_                                   mcs_table_transform_precoder;
  transform_precoder_e_                                             transform_precoder;
  codebook_subset_e_                                                codebook_subset;
  uint8_t                                                           max_rank = 1;
  setup_release_c<uci_on_pusch_s>                                   uci_on_pusch;
  // ...
  // group 0
  bool harq_process_num_size_dci_0_2_r16_present           = false;
  bool dmrs_seq_initization_dci_0_2_r16_present            = false;
  bool nof_bits_for_rv_dci_0_2_r16_present                 = false;
  bool ant_ports_field_presence_dci_0_2_r16_present        = false;
  bool codebook_subset_dci_0_2_r16_present                 = false;
  bool invalid_symbol_pattern_ind_dci_0_2_r16_present      = false;
  bool max_rank_dci_0_2_r16_present                        = false;
  bool mcs_table_dci_0_2_r16_present                       = false;
  bool mcs_table_transform_precoder_dci_0_2_r16_present    = false;
  bool prio_ind_dci_0_2_r16_present                        = false;
  bool pusch_rep_type_ind_dci_0_2_r16_present              = false;
  bool res_alloc_dci_0_2_r16_present                       = false;
  bool res_alloc_type1_granularity_dci_0_2_r16_present     = false;
  bool invalid_symbol_pattern_ind_dci_0_1_r16_present      = false;
  bool prio_ind_dci_0_1_r16_present                        = false;
  bool pusch_rep_type_ind_dci_0_1_r16_present              = false;
  bool freq_hop_dci_0_1_r16_present                        = false;
  bool ul_full_pwr_tx_r16_present                          = false;
  bool nof_invalid_symbols_for_dl_ul_switching_r16_present = false;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 16>, 1, 2>>>    minimum_sched_offset_k2_r16;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 63>, 1, 64>>>   ul_access_cfg_list_dci_0_1_r16;
  uint8_t                                                                 harq_process_num_size_dci_0_2_r16 = 0;
  uint8_t                                                                 nof_bits_for_rv_dci_0_2_r16       = 0;
  copy_ptr<setup_release_c<dmrs_ul_cfg_s>>                                dmrs_ul_for_pusch_map_type_a_dci_0_2_r16;
  copy_ptr<setup_release_c<dmrs_ul_cfg_s>>                                dmrs_ul_for_pusch_map_type_b_dci_0_2_r16;
  copy_ptr<freq_hop_dci_0_2_r16_c_>                                       freq_hop_dci_0_2_r16;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint16_t, 1, 274>, 1, 4>>>  freq_hop_offset_lists_dci_0_2_r16;
  codebook_subset_dci_0_2_r16_e_                                          codebook_subset_dci_0_2_r16;
  uint8_t                                                                 max_rank_dci_0_2_r16 = 1;
  mcs_table_dci_0_2_r16_e_                                                mcs_table_dci_0_2_r16;
  mcs_table_transform_precoder_dci_0_2_r16_e_                             mcs_table_transform_precoder_dci_0_2_r16;
  pusch_rep_type_ind_dci_0_2_r16_e_                                       pusch_rep_type_ind_dci_0_2_r16;
  res_alloc_dci_0_2_r16_e_                                                res_alloc_dci_0_2_r16;
  res_alloc_type1_granularity_dci_0_2_r16_e_                              res_alloc_type1_granularity_dci_0_2_r16;
  copy_ptr<setup_release_c<dyn_seq_of<uci_on_pusch_dci_0_2_r16_s, 1, 2>>> uci_on_pusch_list_dci_0_2_r16;
  copy_ptr<setup_release_c<dyn_seq_of<pusch_time_domain_res_alloc_r16_s, 1, 64>>>
      pusch_time_domain_alloc_list_dci_0_2_r16;
  copy_ptr<setup_release_c<dyn_seq_of<pusch_time_domain_res_alloc_r16_s, 1, 64>>>
                                                              pusch_time_domain_alloc_list_dci_0_1_r16;
  pusch_rep_type_ind_dci_0_1_r16_e_                           pusch_rep_type_ind_dci_0_1_r16;
  freq_hop_dci_0_1_r16_e_                                     freq_hop_dci_0_1_r16;
  copy_ptr<setup_release_c<dyn_seq_of<uci_on_pusch_s, 1, 2>>> uci_on_pusch_list_dci_0_1_r16;
  copy_ptr<invalid_symbol_pattern_r16_s>                      invalid_symbol_pattern_r16;
  copy_ptr<setup_release_c<pusch_pwr_ctrl_v1610_s>>           pusch_pwr_ctrl_v1610;
  ul_full_pwr_tx_r16_e_                                       ul_full_pwr_tx_r16;
  copy_ptr<setup_release_c<dyn_seq_of<pusch_time_domain_res_alloc_r16_s, 1, 64>>>
          pusch_time_domain_alloc_list_for_multi_pusch_r16;
  uint8_t nof_invalid_symbols_for_dl_ul_switching_r16 = 1;
  // group 1
  bool map_pattern_r17_present                     = false;
  bool second_tpc_field_dci_0_1_r17_present        = false;
  bool second_tpc_field_dci_0_2_r17_present        = false;
  bool seq_offset_for_rv_r17_present               = false;
  bool available_slot_count_r17_present            = false;
  bool harq_process_num_size_dci_0_2_v1700_present = false;
  bool harq_process_num_size_dci_0_1_r17_present   = false;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 63>, 1, 64>>> ul_access_cfg_list_dci_0_2_r17;
  copy_ptr<setup_release_c<beta_offsets_cross_pri_sel_r17_c>>           beta_offsets_cross_pri0_r17;
  copy_ptr<setup_release_c<beta_offsets_cross_pri_sel_r17_c>>           beta_offsets_cross_pri1_r17;
  copy_ptr<setup_release_c<beta_offsets_cross_pri_sel_dci_0_2_r17_c>>   beta_offsets_cross_pri0_dci_0_2_r17;
  copy_ptr<setup_release_c<beta_offsets_cross_pri_sel_dci_0_2_r17_c>>   beta_offsets_cross_pri1_dci_0_2_r17;
  map_pattern_r17_e_                                                    map_pattern_r17;
  uint8_t                                                               seq_offset_for_rv_r17 = 0;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 2>, 1, 3>>>   ul_access_cfg_list_dci_0_1_r17;
  copy_ptr<setup_release_c<dyn_seq_of<integer<uint8_t, 0, 64>, 1, 2>>>  minimum_sched_offset_k2_r17;
  copy_ptr<setup_release_c<dmrs_bundling_pusch_cfg_r17_s>>              dmrs_bundling_pusch_cfg_r17;
  uint8_t                                                               harq_process_num_size_dci_0_2_v1700 = 5;
  uint8_t                                                               harq_process_num_size_dci_0_1_r17   = 5;
  copy_ptr<mpe_res_pool_to_add_mod_list_r17_l_>                         mpe_res_pool_to_add_mod_list_r17;
  copy_ptr<mpe_res_pool_to_release_list_r17_l_>                         mpe_res_pool_to_release_list_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RadioLinkMonitoringRS ::= SEQUENCE
struct radio_link_monitoring_rs_s {
  struct purpose_opts {
    enum options { beam_fail, rlf, both, nulltype } value;

    const char* to_string() const;
  };
  using purpose_e_ = enumerated<purpose_opts>;
  struct detection_res_c_ {
    struct types_opts {
      enum options { ssb_idx, csi_rs_idx, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    detection_res_c_() = default;
    detection_res_c_(const detection_res_c_& other);
    detection_res_c_& operator=(const detection_res_c_& other);
    ~detection_res_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx()
    {
      assert_choice_type(types::ssb_idx, type_, "detectionResource");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx()
    {
      assert_choice_type(types::csi_rs_idx, type_, "detectionResource");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_idx() const
    {
      assert_choice_type(types::ssb_idx, type_, "detectionResource");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx() const
    {
      assert_choice_type(types::csi_rs_idx, type_, "detectionResource");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_idx();
    uint8_t& set_csi_rs_idx();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  bool             ext                         = false;
  uint8_t          radio_link_monitoring_rs_id = 0;
  purpose_e_       purpose;
  detection_res_c_ detection_res;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SPS-ConfigMulticastToAddModList-r17 ::= SEQUENCE (SIZE (1..8)) OF SPS-Config
using sps_cfg_multicast_to_add_mod_list_r17_l = dyn_array<sps_cfg_s>;

// SPS-ConfigMulticastToReleaseList-r17 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (0..7)
using sps_cfg_multicast_to_release_list_r17_l = bounded_array<uint8_t, 8>;

// SRS-Config ::= SEQUENCE
struct srs_cfg_s {
  using srs_res_set_to_release_list_l_             = bounded_array<uint8_t, 16>;
  using srs_res_set_to_add_mod_list_l_             = dyn_array<srs_res_set_s>;
  using srs_res_to_release_list_l_                 = dyn_array<uint8_t>;
  using srs_res_to_add_mod_list_l_                 = dyn_array<srs_res_s>;
  using srs_res_set_to_add_mod_list_dci_0_2_r16_l_ = dyn_array<srs_res_set_s>;
  using srs_res_set_to_release_list_dci_0_2_r16_l_ = bounded_array<uint8_t, 16>;
  using srs_pos_res_set_to_release_list_r16_l_     = bounded_array<uint8_t, 16>;
  using srs_pos_res_set_to_add_mod_list_r16_l_     = dyn_array<srs_pos_res_set_r16_s>;
  using srs_pos_res_to_release_list_r16_l_         = dyn_array<uint8_t>;
  using srs_pos_res_to_add_mod_list_r16_l_         = dyn_array<srs_pos_res_r16_s>;

  // member variables
  bool                           ext                      = false;
  bool                           tpc_accumulation_present = false;
  srs_res_set_to_release_list_l_ srs_res_set_to_release_list;
  srs_res_set_to_add_mod_list_l_ srs_res_set_to_add_mod_list;
  srs_res_to_release_list_l_     srs_res_to_release_list;
  srs_res_to_add_mod_list_l_     srs_res_to_add_mod_list;
  // ...
  // group 0
  bool                                                 srs_request_dci_1_2_r16_present = false;
  bool                                                 srs_request_dci_0_2_r16_present = false;
  uint8_t                                              srs_request_dci_1_2_r16         = 1;
  uint8_t                                              srs_request_dci_0_2_r16         = 1;
  copy_ptr<srs_res_set_to_add_mod_list_dci_0_2_r16_l_> srs_res_set_to_add_mod_list_dci_0_2_r16;
  copy_ptr<srs_res_set_to_release_list_dci_0_2_r16_l_> srs_res_set_to_release_list_dci_0_2_r16;
  copy_ptr<srs_pos_res_set_to_release_list_r16_l_>     srs_pos_res_set_to_release_list_r16;
  copy_ptr<srs_pos_res_set_to_add_mod_list_r16_l_>     srs_pos_res_set_to_add_mod_list_r16;
  copy_ptr<srs_pos_res_to_release_list_r16_l_>         srs_pos_res_to_release_list_r16;
  copy_ptr<srs_pos_res_to_add_mod_list_r16_l_>         srs_pos_res_to_add_mod_list_r16;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BeamFailureRecoveryRSConfig-r16 ::= SEQUENCE
struct beam_fail_recovery_rs_cfg_r16_s {
  using candidate_beam_rs_list_r16_l_  = dyn_array<candidate_beam_rs_r16_s>;
  using candidate_beam_rs_list2_r17_l_ = dyn_array<candidate_beam_rs_r16_s>;

  // member variables
  bool                          ext                        = false;
  bool                          rsrp_thres_bfr_r16_present = false;
  uint8_t                       rsrp_thres_bfr_r16         = 0;
  candidate_beam_rs_list_r16_l_ candidate_beam_rs_list_r16;
  // ...
  // group 0
  copy_ptr<candidate_beam_rs_list2_r17_l_> candidate_beam_rs_list2_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// ConfiguredGrantConfigToAddModList-r16 ::= SEQUENCE (SIZE (1..12)) OF ConfiguredGrantConfig
using cfg_grant_cfg_to_add_mod_list_r16_l = dyn_array<cfg_grant_cfg_s>;

// ConfiguredGrantConfigToReleaseList-r16 ::= SEQUENCE (SIZE (1..12)) OF INTEGER (0..11)
using cfg_grant_cfg_to_release_list_r16_l = bounded_array<uint8_t, 12>;

// ConfiguredGrantConfigType2DeactivationStateList-r16 ::= SEQUENCE (SIZE (1..16)) OF
// ConfiguredGrantConfigType2DeactivationState-r16
using cfg_grant_cfg_type2_deactivation_state_list_r16_l = dyn_array<cfg_grant_cfg_type2_deactivation_state_r16_l>;

// CFR-ConfigMulticast-r17 ::= SEQUENCE
struct cfr_cfg_multicast_r17_s {
  bool                                    location_and_bw_multicast_r17_present = false;
  bool                                    pdcch_cfg_multicast_r17_present       = false;
  bool                                    pdsch_cfg_multicast_r17_present       = false;
  uint16_t                                location_and_bw_multicast_r17         = 0;
  pdcch_cfg_s                             pdcch_cfg_multicast_r17;
  pdsch_cfg_s                             pdsch_cfg_multicast_r17;
  sps_cfg_multicast_to_add_mod_list_r17_l sps_cfg_multicast_to_add_mod_list_r17;
  sps_cfg_multicast_to_release_list_r17_l sps_cfg_multicast_to_release_list_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DL-PPW-PreConfig-r17 ::= SEQUENCE
struct dl_ppw_pre_cfg_r17_s {
  struct type_r17_opts {
    enum options { type1_a, type1_b, type2, nulltype } value;

    const char* to_string() const;
  };
  using type_r17_e_ = enumerated<type_r17_opts>;
  struct prio_r17_opts {
    enum options { st1, st2, st3, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using prio_r17_e_ = enumerated<prio_r17_opts>;

  // member variables
  bool                                    type_r17_present = false;
  bool                                    prio_r17_present = false;
  uint8_t                                 dl_ppw_id_r17    = 0;
  dl_ppw_periodicity_and_start_slot_r17_c dl_ppw_periodicity_and_start_slot_r17;
  uint8_t                                 len_r17 = 1;
  type_r17_e_                             type_r17;
  prio_r17_e_                             prio_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// NumberOfMsg3-Repetitions-r17 ::= ENUMERATED
struct nof_msg3_repeats_r17_opts {
  enum options { n1, n2, n3, n4, n7, n8, n12, n16, nulltype } value;
  typedef uint8_t number_type;

  const char* to_string() const;
  uint8_t     to_number() const;
};
using nof_msg3_repeats_r17_e = enumerated<nof_msg3_repeats_r17_opts>;

// NonCellDefiningSSB-r17 ::= SEQUENCE
struct non_cell_defining_ssb_r17_s {
  struct ssb_periodicity_r17_opts {
    enum options { ms5, ms10, ms20, ms40, ms80, ms160, spare2, spare1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ssb_periodicity_r17_e_ = enumerated<ssb_periodicity_r17_opts>;
  struct ssb_time_offset_r17_opts {
    enum options { ms5, ms10, ms15, ms20, ms40, ms80, spare2, spare1, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using ssb_time_offset_r17_e_ = enumerated<ssb_time_offset_r17_opts>;

  // member variables
  bool                   ext                         = false;
  bool                   ssb_periodicity_r17_present = false;
  bool                   ssb_time_offset_r17_present = false;
  uint32_t               absolute_freq_ssb_r17       = 0;
  ssb_periodicity_r17_e_ ssb_periodicity_r17;
  ssb_time_offset_r17_e_ ssb_time_offset_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// PathlossReferenceRS-r17 ::= SEQUENCE
struct pathloss_ref_rs_r17_s {
  struct ref_sig_r17_c_ {
    struct types_opts {
      enum options { ssb_idx, csi_rs_idx, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ref_sig_r17_c_() = default;
    ref_sig_r17_c_(const ref_sig_r17_c_& other);
    ref_sig_r17_c_& operator=(const ref_sig_r17_c_& other);
    ~ref_sig_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx()
    {
      assert_choice_type(types::ssb_idx, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx()
    {
      assert_choice_type(types::csi_rs_idx, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_idx() const
    {
      assert_choice_type(types::ssb_idx, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx() const
    {
      assert_choice_type(types::csi_rs_idx, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_idx();
    uint8_t& set_csi_rs_idx();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  bool           add_pci_r17_present    = false;
  uint8_t        pathloss_ref_rs_id_r17 = 0;
  ref_sig_r17_c_ ref_sig_r17;
  uint8_t        add_pci_r17 = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// RadioLinkMonitoringConfig ::= SEQUENCE
struct radio_link_monitoring_cfg_s {
  using fail_detection_res_to_add_mod_list_l_ = dyn_array<radio_link_monitoring_rs_s>;
  using fail_detection_res_to_release_list_l_ = bounded_array<uint8_t, 10>;
  struct beam_fail_instance_max_count_opts {
    enum options { n1, n2, n3, n4, n5, n6, n8, n10, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using beam_fail_instance_max_count_e_ = enumerated<beam_fail_instance_max_count_opts>;
  struct beam_fail_detection_timer_opts {
    enum options { pbfd1, pbfd2, pbfd3, pbfd4, pbfd5, pbfd6, pbfd8, pbfd10, nulltype } value;
    typedef uint8_t number_type;

    const char* to_string() const;
    uint8_t     to_number() const;
  };
  using beam_fail_detection_timer_e_ = enumerated<beam_fail_detection_timer_opts>;

  // member variables
  bool                                  ext                                  = false;
  bool                                  beam_fail_instance_max_count_present = false;
  bool                                  beam_fail_detection_timer_present    = false;
  fail_detection_res_to_add_mod_list_l_ fail_detection_res_to_add_mod_list;
  fail_detection_res_to_release_list_l_ fail_detection_res_to_release_list;
  beam_fail_instance_max_count_e_       beam_fail_instance_max_count;
  beam_fail_detection_timer_e_          beam_fail_detection_timer;
  // ...
  // group 0
  copy_ptr<beam_fail_detection_r17_s> beam_fail_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// SPS-ConfigDeactivationState-r16 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (0..7)
using sps_cfg_deactivation_state_r16_l = bounded_array<uint8_t, 8>;

// TCI-UL-State-r17 ::= SEQUENCE
struct tci_ul_state_r17_s {
  struct ref_sig_r17_c_ {
    struct types_opts {
      enum options { ssb_idx_r17, csi_rs_idx_r17, srs_r17, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ref_sig_r17_c_() = default;
    ref_sig_r17_c_(const ref_sig_r17_c_& other);
    ref_sig_r17_c_& operator=(const ref_sig_r17_c_& other);
    ~ref_sig_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    uint8_t& ssb_idx_r17()
    {
      assert_choice_type(types::ssb_idx_r17, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    uint8_t& csi_rs_idx_r17()
    {
      assert_choice_type(types::csi_rs_idx_r17, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    uint8_t& srs_r17()
    {
      assert_choice_type(types::srs_r17, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& ssb_idx_r17() const
    {
      assert_choice_type(types::ssb_idx_r17, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& csi_rs_idx_r17() const
    {
      assert_choice_type(types::csi_rs_idx_r17, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    const uint8_t& srs_r17() const
    {
      assert_choice_type(types::srs_r17, type_, "referenceSignal-r17");
      return c.get<uint8_t>();
    }
    uint8_t& set_ssb_idx_r17();
    uint8_t& set_csi_rs_idx_r17();
    uint8_t& set_srs_r17();

  private:
    types               type_;
    pod_choice_buffer_t c;

    void destroy_();
  };

  // member variables
  bool           ext                            = false;
  bool           serving_cell_id_r17_present    = false;
  bool           bwp_id_r17_present             = false;
  bool           add_pci_r17_present            = false;
  bool           ul_pwr_ctrl_r17_present        = false;
  bool           pathloss_ref_rs_id_r17_present = false;
  uint8_t        tci_ul_state_id_r17            = 0;
  uint8_t        serving_cell_id_r17            = 0;
  uint8_t        bwp_id_r17                     = 0;
  ref_sig_r17_c_ ref_sig_r17;
  uint8_t        add_pci_r17            = 1;
  uint8_t        ul_pwr_ctrl_r17        = 1;
  uint8_t        pathloss_ref_rs_id_r17 = 0;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BWP-DownlinkCommon ::= SEQUENCE
struct bwp_dl_common_s {
  bool                                ext                      = false;
  bool                                pdcch_cfg_common_present = false;
  bool                                pdsch_cfg_common_present = false;
  bwp_s                               generic_params;
  setup_release_c<pdcch_cfg_common_s> pdcch_cfg_common;
  setup_release_c<pdsch_cfg_common_s> pdsch_cfg_common;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BWP-UplinkCommon ::= SEQUENCE
struct bwp_ul_common_s {
  using nof_msg3_repeats_list_r17_l_ = std::array<nof_msg3_repeats_r17_e, 4>;
  using mcs_msg3_repeats_r17_l_      = std::array<uint8_t, 8>;

  // member variables
  bool                                ext                      = false;
  bool                                rach_cfg_common_present  = false;
  bool                                pusch_cfg_common_present = false;
  bool                                pucch_cfg_common_present = false;
  bwp_s                               generic_params;
  setup_release_c<rach_cfg_common_s>  rach_cfg_common;
  setup_release_c<pusch_cfg_common_s> pusch_cfg_common;
  setup_release_c<pucch_cfg_common_s> pucch_cfg_common;
  // ...
  // group 0
  bool                                              use_interlace_pucch_pusch_r16_present = false;
  copy_ptr<setup_release_c<rach_cfg_common_s>>      rach_cfg_common_iab_r16;
  copy_ptr<setup_release_c<msg_a_cfg_common_r16_s>> msg_a_cfg_common_r16;
  // group 1
  bool enable_ra_prioritization_for_slicing_r17_present = false;
  bool rsrp_thres_msg3_r17_present                      = false;
  bool enable_ra_prioritization_for_slicing_r17         = false;
  copy_ptr<setup_release_c<dyn_seq_of<add_rach_cfg_r17_s, 1, 256>>> add_rach_cfg_list_r17;
  uint8_t                                                           rsrp_thres_msg3_r17 = 0;
  copy_ptr<nof_msg3_repeats_list_r17_l_>                            nof_msg3_repeats_list_r17;
  copy_ptr<mcs_msg3_repeats_r17_l_>                                 mcs_msg3_repeats_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BWP-UplinkDedicated ::= SEQUENCE
struct bwp_ul_ded_s {
  struct ul_tci_state_list_r17_c_ {
    struct explicitlist_s_ {
      using ul_tci_to_add_mod_list_r17_l_ = dyn_array<tci_ul_state_r17_s>;
      using ul_tci_to_release_list_r17_l_ = dyn_array<uint8_t>;

      // member variables
      ul_tci_to_add_mod_list_r17_l_ ul_tci_to_add_mod_list_r17;
      ul_tci_to_release_list_r17_l_ ul_tci_to_release_list_r17;
    };
    struct types_opts {
      enum options { explicitlist, unified_tci_state_ref_r17, nulltype } value;

      const char* to_string() const;
    };
    using types = enumerated<types_opts>;

    // choice methods
    ul_tci_state_list_r17_c_() = default;
    ul_tci_state_list_r17_c_(const ul_tci_state_list_r17_c_& other);
    ul_tci_state_list_r17_c_& operator=(const ul_tci_state_list_r17_c_& other);
    ~ul_tci_state_list_r17_c_() { destroy_(); }
    void        set(types::options e = types::nulltype);
    types       type() const { return type_; }
    SRSASN_CODE pack(bit_ref& bref) const;
    SRSASN_CODE unpack(cbit_ref& bref);
    void        to_json(json_writer& j) const;
    // getters
    explicitlist_s_& explicitlist()
    {
      assert_choice_type(types::explicitlist, type_, "ul-TCI-StateList-r17");
      return c.get<explicitlist_s_>();
    }
    serving_cell_and_bwp_id_r17_s& unified_tci_state_ref_r17()
    {
      assert_choice_type(types::unified_tci_state_ref_r17, type_, "ul-TCI-StateList-r17");
      return c.get<serving_cell_and_bwp_id_r17_s>();
    }
    const explicitlist_s_& explicitlist() const
    {
      assert_choice_type(types::explicitlist, type_, "ul-TCI-StateList-r17");
      return c.get<explicitlist_s_>();
    }
    const serving_cell_and_bwp_id_r17_s& unified_tci_state_ref_r17() const
    {
      assert_choice_type(types::unified_tci_state_ref_r17, type_, "ul-TCI-StateList-r17");
      return c.get<serving_cell_and_bwp_id_r17_s>();
    }
    explicitlist_s_&               set_explicitlist();
    serving_cell_and_bwp_id_r17_s& set_unified_tci_state_ref_r17();

  private:
    types                                                           type_;
    choice_buffer_t<explicitlist_s_, serving_cell_and_bwp_id_r17_s> c;

    void destroy_();
  };
  using pathloss_ref_rs_to_add_mod_list_r17_l_ = dyn_array<pathloss_ref_rs_r17_s>;
  using pathloss_ref_rs_to_release_list_r17_l_ = dyn_array<uint8_t>;

  // member variables
  bool                                      ext                            = false;
  bool                                      pucch_cfg_present              = false;
  bool                                      pusch_cfg_present              = false;
  bool                                      cfg_grant_cfg_present          = false;
  bool                                      srs_cfg_present                = false;
  bool                                      beam_fail_recovery_cfg_present = false;
  setup_release_c<pucch_cfg_s>              pucch_cfg;
  setup_release_c<pusch_cfg_s>              pusch_cfg;
  setup_release_c<cfg_grant_cfg_s>          cfg_grant_cfg;
  setup_release_c<srs_cfg_s>                srs_cfg;
  setup_release_c<beam_fail_recovery_cfg_s> beam_fail_recovery_cfg;
  // ...
  // group 0
  bool                                                        cp_ext_c2_r16_present                 = false;
  bool                                                        cp_ext_c3_r16_present                 = false;
  bool                                                        use_interlace_pucch_pusch_r16_present = false;
  copy_ptr<setup_release_c<pucch_cfg_s>>                      sl_pucch_cfg_r16;
  uint8_t                                                     cp_ext_c2_r16 = 1;
  uint8_t                                                     cp_ext_c3_r16 = 1;
  copy_ptr<setup_release_c<dyn_seq_of<pucch_cfg_s, 1, 2>>>    pucch_cfg_list_r16;
  copy_ptr<setup_release_c<lbt_fail_recovery_cfg_r16_s>>      lbt_fail_recovery_cfg_r16;
  copy_ptr<cfg_grant_cfg_to_add_mod_list_r16_l>               cfg_grant_cfg_to_add_mod_list_r16;
  copy_ptr<cfg_grant_cfg_to_release_list_r16_l>               cfg_grant_cfg_to_release_list_r16;
  copy_ptr<cfg_grant_cfg_type2_deactivation_state_list_r16_l> cfg_grant_cfg_type2_deactivation_state_list_r16;
  // group 1
  bool                                                     ul_pwr_ctrl_r17_present = false;
  copy_ptr<ul_tci_state_list_r17_c_>                       ul_tci_state_list_r17;
  uint8_t                                                  ul_pwr_ctrl_r17 = 1;
  copy_ptr<setup_release_c<dyn_seq_of<pucch_cfg_s, 1, 2>>> pucch_cfg_list_multicast1_r17;
  copy_ptr<setup_release_c<dyn_seq_of<pucch_cfg_s, 1, 2>>> pucch_cfg_list_multicast2_r17;
  // group 2
  copy_ptr<setup_release_c<pucch_cfg_s>> pucch_cfg_multicast1_r17;
  copy_ptr<setup_release_c<pucch_cfg_s>> pucch_cfg_multicast2_r17;
  // group 3
  copy_ptr<pathloss_ref_rs_to_add_mod_list_r17_l_> pathloss_ref_rs_to_add_mod_list_r17;
  copy_ptr<pathloss_ref_rs_to_release_list_r17_l_> pathloss_ref_rs_to_release_list_r17;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// DL-PPW-PreConfigToAddModList-r17 ::= SEQUENCE (SIZE (1..4)) OF DL-PPW-PreConfig-r17
using dl_ppw_pre_cfg_to_add_mod_list_r17_l = dyn_array<dl_ppw_pre_cfg_r17_s>;

// DL-PPW-PreConfigToReleaseList-r17 ::= SEQUENCE (SIZE (1..4)) OF INTEGER (0..15)
using dl_ppw_pre_cfg_to_release_list_r17_l = bounded_array<uint8_t, 4>;

// SPS-ConfigDeactivationStateList-r16 ::= SEQUENCE (SIZE (1..16)) OF SPS-ConfigDeactivationState-r16
using sps_cfg_deactivation_state_list_r16_l = dyn_array<sps_cfg_deactivation_state_r16_l>;

// SPS-ConfigToAddModList-r16 ::= SEQUENCE (SIZE (1..8)) OF SPS-Config
using sps_cfg_to_add_mod_list_r16_l = dyn_array<sps_cfg_s>;

// SPS-ConfigToReleaseList-r16 ::= SEQUENCE (SIZE (1..8)) OF INTEGER (0..7)
using sps_cfg_to_release_list_r16_l = bounded_array<uint8_t, 8>;

// BWP-DownlinkDedicated ::= SEQUENCE
struct bwp_dl_ded_s {
  bool                                         ext                               = false;
  bool                                         pdcch_cfg_present                 = false;
  bool                                         pdsch_cfg_present                 = false;
  bool                                         sps_cfg_present                   = false;
  bool                                         radio_link_monitoring_cfg_present = false;
  setup_release_c<pdcch_cfg_s>                 pdcch_cfg;
  setup_release_c<pdsch_cfg_s>                 pdsch_cfg;
  setup_release_c<sps_cfg_s>                   sps_cfg;
  setup_release_c<radio_link_monitoring_cfg_s> radio_link_monitoring_cfg;
  // ...
  // group 0
  copy_ptr<sps_cfg_to_add_mod_list_r16_l>                    sps_cfg_to_add_mod_list_r16;
  copy_ptr<sps_cfg_to_release_list_r16_l>                    sps_cfg_to_release_list_r16;
  copy_ptr<sps_cfg_deactivation_state_list_r16_l>            sps_cfg_deactivation_state_list_r16;
  copy_ptr<setup_release_c<beam_fail_recovery_rs_cfg_r16_s>> beam_fail_recovery_scell_cfg_r16;
  copy_ptr<setup_release_c<pdcch_cfg_s>>                     sl_pdcch_cfg_r16;
  copy_ptr<setup_release_c<pdcch_cfg_s>>                     sl_v2x_pdcch_cfg_r16;
  // group 1
  bool                                                       pre_conf_gap_status_r17_present                  = false;
  bool                                                       harq_feedback_enablingfor_sp_sactive_r17_present = false;
  bool                                                       serving_cell_mo_r17_present                      = false;
  fixed_bitstring<8>                                         pre_conf_gap_status_r17;
  copy_ptr<setup_release_c<beam_fail_recovery_rs_cfg_r16_s>> beam_fail_recovery_sp_cell_cfg_r17;
  bool                                                       harq_feedback_enablingfor_sp_sactive_r17 = false;
  copy_ptr<setup_release_c<cfr_cfg_multicast_r17_s>>         cfr_cfg_multicast_r17;
  copy_ptr<dl_ppw_pre_cfg_to_add_mod_list_r17_l>             dl_ppw_pre_cfg_to_add_mod_list_r17;
  copy_ptr<dl_ppw_pre_cfg_to_release_list_r17_l>             dl_ppw_pre_cfg_to_release_list_r17;
  copy_ptr<non_cell_defining_ssb_r17_s>                      non_cell_defining_ssb_r17;
  uint8_t                                                    serving_cell_mo_r17 = 1;

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BWP-Uplink ::= SEQUENCE
struct bwp_ul_s {
  bool            ext                = false;
  bool            bwp_common_present = false;
  bool            bwp_ded_present    = false;
  uint8_t         bwp_id             = 0;
  bwp_ul_common_s bwp_common;
  bwp_ul_ded_s    bwp_ded;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BWP-Downlink ::= SEQUENCE
struct bwp_dl_s {
  bool            ext                = false;
  bool            bwp_common_present = false;
  bool            bwp_ded_present    = false;
  uint8_t         bwp_id             = 0;
  bwp_dl_common_s bwp_common;
  bwp_dl_ded_s    bwp_ded;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BWP-UplinkDedicatedSDT-r17 ::= SEQUENCE
struct bwp_ul_ded_sdt_r17_s {
  bool                                ext                   = false;
  bool                                pusch_cfg_r17_present = false;
  setup_release_c<pusch_cfg_s>        pusch_cfg_r17;
  cfg_grant_cfg_to_add_mod_list_r16_l cfg_grant_cfg_to_add_mod_list_r17;
  cfg_grant_cfg_to_release_list_r16_l cfg_grant_cfg_to_release_list_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

// BWP-DownlinkDedicatedSDT-r17 ::= SEQUENCE
struct bwp_dl_ded_sdt_r17_s {
  bool                         ext                   = false;
  bool                         pdcch_cfg_r17_present = false;
  bool                         pdsch_cfg_r17_present = false;
  setup_release_c<pdcch_cfg_s> pdcch_cfg_r17;
  setup_release_c<pdsch_cfg_s> pdsch_cfg_r17;
  // ...

  // sequence methods
  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
  void        to_json(json_writer& j) const;
};

} // namespace rrc_nr
} // namespace asn1
