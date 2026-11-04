/* The guest-code-address table (tools/make_entry_table.py): the bridge between a guest code
   address the game passes around and the host function that runs it. */
#include <stddef.h>
#include <stdint.h>
#define ENTRY_TABLE_DEFINE 1
#include "entry_table.h"

typedef uint32_t ps2_entry_fn(uint32_t, uint32_t, uint32_t, uint32_t,
                              uint32_t, uint32_t, uint32_t, uint32_t);

/* The kernel asks for a callable: the host function at the guest address, else nothing. */
#include <stdio.h>
extern int hle_trace_on;

ps2_entry_fn *hle_resolve_entry(uint32_t guest)
{
    size_t lo = 0, hi = ENTRY_COUNT;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (entries[mid].guest == guest)
            return (ps2_entry_fn *)entries[mid].host;
        if (entries[mid].guest < guest)
            lo = mid + 1;
        else
            hi = mid;
    }
    return 0;
}

/* A code address as the game passes it around: the guest address, not the host function behind it.
   The recompiled code takes function addresses for callbacks, vtables and the like; those must be
   guest numbers, or the game reads a host pointer (64 bits, truncated) as an address. */
uint32_t ps2_guest_code_address(uint32_t guest) { return guest; }

/* Runs the guest function at an address with the given arguments (eret and indirect calls). */
uint32_t hle_call_guest(uint32_t guest, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3,
                        uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{
    ps2_entry_fn *fn = hle_resolve_entry(guest);
    return fn ? fn(a0, a1, a2, a3, a4, a5, a6, a7) : 0;
}
