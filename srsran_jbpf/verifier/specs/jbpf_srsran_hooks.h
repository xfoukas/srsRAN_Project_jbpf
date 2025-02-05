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

DECLARE_JBPF_HOOK(mac_bsr_update,
		   struct jbpf_ran_layer2_ctx ctx,
		   ctx,
		   HOOK_PROTO(
               struct jbpf_bsr_update *bsr,
                  uint16_t ctx_id,
                  uint16_t frame,
                  uint16_t slot,
                  uint16_t cell_id
			      ),
		   HOOK_ASSIGN(
			       ctx.ctx_id = ctx_id;
			       ctx.cell_id = cell_id;
			       ctx.slot = slot;
			       ctx.frame = frame;
			       ctx.data = (uint64_t)(void *) bsr;
			       ctx.data_end = (uint64_t) ((uint8_t*)bsr + sizeof(struct jbpf_bsr_update));
			       )
		   )

#ifdef __cplusplus
}
#endif


#endif // JBPF_SRSRAN_HOOKS_
