#ifndef JBPF_SRSRAN_CONTEXTS_H
#define JBPF_SRSRAN_CONTEXTS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "jbpf_helper_api_defs_ext.h"

typedef enum {
    JBPF_PROG_TYPE_RAN_OFH = CUSTOM_PROGRAM_START_ID,
    JBPF_PROG_TYPE_RAN_LAYER2,
    JBPF_PROG_TYPE_RAN_MAC_SCHED,
    JBPF_PROG_TYPE_RAN_GENERIC    
} jbpf_srsran_index_e;

/* OFH context*/
struct jbpf_ran_ofh_ctx {
  uint64_t data; /* Pointer to beginning of buffer with int16_t IQ samples */
  uint64_t data_end; /* Pointer to end+1 of packet */
  uint64_t meta_data; /* Used by ebpf */
  /* Combination of frame, slot and cell_id, provide a unique
     context for an execution pipeline */
  uint16_t ctx_id; /* Context id (could be implementation specific) */
  uint8_t direction; /* 0 DL, 1 UL */
};

/* L2 context*/
struct jbpf_ran_layer2_ctx {
    uint64_t data; /* Pointer to beginning of variable-sized L2 struct */
    uint64_t data_end; /* Pointer to end+1 of variable-sized struct */
    uint64_t meta_data; /* Used for the program to store metadata */
    uint16_t ctx_id; 
    uint16_t frame; /* 3GPP frame number */
    uint16_t slot; /* 3GPP slot number */
    uint16_t cell_id; /* Cell id */
};

/* MAC Scheduler context */
struct jbpf_mac_sched_ctx {
    uint64_t data; /* Pointer to beginning of variable-sized L2 struct */
    uint64_t data_end; /* Pointer to end+1 of variable-sized struct */
    uint64_t meta_data; /* Used by ebpf */
    uint64_t srs_meta_data1; /* Used for the program to store metadata */
    uint16_t ctx_id; /* Context id (could be implementation specific) */
    uint16_t cell_id; /* Cell id */
    uint16_t rnti; /* UE RNTI */
};

/* PDCP context */
struct jbpf_pdcp_ctx_info {
    uint16_t ctx_id;   /* Context id (could be implementation specific) */
    uint32_t ue_index;
    uint8_t is_srb; /* true=srb, false=drb */
    uint8_t rb_id;   /* 0=srb0, 1=srb1, 2=srb2, 3=srb3, 
                                1=drb1, 2=drb2, 3-drb3 ... */
    uint8_t rlc_mode;  /* 0=UM, 1=AM*/
};

/* E1 context info */
struct jbpf_e1_ctx_info {
    uint16_t ctx_id;   /* Context id (could be implementation specific) */
    uint64_t ue_index; 
    uint64_t gnb_cu_cp_ue_e1ap_id; 
    uint64_t gnb_cu_up_ue_e1ap_id; 
};

/* CUCP UE context info */
struct jbpf_cucp_uemgr_ctx_info {
    uint16_t ctx_id;    /* Context id (could be implementation specific) */
    uint16_t du_index;  
    uint32_t plmn;      /* (mcc << 16) || mnc */ 
    uint64_t ue_index; 
};

/* RRC context info */
struct jbpf_rrc_ctx_info {
    uint16_t ctx_id;    /* Context id (could be implementation specific) */
    uint64_t ue_index; 
};

/* RLC context info */

typedef enum {
    JBPF_RLC_MODE_TM = 1,
    JBPF_RLC_MODE_UM,
    JBPF_RLC_MODE_AM,
    JBPF_RLC_MODE_MAX
} JbpfRlcMode_t;

struct jbpf_rlc_ctx_info {
    uint16_t ctx_id;    /* Context id (could be implementation specific) */
    uint64_t gnb_du_id;
    uint16_t ue_index; 
    uint8_t is_srb;  /* true=srb, false=drb */
    uint8_t rb_id;   /* 0=srb0, 1=srb1, 2=srb2, 3=srb3, 
                                1=drb1, 2=drb2, 3-drb3 ... */
    JbpfRlcMode_t rlc_mode;  /* 0=TM, 1=UM, 2=AM*/
};

/* srsRAN generic context */
struct jbpf_ran_generic_ctx {
    uint64_t data; /* Pointer to beginning of buffer with int16_t IQ samples */
    uint64_t data_end; /* Pointer to end+1 of packet */
    uint64_t meta_data; /* Used by ebpf */
    uint64_t srs_meta_data1; /* Used for the program to store metadata */
    uint64_t srs_meta_data2; /* Used for the program to store metadata */
    uint64_t srs_meta_data3; /* Used for the program to store metadata */
    uint64_t srs_meta_data4; /* Used for the program to store metadata */
};
  




#ifdef __cplusplus
}
#endif

#endif // JBPF_SRSRAN_CONTEXTS_H
