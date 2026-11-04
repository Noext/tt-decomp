/* The SIF: the EE's bus to the IOP.

   The game's SDK init talks to the IOP with command packets over SIF0 (IOP -> EE) and SIF1
   (EE -> IOP). The layout and the command ids come from ps2sdk (common/include/sifcmd-common.h,
   ee/kernel/src/sifcmd.c) and were confirmed against the game's own code:

     func_500CE8   AddDmacHandler(DMAC_SIF0=5, func_5010F0, 0); EnableDmac(5)
     func_5010F0   reads cmd_data->pktbuf (0x790058), dispatches by header->cid, then
                   zeroes header->psize and re-arms the DMA chain
     func_501070   builds SifCmdHeader_t {psize|dsize, dest, cid, opt} and calls SifSetDma
     func_500B40   sceSifGetSreg(n) -> cmd_data->sregs[n] at 0x790200
     func_5012E8   sceSifInitCmd: sends SIF_CMD_INIT_CMD (0x80000002) and waits for sreg[0]

   What the IOP does with a command is this file's job. It is modelled, not emulated: the packet
   the game built in guest memory is read, the command is executed, and a reply packet is written
   into the EE's receive buffer and delivered through the DMAC handler the game registered.

   Nothing here writes a value the game is waiting for into the game's own structures. The reply is
   a real packet, sent the way the IOP would send it; the game's handler processes it.
 */
#ifndef SIF_H
#define SIF_H

#include <stdint.h>

/* The SIF registers (iop/system/sifman/include/sifman.h). */
enum {
    SIF_REG_MAINADDR = 1,
    SIF_REG_SUBADDR  = 2,
    SIF_REG_MSFLAG   = 3,
    SIF_REG_SMFLAG   = 4
};
#define SIF_STAT_SIFINIT 0x10000u
#define SIF_STAT_CMDINIT 0x20000u
#define SIF_STAT_BOOTEND 0x40000u

/* Command ids (common/include/sifcmd-common.h). */
#define SIF_CMD_ID_SYSTEM 0x80000000u
#define SIF_CMD_CHANGE_SADDR (SIF_CMD_ID_SYSTEM | 0u)
#define SIF_CMD_SET_SREG     (SIF_CMD_ID_SYSTEM | 1u)
#define SIF_CMD_INIT_CMD     (SIF_CMD_ID_SYSTEM | 2u)
#define SIF_CMD_RESET_CMD    (SIF_CMD_ID_SYSTEM | 3u)

#define SIF_SREG_RPCINIT 0

/* The packet header, in guest memory: psize is the whole packet size in bytes (1..112), dsize the
   payload, dest where the payload goes on the other side, cid the command. */
typedef struct {
    uint32_t psize_dsize;   /* psize : 8 (low), dsize : 24 (high) */
    uint32_t dest;
    uint32_t cid;
    uint32_t opt;
} sif_cmd_header;

/* The EE's command state, as ps2sdk's sceSifInitCmd lays it out (struct cmd_data). The game has
   its own copy at 0x790058; these are the addresses it holds. */
typedef struct {
    uint32_t pktbuf;      /* receive buffer: where the IOP writes its packets */
    uint32_t unused;
    uint32_t iopbuf;      /* the IOP's receive buffer: where the EE's packets go */
    uint32_t sys_handlers;
    uint32_t nr_sys;
    uint32_t usr_handlers;
    uint32_t nr_usr;
    uint32_t sregs;       /* the software registers the game reads with sceSifGetSreg */
} sif_cmd_data;

/* Registers a DMA handler for a channel (AddDmacHandler, 0x12). Channel 5 is SIF0. */
int sif_add_dmac_handler(int channel, uint32_t handler);
void sif_enable_dmac(int channel);

/* SIF register access: sceSifSetReg (0x79), sceSifGetReg (0x7A). */
void sif_set_reg(uint32_t reg, uint32_t value);
uint32_t sif_get_reg(uint32_t reg);

/* sceSifSetDma (0x77): queue the transfer; returns a transfer id (>0) as the SDK does. */
int sif_set_dma(uint32_t dmat, int count);

/* sceSifDmaStat (0x76): 0 or -1 once done; the port completes transfers at the safe point. */
int sif_dma_stat(int id);

/* Called at each safe point, in order: run the IOP's work for whatever was queued, then deliver
   any reply through the DMAC handler the game registered. */
void sif_poll(void);

/* Non-zero while the SIF has work queued (the kernel's idle loop uses it). */
int sif_busy(void);

void sif_init(void);

#endif
