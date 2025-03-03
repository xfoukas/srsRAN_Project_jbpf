#ifndef JBPF_SRSRAN_HOOKS_
#define JBPF_SRSRAN_HOOKS_

#ifdef __cplusplus
extern "C"
{
#endif

#include "jbpf_hook.h"
#include "jbpf_srsran_defs.h"
#include "jbpf_srsran_contexts.h"

DECLARE_JBPF_HOOK(capture_xran_packet,
		   struct jbpf_ran_ofh_ctx ctx,
		   ctx,
		   HOOK_PROTO(
			      const void* xran_packet,
			      uint32_t packet_len, 
			      uint16_t ctx_id,
				  uint8_t direction
			      ),
		   HOOK_ASSIGN(
			       ctx.ctx_id = ctx_id;
			       ctx.data = (uint64_t)xran_packet;
			       ctx.data_end = (uint64_t) ((uint8_t *)xran_packet + packet_len);
				   ctx.direction = direction;
			       )
		   )



#define DECLARE_JBPF_FAPI_HOOK(name) \
    DECLARE_JBPF_HOOK(name, \
        struct jbpf_ran_layer2_ctx ctx, \
        ctx, \
        HOOK_PROTO( \
            void *ind, \
            uint16_t ctx_id, \
            uint16_t cell_id, \
            size_t payload_size \
        ), \
        HOOK_ASSIGN( \
            ctx.ctx_id = ctx_id; \
            ctx.cell_id = cell_id; \
            ctx.data = (uint64_t)ind; \
            ctx.data_end = (uint64_t) ((uint8_t*)ind + payload_size); \
        ) \
    )

// FAPI phy-to-mac hooks
DECLARE_JBPF_FAPI_HOOK(fapi_rx_data_indication);
DECLARE_JBPF_FAPI_HOOK(fapi_crc_indication);
DECLARE_JBPF_FAPI_HOOK(fapi_uci_indication);
DECLARE_JBPF_FAPI_HOOK(fapi_srs_indication);
DECLARE_JBPF_FAPI_HOOK(fapi_rach_indication);

// FAPI mac-to-phy hooks
DECLARE_JBPF_FAPI_HOOK(fapi_dl_tti_request);
DECLARE_JBPF_FAPI_HOOK(fapi_ul_tti_request);
DECLARE_JBPF_FAPI_HOOK(fapi_ul_dci_request);
DECLARE_JBPF_FAPI_HOOK(fapi_tx_data_request);




// MAC scheduler

#define DECLARE_MAC_SCHED_HOOK(name) \
    DECLARE_JBPF_HOOK(name, \
        struct jbpf_mac_sched_ctx ctx, \
        ctx, \
        HOOK_PROTO( \
            void *ind, \
            uint16_t ctx_id, \
            uint16_t cell_id, \
            uint16_t rnti, \
            size_t payload_size \
        ), \
        HOOK_ASSIGN( \
            ctx.ctx_id = ctx_id; \
            ctx.data = (uint64_t)ind; \
            ctx.data_end = (uint64_t) ((uint8_t*)ind + payload_size); \
        ) \
    )

#define DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(name) \
    DECLARE_JBPF_HOOK(name, \
        struct jbpf_mac_sched_ctx ctx, \
        ctx, \
        HOOK_PROTO( \
            uint16_t ctx_id, \
            uint16_t cell_id, \
            uint16_t rnti \
        ), \
        HOOK_ASSIGN( \
            ctx.ctx_id = ctx_id; \
            ctx.data = (uint64_t)0; \
            ctx.data_end = (uint64_t)0; \
        ) \
    )

DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(mac_sched_ue_creation);
DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(mac_sched_ue_reconfig);
DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(mac_sched_ue_deletion);
DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(mac_sched_ue_config_applied);
DECLARE_MAC_SCHED_HOOK(mac_sched_ul_bsr_indication);
DECLARE_MAC_SCHED_HOOK(mac_sched_crc_indication);
DECLARE_MAC_SCHED_HOOK(mac_sched_uci_indication);
DECLARE_MAC_SCHED_HOOK(mac_sched_dl_mac_ce_indication);
DECLARE_MAC_SCHED_HOOK(mac_sched_ul_phr_indication);
DECLARE_MAC_SCHED_HOOK(mac_sched_dl_buffer_state_indication);
DECLARE_MAC_SCHED_HOOK(mac_sched_srs_indication);



#ifdef __cplusplus
}
#endif


#endif // JBPF_SRSRAN_HOOKS_
