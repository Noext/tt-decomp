/* The SIF: the EE's bus to the IOP (see sif.h for the protocol and where it comes from).

   The game lays out its own command state at 0x790058 (func_500B88/0x500C10..0x500C38):

     +0x00  pktbuf       0x2078FF80   the EE's receive buffer (uncached)
     +0x04  unused       0x20790000
     +0x0C  sys_handlers 0x790080     32 entries of {handler, harg}
     +0x10  nr_sys       0x20
     +0x1C  sregs        0x790200     the software registers, read by func_500B40

   This file keeps only what the IOP needs: the SIF registers, the queue of packets the EE sent,
   and the DMAC handlers. Everything it does to the game happens through packets in guest memory —
   the game's own handler at func_5010F0 reads them and calls its own command handlers.
 */
#include "sif.h"
#include "ps2asm.h"
#include "../runtime/memory.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern int g_trace_sif;
extern uint32_t hle_call_guest(uint32_t, uint32_t, uint32_t, uint32_t,
                                uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define CMD_DATA_AT 0x00790058u   /* the game's struct cmd_data */
#define SIF0_CHANNEL 5            /* AddDmacHandler(5, ...) in func_500CE8 */
#define MAX_QUEUED 16

static uint32_t sif_reg[8];
static uint32_t dmac_handler[16];
static int dmac_enabled[16];

/* The packets the EE sent, waiting for the IOP to run them. Each is copied out of guest memory
   when the DMA is queued, the way a real transfer would have moved the bytes. */
typedef struct { uint32_t cid, pkt, size; } queued_cmd;
static queued_cmd queue[MAX_QUEUED];
static int queue_head, queue_tail;

/* One reply waiting to be delivered through SIF0. */
static struct { uint32_t buf, size; int pending; } reply;

static uint32_t guest_word(uint32_t at) { uint32_t v; memcpy(&v, (void *)(uintptr_t)ps2_ram_canonical(at), 4); return v; }
static void guest_write(uint32_t at, const void *src, uint32_t n)
{
    memcpy((void *)(uintptr_t)ps2_ram_canonical(at), src, n);
}

void sif_init(void)
{
    memset(sif_reg, 0, sizeof sif_reg);
    memset(dmac_handler, 0, sizeof dmac_handler);
    memset(dmac_enabled, 0, sizeof dmac_enabled);
    queue_head = queue_tail = 0;
    reply.pending = 0;
    /* The IOP side is up: SIF and its command layer have initialised. */
    sif_reg[SIF_REG_SMFLAG] = SIF_STAT_SIFINIT | SIF_STAT_CMDINIT | SIF_STAT_BOOTEND;
}

int sif_add_dmac_handler(int channel, uint32_t handler)
{
    if (channel < 0 || channel >= 16) return 0;
    dmac_handler[channel] = handler;
    if (g_trace_sif)
        fprintf(stderr, "SIF: DMAC channel %d handler 0x%08x\n", channel, handler);
    return channel;
}

void sif_enable_dmac(int channel)
{
    if (channel >= 0 && channel < 16) dmac_enabled[channel] = 1;
}

void sif_set_reg(uint32_t reg, uint32_t value)
{
    if (reg < 8) sif_reg[reg] = value;
    if (g_trace_sif)
        fprintf(stderr, "SIF: SetReg(%u, 0x%08x)\n", reg, value);
}

uint32_t sif_get_reg(uint32_t reg)
{
    uint32_t v = reg < 8 ? sif_reg[reg] : 0;
    if (g_trace_sif)
        fprintf(stderr, "SIF: GetReg(%u) -> 0x%08x\n", reg, v);
    return v;
}

int sif_set_dma(uint32_t dmat, int count)
{
    /* SifDmaTransfer_t { void *src; void *dest; int size; int attr; } — 16 bytes each. */
    for (int i = 0; i < count; i++) {
        uint32_t e = dmat + (uint32_t)i * 16;
        uint32_t src = guest_word(e), dst = guest_word(e + 4);
        uint32_t size = guest_word(e + 8);
        if (!size || size > 0x10000) continue;
        /* The bytes really move, as a DMA would: guest memory to guest memory. */
        memmove((void *)(uintptr_t)ps2_ram_canonical(dst),
                (void *)(uintptr_t)ps2_ram_canonical(src), size);
        if (g_trace_sif)
            fprintf(stderr, "SIF: SetDma src=0x%08x dst=0x%08x size=%u\n", src, dst, size);
    }
    /* The first entry's destination is the IOP's receive buffer: that packet is a command. */
    if (count > 0) {
        uint32_t e = dmat;
        uint32_t src = guest_word(e), dst = guest_word(e + 4), size = guest_word(e + 8);
        uint32_t cid = guest_word(dst + 8);          /* header->cid is at +8 */
        if (size >= 16 && queue_tail - queue_head < MAX_QUEUED) {
            queue[queue_tail++ % MAX_QUEUED] = (queued_cmd){cid, src, size};
        }
    }
    return 1;   /* a transfer id: the SDK's callers treat > 0 as queued */
}

int sif_dma_stat(int id)
{
    (void)id;
    return -1;   /* 0xFFFFFFFF: the transfer has completed */
}

/* The IOP's half: run one queued command and prepare its reply. */
static void iop_run_command(const queued_cmd *c)
{
    uint32_t cid = c->cid & ~SIF_CMD_ID_SYSTEM;
    if (g_trace_sif)
        fprintf(stderr, "IOP: command 0x%08x (cid %u) from 0x%08x\n", c->cid, cid, c->pkt);

    if (c->cid == SIF_CMD_INIT_CMD) {
        /* sceSifInitCmd: answer with SIF_CMD_SET_SREG {index: 0, value: 1}. That is what makes
           sceSifGetSreg(0) (func_500B40) non-zero, which is the loop the boot is stuck in. */
        uint32_t pkt = 0;
        /* Where to put it: the EE's receive buffer, from its own cmd_data. */
        if (guest_word(CMD_DATA_AT) != 0)
            pkt = guest_word(CMD_DATA_AT);              /* pktbuf */
        if (!pkt) return;
        uint32_t buf[4];
        buf[0] = 16u;                                   /* psize = 16, dsize = 0 */
        buf[1] = 0;                                     /* dest */
        buf[2] = SIF_CMD_SET_SREG;                      /* cid 0x80000001 */
        buf[3] = 0;                                     /* opt */
        uint32_t body[2] = { SIF_SREG_RPCINIT, 1 };     /* index 0, value 1 */
        guest_write(pkt, buf, 16);
        guest_write(pkt + 16, body, 8);
        reply.buf = pkt;
        reply.size = 24;
        reply.pending = 1;
        sif_reg[SIF_REG_SMFLAG] |= SIF_STAT_CMDINIT;
    }
}

void sif_poll(void)
{
    if (queue_head != queue_tail) {
        iop_run_command(&queue[queue_head % MAX_QUEUED]);
        queue_head++;
    }
    if (reply.pending) {
        reply.pending = 0;
        uint32_t h = dmac_handler[SIF0_CHANNEL];
        if (h) {
            if (g_trace_sif) fprintf(stderr, "SIF: deliver 0x%08x to handler 0x%08x\n", reply.buf, h);
            hle_call_guest(h, SIF0_CHANNEL, reply.buf, reply.size, 0, 0, 0, 0, 0);
        }
    }
}

int sif_busy(void) { return queue_head != queue_tail || reply.pending; }
