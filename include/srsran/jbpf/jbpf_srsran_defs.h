#ifndef JBPF_SRSRAN_DEFS_H_
#define JBPF_SRSRAN_DEFS_H_

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define JBPF_NUM_OF_LCG       8
#define JBPF_MAX_UE_SUPPORTED (16*5)

struct output_socket
{
    int sockfd;
    struct sockaddr_in server_addr;
};

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
#endif