#ifndef JBPF_SRSRAN_CONTEXTS_H
#define JBPF_SRSRAN_CONTEXTS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "jbpf_helper_api_defs_ext.h"

typedef enum {
    JBPF_PROG_TYPE_SRSRAN_OFH = CUSTOM_PROGRAM_START_ID,
    JBPF_PROG_TYPE_SRSRAN_LAYER2,
} jbpf_srsran_index_e;

/* OFH context*/
struct jbpf_ran_ofh_ctx {
  uint64_t data; /* Pointer to beginning of buffer with int16_t IQ samples */
  uint64_t data_end; /* Pointer to end+1 of packet */
  uint64_t meta_data; /* Used for the program to store metadata */
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
    uint16_t ctx_id; /* Context id (could be implementation specific) */
    uint16_t frame; /* 3GPP frame number */
    uint16_t slot; /* 3GPP slot number */
    uint16_t cell_id; /* Cell id */
};

#ifdef __cplusplus
}
#endif

#endif // JBPF_SRSRAN_CONTEXTS_H
