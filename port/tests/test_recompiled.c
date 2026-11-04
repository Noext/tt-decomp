/* Does a genuinely recompiled EU function run on the HLE? This one is from the EU image. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "recomp.h"
#include <stdarg.h>
void kernel_init(void); void kernel_run(uint32_t entry);
ps2_cpu ps2;
void (*ps2_intc_read)(void);
void (*ps2_graphics_sync)(void);
void (*ps2_mmio_write)(uintptr_t, uint64_t, unsigned);
/* The guest address space: RAM at 0x08000000 + (addr & 0x01FFFFFF), as memory.h says. */
#define RAM_BASE 0x08000000u
static uintptr_t canon(uintptr_t a) { return RAM_BASE + (a & 0x01FFFFFFu); }
void ps2_store32(uintptr_t a, uint32_t v) { *(uint32_t *)canon(a) = v; }
void ps2_store64(uintptr_t a, uint64_t v) { *(uint64_t *)canon(a) = v; }

static uint8_t *ram;
/* The emulated stack pointer is a GUEST address, and the stack itself must be guest memory:
   the recompiled code stores to ADDR(29, 0) through ps2_store64. */
void ps2_recomp_enter(void) { if (!ps2.gpr[29].d[0]) ps2.gpr[29].d[0] = 0x00100000u; }
typedef uint64_t (*guest_entry_fn)(void);
/* func_003AA338 from the EU image: calls func_003AA2E0(1, 0xFFFF). */
void rc_func_003AA2E0(void) { /* the callee, stubbed: records its arguments */ }
void rc_func_003AA338(void);
guest_entry_fn hle_resolve_entry(uint32_t g) { (void)g; return 0; }

/* Load the compiled object's code by including it. */
#include "/root/tt-eu/tt-eu/build/port/recomp/func_003AA338.c"

int main(void)
{
    /* The guest's RAM must be AT its PS2 base: a 32-bit guest pointer is added to it. */
    ram = mmap((void *)0x08000000u, 32u<<20, PROT_READ|PROT_WRITE,
               MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE, -1, 0);
    if (ram == MAP_FAILED) { perror("mmap guest RAM at 0x08000000"); return 1; }
    printf("guest RAM at %p\n", (void *)ram);
    /* canary: the first store must land here */
    ((volatile uint32_t *)ram)[0] = 0xDEADBEEF;
    printf("canary at guest 0: %08x\n", ((volatile uint32_t *)ram)[0]);
    memset(&ps2, 0, sizeof ps2);
    kernel_init();
    printf("calling the recompiled EU function directly...\n");
    printf("sp before call: 0x%llx\n", (unsigned long long)ps2.gpr[29].d[0]);
    unsigned r = func_003AA338(0,0,0,0,0,0,0,0);
    printf("sp after call: 0x%llx\n", (unsigned long long)ps2.gpr[29].d[0]);
    printf("returned 0x%08x\n", r);
    return 0;
}
