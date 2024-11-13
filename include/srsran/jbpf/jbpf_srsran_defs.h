#ifndef JBPF_SRSRAN_DEFS_H_
#define JBPF_SRSRAN_DEFS_H_

#include <stdint.h>

#define JBPF_NUM_OF_LCG       8
#define JBPF_MAX_UE_SUPPORTED (16*5)

// lc = logical channel id. 28.321 Dl: 6.2.1-1 and UL: 6.2.1-2
// lcg = logical channel group.  Can be a max of 8 LCG.
// Each logical channel is assigned to an LCG.
// BSR is reported per LCG.
// Assume that bsr_net is number of UL bytes at UE 
struct jbpf_bsr_report
{
    uint16_t rnti;
    uint16_t ue_index;
    uint32_t  lcg_bitmask;                        
    uint32_t bsr_net[JBPF_NUM_OF_LCG];

};

// HOOK NAME: mac_bsr_update
struct jbpf_bsr_update
{
    uint8_t num_bsr_report;
    struct jbpf_bsr_report bsr_report[JBPF_MAX_UE_SUPPORTED];
};

/* OFH context*/
struct janus_ran_ofh_ctx {
  uint64_t data; /* Pointer to beginning of buffer with int16_t IQ samples */
  uint64_t data_end; /* Pointer to end+1 of packet */
  uint64_t meta_data; /* Used for the program to store metadata */
  /* Combination of frame, slot and cell_id, provide a unique
     context for an execution pipeline */
  uint16_t ctx_id; /* Context id (could be implementation specific) */
  uint8_t direction; /* 0 DL, 1 UL */
};

/* L2 context*/
struct janus_ran_layer2_ctx {
    uint64_t data; /* Pointer to beginning of variable-sized L2 struct */
    uint64_t data_end; /* Pointer to end+1 of variable-sized struct */
    uint64_t meta_data; /* Used for the program to store metadata */
    uint16_t ctx_id; /* Context id (could be implementation specific) */
    uint16_t frame; /* 3GPP frame number */
    uint16_t slot; /* 3GPP slot number */
    uint16_t cell_id; /* Cell id */
};

#endif