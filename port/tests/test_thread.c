/* Runs a host function through the kernel: proves fibers, register save and return value. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "recomp.h"
void kernel_init(void);
void kernel_run(uint32_t entry);
ps2_cpu ps2;
void ps2_store32(uintptr_t a, uint32_t v) { *(uint32_t *)a = v; }
void ps2_store64(uintptr_t a, uint64_t v) { *(uint64_t *)a = v; }
static uint8_t guest_stack[1 << 20];
void ps2_recomp_enter(void) { if (!ps2.gpr[29].d[0]) ps2.gpr[29].d[0] = (uint32_t)(uintptr_t)(guest_stack + sizeof guest_stack - 16); }

static long calls;
static uint64_t target(void) { calls++; ps2.gpr[2].d[0] = 0x1234; return 0x1234; }
/* The test plays the host's resolver: guest address 0x100000 is this host function. */
typedef uint64_t (*guest_entry_fn)(void);
guest_entry_fn hle_resolve_entry(uint32_t guest) { return guest == 0x100000 ? target : 0; }

int main(void)
{
    memset(&ps2, 0, sizeof ps2);
    kernel_init();
    kernel_run(0x100000);
    printf("calls=%ld v0=0x%llx\n", calls, (unsigned long long)ps2.gpr[2].d[0]);
    return calls == 1 && ps2.gpr[2].d[0] == 0x1234 ? 0 : 1;
}
