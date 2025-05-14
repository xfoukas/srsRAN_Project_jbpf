#ifndef JBPF_SRSRAN_HOOKS_
#define JBPF_SRSRAN_HOOKS_

/*
 * UE Indexing Across Subsystems
 *
 * This comment outlines how UE (User Equipment) indexes are managed and tracked across 
 * various subsystems (DU, CU-CP, and CU-UP).
 *
 * All functions invoked as hooks for UE-specific operations will include a "UE index".
 * However, the DU, CU-CP, and CU-UP subsystems manage their UE indexes independently.
 * As a result, the same UE may have different index values in each subsystem.
 *
 * To clarify this in the code, the context structures use the following field names:
 *     - du_ue_index
 *     - cu_cp_ue_index
 *     - cu_up_ue_index
 *
 * Mapping UE indexes across subsystems involves the following conventions:
 *
 * DU (Distributed Unit):
 * -----------------------
 * The function `du_ue_ctx_creation` is called when a UE is created at the DU.
 * Parameters:
 *     - du_ue_index
 *     - plmn
 *     - nci
 *     - pci
 *     - tac
 *     - crnti
 *
 * CU-CP (Central Unit - Control Plane):
 * -------------------------------------
 * The function `cucp_uemgr_ue_add` is called when a UE is created at the CU-CP.
 * Parameters:
 *     - cu_cp_ue_index
 *     - plmn
 *     - pci
 *     - rnti
 *
 * The `cu_cp_ue_index` can be mapped to a `du_ue_index` based on matching (plmn, pci, rnti).
 *
 * CU-UP (Central Unit - User Plane):
 * ----------------------------------
 * The UE context in the CU-UP is set up via the E1AP Bearer Setup procedure.
 * The following hooks are invoked:
 *
 *     - `e1_cucp_bearer_context_setup`
 *         Parameters: cu_cp_ue_index, gnb_cu_cp_ue_e1ap_id
 *
 *     - `e1_cuup_bearer_context_setup`
 *         Parameters: cu_cp_ue_index, gnb_cu_cp_ue_e1ap_id, gnb_cu_up_ue_e1ap_id
 *
 * Mapping details:
 *     - `gnb_cu_cp_ue_e1ap_id` corresponds to `cu_cp_ue_index`
 *     - (`gnb_cu_cp_ue_e1ap_id`, `gnb_cu_up_ue_e1ap_id`) together identify the `cu_up_ue_index`
 *
 * PDCP (Packet Data Convergence Protocol):
 * ----------------------------------------
 * PDCP hooks are invoked in both CU-CP (for SRBs) and CU-UP (for DRBs), and share a unified 
 * context structure:
 *
 *     struct jbpf_pdcp_ctx_info {
 *         uint16_t ctx_id;       // Context ID (implementation specific)
 *         uint32_t cu_ue_index;  // SRB: cu_cp_ue_index, DRB: cu_up_ue_index
 *         uint8_t  is_srb;       // true = SRB, false = DRB
 *         uint8_t  rb_id;        // SRB: 0=srb0, 1=srb1, 2=srb2
 *                                // DRB: 1=drb1, 2=drb2, 3=drb3, etc.
 *         uint8_t  rlc_mode;     // 0 = UM, 1 = AM
 *     };
 *
 * Notes:
 *     - In CU-CP, hooks are called for SRBs. Thus, `cu_ue_index` refers to `cu_cp_ue_index`.
 *     - In CU-UP, hooks are called for DRBs. Thus, `cu_ue_index` refers to `cu_up_ue_index`.
 */



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



// DU UE creation/deletion

DECLARE_JBPF_HOOK(du_ue_ctx_creation,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_du_ue_ctx_info *ue_info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)ue_info;
        ctx.data_end = (uint64_t) ((uint8_t*)ue_info + sizeof(struct jbpf_du_ue_ctx_info));
    )
)

DECLARE_JBPF_HOOK(du_ue_ctx_update_crnti,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_du_ue_ctx_info *ue_info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)ue_info;
        ctx.data_end = (uint64_t) ((uint8_t*)ue_info + sizeof(struct jbpf_du_ue_ctx_info));
    )
)

DECLARE_JBPF_HOOK(du_ue_ctx_deletion,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_du_ue_ctx_info *ue_info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)ue_info;
        ctx.data_end = (uint64_t) ((uint8_t*)ue_info + sizeof(struct jbpf_du_ue_ctx_info));
    )
)


// MAC scheduler

#define DECLARE_MAC_SCHED_HOOK(name) \
    DECLARE_JBPF_HOOK(name, \
        struct jbpf_mac_sched_ctx ctx, \
        ctx, \
        HOOK_PROTO( \
            void *ind, \
            uint16_t ctx_id, \
            uint16_t du_ue_index, \
            uint16_t cell_id, \
            uint16_t rnti, \
            size_t payload_size \
        ), \
        HOOK_ASSIGN( \
            ctx.ctx_id = ctx_id; \
            ctx.du_ue_index = du_ue_index; \
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
            uint16_t du_ue_index, \
            uint16_t cell_id, \
            uint16_t rnti \
        ), \
        HOOK_ASSIGN( \
            ctx.ctx_id = ctx_id; \
            ctx.du_ue_index = du_ue_index; \
            ctx.data = (uint64_t)0; \
            ctx.data_end = (uint64_t)0; \
        ) \
    )

DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(mac_sched_ue_creation);
DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(mac_sched_ue_reconfig);
DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(mac_sched_ue_deletion);
DECLARE_MAC_SCHED_HOOK_NO_PAYLOAD(mac_sched_ue_config_applied);
DECLARE_MAC_SCHED_HOOK(mac_sched_ul_bsr_indication);           /* ctx.data will be a srsran::ul_bsr_indication_message */
DECLARE_MAC_SCHED_HOOK(mac_sched_crc_indication);              /* ctx.data will be a srsran::ul_crc_pdu_indication */
DECLARE_MAC_SCHED_HOOK(mac_sched_uci_indication);              /* ctx.data will be a srsran::uci_indication::uci_pdu */
DECLARE_MAC_SCHED_HOOK(mac_sched_dl_mac_ce_indication);        /* ctx.data will be a srsran::dl_mac_ce_indication */
DECLARE_MAC_SCHED_HOOK(mac_sched_ul_phr_indication);           /* ctx.data will be a srsran::ul_phr_indication_message */
DECLARE_MAC_SCHED_HOOK(mac_sched_dl_buffer_state_indication);  /* ctx.data will be a srsran::dl_buffer_state_indication_message */
DECLARE_MAC_SCHED_HOOK(mac_sched_srs_indication);              /* ctx.data will be a srsran::srs_indication::srs_indication_pdu */



// PDCP

// trigger: when PDCP DL entity is created
DECLARE_JBPF_HOOK(pdcp_dl_creation,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
    )
)

// trigger: when PDCP DL entity is deleted
DECLARE_JBPF_HOOK(pdcp_dl_deletion,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
    )
)

// trigger: when new SDU is received from higher layers
DECLARE_JBPF_HOOK(pdcp_dl_new_sdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t sdu_length,
        uint32_t count,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)sdu_length << 32 | count;
        ctx.srs_meta_data2 = window_size;
    )
)

// trigger: when a PDU is sent to lower layers
DECLARE_JBPF_HOOK(pdcp_dl_tx_data_pdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t sdu_length,
        uint32_t count,
        uint8_t is_retx,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)sdu_length << 32 | count;
        ctx.srs_meta_data2 = (uint64_t)is_retx << 32 | window_size;
    )
)

// trigger: when a control PDU is sent to lower layers
DECLARE_JBPF_HOOK(pdcp_dl_tx_control_pdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t sdu_length,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)sdu_length << 32 | window_size;
    )
)

// trigger: when the first byte of an SDU is sent to lower layers
DECLARE_JBPF_HOOK(pdcp_dl_handle_tx_notification,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t notif_count,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)notif_count << 32 | window_size;
    )
)

// RLC TM/UM mode, trigger: when the SDU is completely sent to lower layers.
// RLM AM mode, trigger: when all of the PDU used to transmit an SDU have 
// been acknowledged by the UE.
DECLARE_JBPF_HOOK(pdcp_dl_handle_delivery_notification,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t notif_count,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)notif_count << 32 | window_size;
    )
)

// trigger: when an SDU is discarded by the PDCP layer
DECLARE_JBPF_HOOK(pdcp_dl_discard_pdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t count,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)count << 32 | window_size;
    )
)

// trigger: when a PDCP bearer is re-established
DECLARE_JBPF_HOOK(pdcp_dl_reestablish,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
    )
)

// trigger: when a PDU is received from lower layers
DECLARE_JBPF_HOOK(pdcp_ul_rx_data_pdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t sdu_length,
        uint32_t header_length,
        uint32_t count,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)sdu_length << 32 | header_length;
        ctx.srs_meta_data2 = (uint64_t)count << 32 | window_size;
    )
)

// trigger: when a control PDU is received from lower layers
DECLARE_JBPF_HOOK(pdcp_ul_rx_control_pdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t pdu_length,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)pdu_length << 32 | window_size;
    )
)

// trigger: when PDCP UL entity is created
DECLARE_JBPF_HOOK(pdcp_ul_creation,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
    )
)

// trigger: when PDCP UL entity is deleted
DECLARE_JBPF_HOOK(pdcp_ul_deletion,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
    )
)

// trigger: when an SDU is delivered to higher layers
DECLARE_JBPF_HOOK(pdcp_ul_deliver_sdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer,
        uint32_t sdu_length,
        uint32_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)sdu_length << 32 | window_size;
    )
)

// trigger: when a PDCP bearer is re-established
DECLARE_JBPF_HOOK(pdcp_ul_reestablish,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_pdcp_ctx_info *bearer),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)bearer;
        ctx.data_end = (uint64_t) ((uint8_t*)bearer + sizeof(struct jbpf_pdcp_ctx_info));
    )
)


 // E1AP

DECLARE_JBPF_HOOK(e1_cucp_bearer_context_setup,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cucp_e1_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cucp_e1_ctx_info));
    )
)

DECLARE_JBPF_HOOK(e1_cucp_bearer_context_modification,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cucp_e1_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cucp_e1_ctx_info));
    )
)

DECLARE_JBPF_HOOK(e1_cucp_bearer_context_release,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cucp_e1_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cucp_e1_ctx_info));
    )
)

DECLARE_JBPF_HOOK(e1_cucp_bearer_context_inactivity,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cucp_e1_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cucp_e1_ctx_info));
    )
)

DECLARE_JBPF_HOOK(e1_cuup_bearer_context_setup,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cuup_e1_ctx_info *info,
        uint8_t success),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cuup_e1_ctx_info));
        ctx.srs_meta_data1 = success;
    )
)

DECLARE_JBPF_HOOK(e1_cuup_bearer_context_modification,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cuup_e1_ctx_info *info,
        uint8_t success),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cuup_e1_ctx_info));
        ctx.srs_meta_data1 = success;
    )
)

DECLARE_JBPF_HOOK(e1_cuup_bearer_context_release,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cuup_e1_ctx_info *info,
        uint8_t success),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cuup_e1_ctx_info));
        ctx.srs_meta_data1 = success;
    )
)


/* CUCP UE Manager */

DECLARE_JBPF_HOOK(cucp_uemgr_ue_add,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cucp_uemgr_ctx_info *info,
        bool pci_set,
        uint16_t pci,
        bool rnti_set,
        uint16_t rnti),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cucp_uemgr_ctx_info));
        ctx.srs_meta_data1 = ((uint64_t)pci_set) << 16 | pci;
        ctx.srs_meta_data2 = (uint64_t)rnti_set << 16 | rnti;
    )
)

DECLARE_JBPF_HOOK(cucp_uemgr_ue_update,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cucp_uemgr_ctx_info *info,
        uint16_t pci,
        uint16_t rnti),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cucp_uemgr_ctx_info));
        ctx.srs_meta_data1 = pci;
        ctx.srs_meta_data2 = rnti;
    )
)

DECLARE_JBPF_HOOK(cucp_uemgr_ue_remove,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_cucp_uemgr_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_cucp_uemgr_ctx_info));
    )
)


/* RRC */

DECLARE_JBPF_HOOK(rrc_ue_add,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rrc_ctx_info *info,
        uint16_t c_rnti,
        uint16_t pci,
        uint32_t tac,
        uint32_t plmn,
        uint64_t nci),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rrc_ctx_info));
        ctx.srs_meta_data1 = ((uint64_t)c_rnti << 48) | ((uint64_t)pci << 32) | tac;
        ctx.srs_meta_data2 = plmn;
        ctx.srs_meta_data3 = nci;
    )
)

DECLARE_JBPF_HOOK(rrc_ue_update_context,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rrc_ctx_info *info,
        uint64_t old_cu_ue_index,
        uint16_t c_rnti,
        uint16_t pci,
        uint32_t tac,
        uint32_t plmn,
        uint64_t nci),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rrc_ctx_info));
        ctx.srs_meta_data1 = old_cu_ue_index;
        ctx.srs_meta_data2 = ((uint64_t)c_rnti << 48) | ((uint64_t)pci << 32) | tac;
        ctx.srs_meta_data3 = plmn;
        ctx.srs_meta_data4 = nci;
    )
)

DECLARE_JBPF_HOOK(rrc_ue_update_id,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rrc_ctx_info *info,
        uint64_t _5gtimsi),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rrc_ctx_info));
        ctx.srs_meta_data1 =_5gtimsi;
    )
)

DECLARE_JBPF_HOOK(rrc_ue_remove,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rrc_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rrc_ctx_info));
   )
)

typedef enum {
    RRC_SETUP = 1,
    RRC_RECONFIGURATION,
    RRC_REESTABLISHMENT,
    RRC_UE_CAPABILITY,
    RRC_PROCEDURE_MAX
} JbpfRrcProcedure_t;

DECLARE_JBPF_HOOK(rrc_ue_procedure_started,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rrc_ctx_info *info,
        JbpfRrcProcedure_t procedure,
        uint64_t meta),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rrc_ctx_info));
        ctx.srs_meta_data1 = procedure;
        ctx.srs_meta_data2 = meta;
    )
)

DECLARE_JBPF_HOOK(rrc_ue_procedure_completed,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rrc_ctx_info *info,
        JbpfRrcProcedure_t procedure,
        bool success,
        uint64_t meta),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rrc_ctx_info));
        ctx.srs_meta_data1 = ((uint64_t)success << 32) | procedure;
        ctx.srs_meta_data2 = meta;
    )
)

// RLC

typedef enum {
    JBPF_RLC_PDUTYPE_STATUS = 1,
    JBPF_RLC_PDUTYPE_DATA,
    JBPF_RLC_PDUTYPE_DATA_RETX,
    JBPF_RLC_PDUTYPE_MAX
} JbpfRlcPdu_t;

// trigger: when RLC DL entity is created
DECLARE_JBPF_HOOK(rlc_dl_creation,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
    )
)

// trigger: when RLC DL entity is deleted
DECLARE_JBPF_HOOK(rlc_dl_deletion,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
    )
)

// trigger: new SDU received from upper layer
DECLARE_JBPF_HOOK(rlc_dl_new_sdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint32_t sdu_length,
        uint32_t pdcp_sn),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)sdu_length << 32 | pdcp_sn;
    )
)

// trigger: SDU discarded
DECLARE_JBPF_HOOK(rlc_dl_discard_sdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint32_t pdcp_sn),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = pdcp_sn;
    )
)

// trigger: transmission of the SDU starts
DECLARE_JBPF_HOOK(rlc_dl_sdu_send_started,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint32_t pdcp_sn,
        bool is_retx),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)pdcp_sn << 32 | is_retx;
    )
)

// trigger: all bytes of the SDU have been transmitted
DECLARE_JBPF_HOOK(rlc_dl_sdu_send_completed,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint32_t pdcp_sn,
        bool is_retx),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)pdcp_sn << 32 | is_retx;
    )
)

// trigger: all bytes of the SDU have been received by the peer RLC entity
DECLARE_JBPF_HOOK(rlc_dl_sdu_delivered,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint32_t pdcp_sn,
        bool is_retx),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)pdcp_sn << 32 | is_retx;
    )
)

// trigger: when a PDU is delivered to lower layers for transmission
DECLARE_JBPF_HOOK(rlc_dl_tx_pdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        JbpfRlcPdu_t pdu_type,
        uint32_t pdu_len,
        uint16_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)pdu_type << 32 | pdu_len;
        ctx.srs_meta_data2 = window_size;
    )
)

// trigger: when a STATUS PDU is received from lower layers
DECLARE_JBPF_HOOK(rlc_dl_rx_status,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint16_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = window_size;
    )
)


// trigger: when the retx count of an AM PDU is updated.
DECLARE_JBPF_HOOK(rlc_dl_am_tx_pdu_retx_count,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint16_t window_size,
        uint32_t sn,
        uint32_t retx_count),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = window_size;
        ctx.srs_meta_data2 = (uint64_t)sn << 32 | retx_count;
    )
)

// trigger: when the retx count of an AM PDU reaches the maximum
DECLARE_JBPF_HOOK(rlc_dl_am_tx_pdu_max_retx_count_reached,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint16_t window_size,
        uint32_t sn,
        uint32_t retx_count),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = window_size;
        ctx.srs_meta_data2 = (uint64_t)sn << 32 | retx_count;
    )
)

// trigger: when RLC UL entity is created
DECLARE_JBPF_HOOK(rlc_ul_creation,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
    )
)

// trigger: when RLC UL entity is deleted
DECLARE_JBPF_HOOK(rlc_ul_deletion,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
    )
)

// trigger: when a PDU is received from lower layers
DECLARE_JBPF_HOOK(rlc_ul_rx_pdu,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        JbpfRlcPdu_t pdu_type,
        uint32_t pdu_len,
        uint16_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)pdu_type << 32 | pdu_len;
        ctx.srs_meta_data2 = window_size;
    )
)

// trigger: when a PDU is received for an SDU for which no bytes have previously been received.
DECLARE_JBPF_HOOK(rlc_ul_sdu_recv_started,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint32_t sn,
        uint16_t window_size),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)sn << 32 | window_size;
    )
)

// trigger: when an SDU is delivered to higher layers
DECLARE_JBPF_HOOK(rlc_ul_sdu_delivered,
    struct jbpf_ran_generic_ctx ctx,
    ctx,
    HOOK_PROTO(
        struct jbpf_rlc_ctx_info *info,
        uint32_t sn,
        uint16_t window_size,
        uint32_t sdu_length),
    HOOK_ASSIGN(
        ctx.data = (uint64_t)info;
        ctx.data_end = (uint64_t) ((uint8_t*)info + sizeof(struct jbpf_rlc_ctx_info));
        ctx.srs_meta_data1 = (uint64_t)sn << 32 | window_size;
        ctx.srs_meta_data2 = sdu_length;
    )
)


#ifdef __cplusplus
}
#endif


#endif // JBPF_SRSRAN_HOOKS_
