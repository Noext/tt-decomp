/* What the port's runtime and HLE expect from the rest of the port, on a plain Linux host.

   Each of these is a hook downhill-revived's Windows host fills with something richer (a Rust
   renderer, a profiler, a state trace). Here they are the smallest implementations that let the
   recompiled game, its VU microcode and the GS run. */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fenv.h>
#include <stdarg.h>

#include "host_api.h"
#include "../runtime/ps2asm.h"

extern double host_wall_seconds(void);
extern uint64_t host_entry_call(uint32_t);

void hle_log(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fflush(stderr);
}

/* The resolver the kernel uses to turn a guest entry address into host code. The port's build
   fills it; a host that has not linked the recompiled image leaves it empty. */
uint64_t (*ps2_host_entry)(uint32_t guest_address);
uint64_t host_entry_call(uint32_t guest_address)
{
    return ps2_host_entry ? ps2_host_entry(guest_address) : 0;
}

/* The EE and VU floating-point mode: round toward zero with flush-to-zero, as the PS2 does. */
static unsigned fp_mode = 0x7F80;
unsigned host_fp_csr(void) { return fp_mode; }
void host_fp_set_csr(unsigned csr) { fp_mode = csr; }

/* The GS. vif.c calls into it when a GIF packet completes. */
void graphics_provenance(const void *p) { (void)p; }
void graphics_xgkick(const void *p) { (void)p; }
void graphics_gif_feed(const void *p, unsigned size) { (void)p; (void)size; }

/* Symbols the runtime and the VU interpreter expect from the port's build. */
uint32_t gs_presented_frames;
uint64_t kernel_vsync_count;
uint64_t host_cycles(void) { return (uint64_t)(host_wall_seconds() * 147456000.0); }

/* The kernel resolves a guest entry through this name. */
