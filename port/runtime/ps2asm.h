/* PS2 inline asm runtime: the emulated EE/VU0 register state that the game's inline asm
   (rewritten into ps2_asm() calls by port/tools/transform.py) works on. */
#ifndef PS2ASM_H
#define PS2ASM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    uint8_t b[16];
    uint16_t h[8];
    int16_t sh[8];
    uint32_t w[4];
    int32_t sw[4];
    uint64_t d[2];
    float f[4];
} ps2_q128;

/* Registers 0..31 are the real ones; 32..63 stand for asm operands %0..%31 bound by constraint. */
typedef struct {
    ps2_q128 gpr[64];
    uint64_t lo, hi;
    uint64_t lo1, hi1; /* upper halves of the 128-bit LO/HI (MMI) */
    float fpr[64];
    float fpu_acc;
    ps2_q128 vf[32]; /* vf0 always reads (0, 0, 0, 1) */
    float vacc[4];
    uint16_t vi[16];
    float q, i;
    uint32_t r; /* VU0 random register */
    uint8_t *vu0_code; /* VU0 micro memory, 4 KB: at its PS2 address 0x11000000 in the game */
    ps2_q128 *vu0_mem; /* VU0 data memory, 4 KB: at its PS2 address 0x11004000 in the game
                          (port/runtime/memory.c), where the game also writes it */
    uint32_t cop0[32]; /* system control registers: Status (12) for the interrupt switches */
    uint32_t vu0_ctrl[16]; /* VU0 control registers vi16..vi31 (cfc2/ctc2), apart from R, I, Q */
    float p; /* VU1 elementary-function result */
} ps2_cpu;

extern ps2_cpu ps2;
extern void (*ps2_mmio_write)(uintptr_t address, uint64_t value, unsigned size);
extern void (*ps2_graphics_sync)(void);
/* The game also reaches RAM through two other views of the same bytes: the low view
   [0x00400000, 0x02000000) (PS2 physical addresses) and the uncached-accelerated mirror
   [0x38000000, 0x3a000000). Each host maps them onto canonical RAM at 0x08000000 + (address &
   0x1ffffff): the x64/Linux hosts by OS views, folded per access out of line
   (port/host64/memory_access.h), and the browser by the same constant folds (port/web/postlink.py).
   Recompiled code and the runtime's helpers take the canonical address here instead: the same
   bytes, but the access stays on the hosts' inline path (finding 090 SP14: about 70% of the x64
   slow-path calls, 48k per race field). Only the module's guest addresses fold; native test
   builds pass host pointers. */
/* ============================ the guest address space ============================

   ONE translation function, and every access goes through it.

   The EE's address space is not flat: the same 32 MiB of RAM appears through six windows, the
   hardware registers through two, and the scratchpad sits at a fixed address. The guest hands
   the port any of those forms, so an access has to be classified before it can be a host pointer.

   ---------------------------------------------------------------------------------------------
   Guest range                Size      What it is                         Host mapping
   ---------------------------------------------------------------------------------------------
   0x00000000-0x01FFFFFF      32 MiB    RAM, physical / KUSEG              the mapped block
   0x10000000-0x1000FFFF     64 KiB     EE hardware registers, cached      at its own address
   0x11000000-0x1100FFFF     64 KiB     VU0/VU1 micro and data memory      at its own address
   0x12000000-0x12001FFF      8 KiB     GS registers, privileged           at its own address
   0x20000000-0x21FFFFFF      32 MiB    RAM, uncached (KSEG1)              -> 0x08000000 block
   0x30000000-0x31FFFFFF      32 MiB    RAM, uncached accelerated          -> 0x08000000 block
   0x70000000-0x70003FFF     16 KiB     scratchpad                         at its own address
   0x80000000-0x81FFFFFF      32 MiB    RAM, KSEG0 (kernel view)           -> 0x08000000 block
   0xA0000000-0xA1FFFFFF      32 MiB    RAM, KSEG1 (kernel uncached)       -> 0x08000000 block
   0xB0000000-0xB000FFFF     64 KiB     EE hardware registers, uncached    at its own address
   0x38000000-0x39FFFFFF      32 MiB    RAM, the third view the port maps -> 0x08000000 block
   0xC0000000-0xC1FFFFFF      32 MiB    RAM, KSEG1 accelerated             -> 0x08000000 block
   ---------------------------------------------------------------------------------------------

   WHY 0x80076000 BECOMES 0x08076000 AND NOT 0x00076000

   0x80076000 is the kernel's view of physical address 0x00076000 — the same byte the game reaches
   at 0x00076000. The host keeps exactly ONE image of RAM, mapped at 0x08000000 + physical (that
   base is port/tools/data.py's PS2_RAM_BASE, and data files hold absolute PS2 pointers that land
   there). Folding to 0x00076000 instead would be wrong twice over: nothing is mapped there (the
   loader and a non-PIE binary own the low 4 MiB), and if something were, it would be a second
   copy of RAM that diverges from the mapped one on the first write.

   The fold is `0x08000000 + (guest & 0x01FFFFFF)` — mask, then add. The mask is what removes the
   window: 0x80076000 & 0x01FFFFFF = 0x00076000, + 0x08000000 = 0x08076000. Identical arithmetic
   for all six RAM windows, which is why one mask covers them.

   Everything else keeps its address on the host, because the port maps those pages there: the
   hardware registers at 0x10000000 and their uncached mirror at 0xB0000000, VU memory at
   0x11000000, the GS at 0x12000000, the scratchpad at 0x70000000. That is also what makes the
   game's own pointer arithmetic work — it compares, stores and hands these addresses to DMA.

   Anything outside the table is a bug: an un-initialised register, a 64-bit host pointer that
   leaked into guest state, or a wrong constant. `ps2_guest_address` classifies and aborts, naming
   the address, instead of dereferencing it. PS2_AUDIT adds the report at the top of the call.
   ============================================================================= */

#define PS2_RAM_HOST_BASE   0x08000000u
#define PS2_RAM_BYTES       0x02000000u
#define PS2_MASK_32BIT      0xFFFFFFFFu

/* The kind of memory a guest address names. */
enum ps2_region {
    PS2_REGION_RAM,          /* any of the six windows of the 32 MiB block */
    PS2_REGION_MMIO,         /* hardware registers, VU memory, GS */
    PS2_REGION_SCRATCHPAD
};

typedef struct {
    enum ps2_region region;
    uintptr_t host;          /* the address to read or write on this host */
    uint32_t physical;       /* the address the guest itself would call physical */
} ps2_mapping;

#if defined(PS2_AUDIT)
extern void ps2_audit_fail(const char *what, uintptr_t address);
#define PS2_AUDIT_ADDR(what, a) ps2_audit_fail(what, (uintptr_t)(a))
#else
#define PS2_AUDIT_ADDR(what, a) ((void)0)
#endif

/* An address outside every window: a port bug, never guest data. */
void ps2_address_fault(uintptr_t address) __attribute__((noreturn));

/* Classifies one guest address. This is the only place that knows the map. */
static inline ps2_mapping ps2_guest_address(uintptr_t address)
{
    ps2_mapping m;
    uint32_t a = (uint32_t)address & PS2_MASK_32BIT;

    /* Anything that did not fit in 32 bits is a host pointer that leaked into guest state. */
    if (address > PS2_MASK_32BIT)
        ps2_address_fault(address);

    if (a < PS2_RAM_BYTES) {                       /* 0x00000000 KUSEG RAM */
        m.region = PS2_REGION_RAM; m.physical = a; m.host = PS2_RAM_HOST_BASE + a; return m;
    }
    if (a - 0x20000000u < PS2_RAM_BYTES) {         /* 0x20000000 uncached */
        m.physical = a & 0x1FFFFFFFu; m.region = PS2_REGION_RAM;
        m.host = PS2_RAM_HOST_BASE + m.physical; return m;
    }
    if (a - 0x30000000u < PS2_RAM_BYTES) {         /* 0x30000000 uncached accelerated */
        m.physical = a & 0x0FFFFFFFu; m.region = PS2_REGION_RAM;
        m.host = PS2_RAM_HOST_BASE + m.physical; return m;
    }
    if (a - 0x80000000u < PS2_RAM_BYTES) {         /* 0x80000000 KSEG0 */
        m.physical = a & 0x1FFFFFFFu; m.region = PS2_REGION_RAM;
        m.host = PS2_RAM_HOST_BASE + m.physical; return m;
    }
    if (a - 0xA0000000u < PS2_RAM_BYTES) {         /* 0xA0000000 KSEG1 */
        m.physical = a & 0x1FFFFFFFu; m.region = PS2_REGION_RAM;
        m.host = PS2_RAM_HOST_BASE + m.physical; return m;
    }
    if (a - 0xC0000000u < PS2_RAM_BYTES) {         /* 0xC0000000 KSEG1 accelerated */
        m.physical = a & 0x0FFFFFFFu; m.region = PS2_REGION_RAM;
        m.host = PS2_RAM_HOST_BASE + m.physical; return m;
    }
    if (a - 0x38000000u < PS2_RAM_BYTES) {         /* 0x38000000: the port maps the block here */
        m.region = PS2_REGION_RAM; m.physical = a - 0x30000000u;
        m.host = a - 0x30000000u; return m;
    }
    if (a - 0x10000000u < 0x00010000u) {           /* EE registers, cached */
        m.region = PS2_REGION_MMIO; m.physical = a; m.host = a; return m;
    }
    if (a - 0xB0000000u < 0x00010000u) {           /* EE registers, uncached */
        m.region = PS2_REGION_MMIO; m.physical = a & 0x0FFFFFFFu; m.host = a; return m;
    }
    if (a - 0x11000000u < 0x00010000u) {           /* VU0 + VU1 memory */
        m.region = PS2_REGION_MMIO; m.physical = a; m.host = a; return m;
    }
    if (a - 0x12000000u < 0x00002000u) {           /* GS registers */
        m.region = PS2_REGION_MMIO; m.physical = a; m.host = a; return m;
    }
    if (a - 0x70000000u < 0x00004000u) {           /* scratchpad */
        m.region = PS2_REGION_SCRATCHPAD; m.physical = a; m.host = a; return m;
    }
    ps2_address_fault(address);                    /* not classified: a bug, not a slow path */
}

/* The host address of a guest address, classified. */
static inline uintptr_t ps2_ram_canonical(uintptr_t address)
{
    return ps2_guest_address(address).host;
}

/* Fence mapped VU1/VIF1/GS state before the EE reads or changes it, then translate. */
static inline uintptr_t ps2_memory_address(uintptr_t address)
{
    if (ps2_graphics_sync) {
        uint32_t a = (uint32_t)address;
        if ((a >= 0x11008000 && a < 0x11010000) ||
            (a >= 0x10003C00 && a < 0x10004000) ||
            (a >= 0x10005000 && a < 0x10005010) ||
            (a >= 0x10003000 && a < 0x10003030) ||
            (a >= 0x12001000 && a < 0x12001100))
            ps2_graphics_sync();
    }
    return ps2_guest_address(address).host;
}

/* INTC_STAT (0x1000F000) holds the EE's pending interrupt causes. The game reads it only to wait
   for VBLANK_START (func_0025AB70, func_0025AC00: sceGsSyncV), so a read lets the kernel run
   time up to the vblank first. */
#define PS2_INTC_STAT 0x1000F000u
extern void (*ps2_intc_read)(void);
static inline uintptr_t ps2_load_address(uintptr_t address)
{
    if (address == PS2_INTC_STAT && ps2_intc_read)
        ps2_intc_read();
    return ps2_memory_address(address);
}
void ps2_store32(uintptr_t address, uint32_t value);
void ps2_store64(uintptr_t address, uint64_t value);
void ps2_store128(uintptr_t address, const ps2_q128 *value);

/* A VU0 microprogram started with vcallms; addr is the byte address in micro memory. */
typedef void (*ps2_vu0_micro_fn)(ps2_cpu *cpu);
void ps2_vu0_register_micro(unsigned addr, ps2_vu0_micro_fn fn);

/* Registers the native microprograms of port/runtime/vu0_micro.c; call once at startup. */
void ps2_vu0_init(void);

/* Runs the microcode in VU0 micro memory from addr (bytes) to its end (vu0_interp.c), for a
   microprogram with no native version. Returns 0, or -1 on an instruction VU0 doesn't have. */
int ps2_vu0_run(ps2_cpu *cpu, unsigned addr);

/* The translated VU blocks and native kernels (vu0_interp.c, PS2_VU_AOT) run only where micro
   memory holds the microcode they were translated from. Their copy of it is taken here from the
   game's .vutext (vutext: PS2 address 0x263BC0, read from the player's disc), not kept in
   dhd.exe. Once, before any VU code runs; nothing without PS2_VU_AOT. */
void ps2_vu_aot_load(const void *vutext);

/* VU1's registers, which VU0 micro code reads at data address 0x4000 (set by the VIF1 HLE),
   the fence that makes them current, and that 1 KB view as VF0..31 then VI0..31 per quadword. */
extern ps2_cpu *ps2_vu1_regs;
extern uint64_t ps2_vu1_code_version;  /* bumped when VIF1 MPG changes VU1 micro memory */
extern uint64_t ps2_vu0_code_version;  /* bumped when VIF0 MPG changes VU0 micro memory, or the native words load */
extern int ps2_vu1_native_mode;  /* -1: DHD_VU1_NATIVE decides; 0 or 1 forces the native kernels off or on (tests) */
extern int ps2_vu1_aot_mode;  /* -1: DHD_VU_INTERPRETER decides; 0 or 1 forces the VU1 blocks off or on (tests) */
extern int ps2_vu0_native_mode;  /* same override for DHD_VU0_NATIVE */
extern int ps2_vu0_compiled_mode;  /* same override for DHD_VU0_COMPILED (vu0_aot.inc, PS2_VU0_COMPILED builds) */
extern unsigned long long ps2_vu0_compiled_runs;
extern unsigned long long ps2_vu0_native_runs;
extern void (*ps2_vu1_regs_sync)(void);
void ps2_vu0_vu1_file(ps2_q128 file[64]);

/* VU1 has its own register file; vu0_mem/code point to its 16 KB memories.
   TPC (vu0_ctrl[10]) records the continuation byte address for MSCNT. */
typedef int (*ps2_vu_kick_fn)(void *opaque, unsigned qword);
/* Optional diagnostic hook reports visible CLIP at the XGKICK instruction. */
typedef void (*ps2_vu_kick_trace_fn)(unsigned pc, unsigned clip, unsigned qword);
void ps2_vu1_set_kick_trace(ps2_vu_kick_trace_fn trace);
int ps2_vu1_run(ps2_cpu *cpu, unsigned addr, unsigned top, unsigned itop,
                ps2_vu_kick_fn kick, void *opaque);
/* Native-loop coverage, indexed by the dispatch kind's bit number. */
enum { PS2_VU1_NATIVE_KINDS = 14 };
extern unsigned long long ps2_vu1_native_runs[PS2_VU1_NATIVE_KINDS];


/* One instruction of statically recompiled MIPS code (port/tools/recomp.py), e.g.
   "vmulx.xyz $vf1, $vf2, $vf3x". Unlike ps2_asm it may read the argument registers. */
void ps2_exec(const char *insn);
int ps2_exec_check(const char *insn); /* parse only; returns the number of problems */
void ps2_vcallms(const char *tmpl, unsigned addr); /* recompiled vcallms; tmpl names the site */

/* Count of templates that failed to parse or ran something unsupported (bring-up diagnostics). */
int ps2_asm_problems(void);

/* Parse a template without running it; returns the number of problems found. */
int ps2_asm_check(const char *tmpl, const char *constraints, int n);

#ifdef __cplusplus
}
#endif

#endif
