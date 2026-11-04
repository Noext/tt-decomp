/* The guest-code-address table (tools/make_entry_table.py): the bridge between a guest code
   address the game passes around and the host function that runs it. */
#include <stddef.h>
#include <stdint.h>
#define ENTRY_TABLE_DEFINE 1
#include "entry_table.h"

typedef uint32_t ps2_entry_fn(uint32_t, uint32_t, uint32_t, uint32_t,
                              uint32_t, uint32_t, uint32_t, uint32_t);

/* The kernel asks for a callable: the host function at the guest address, else nothing. */
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
