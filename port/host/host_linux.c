/* The host services the port's runtime and HLE use, on Linux: the guest's RAM and its views,
   fixed pages at their PS2 addresses, and the clock. This is the file downhill-revived's Windows
   host would be for this port; the interface is host_api.h, unchanged. */
#define _GNU_SOURCE
#include "host_api.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

static int ram_fd = -1;             /* the shared RAM block, as an anonymous mapping */
static void *ram_base;
static size_t ram_size;

unsigned long host_last_error(void) { return (unsigned long)errno; }

/* Creates the guest's RAM as a memory file, so every view (the uncached mirrors, the low view)
   maps the SAME bytes instead of a copy. */
int host_ram_create(uint32_t size)
{
    ram_fd = memfd_create("ps2-ram", MFD_CLOEXEC);
    if (ram_fd < 0)
        return 0;
    if (ftruncate(ram_fd, size) != 0)
        return 0;
    void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, ram_fd, 0);
    if (p == MAP_FAILED)
        return 0;
    ram_base = p;
    ram_size = size;
    return 1;
}

/* Maps the guest RAM at a fixed address: `at` is the address the view must live at, `offset` the
   byte inside the RAM block the view starts at. */
int host_ram_view(uint32_t at, uint32_t offset, uint32_t size, int over_reservation)
{
    if (at == 0 || ram_fd < 0)
        return 0;
    /* MAP_FIXED_NOREPLACE refuses rather than clobbering; the reservation case may replace. */
    int flags = MAP_SHARED | (over_reservation ? MAP_FIXED : MAP_FIXED_NOREPLACE);
    void *p = mmap((void *)(uintptr_t)at, size, PROT_READ | PROT_WRITE, flags, ram_fd, offset);
    if (p == MAP_FAILED) {
        fprintf(stderr, "host_ram_view: cannot map %u bytes at 0x%08X (%s)\n", size, at, strerror(errno));
        return 0;
    }
    return 1;
}

/* Fixed pages at their PS2 address (scratchpad, hardware registers, VU memory). */
int host_mem_commit(uint32_t at, uint32_t size)
{
    void *p = mmap((void *)(uintptr_t)at, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_FIXED_NOREPLACE, -1, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "host_mem_commit: cannot map %u bytes at 0x%08X (%s)\n", size, at, strerror(errno));
        return 0;
    }
    return 1;
}

int host_mem_protect(uint32_t at, uint32_t size, int protection)
{
    int prot = PROT_NONE;
    if (protection & 1) prot |= PROT_READ;
    if (protection & 2) prot |= PROT_WRITE;
    if (protection & 4) prot |= PROT_EXEC;
    return mprotect((void *)(uintptr_t)at, size, prot) == 0;
}

/* Wall-clock seconds since the epoch, for the port's own profiling (vu1_profile.h). */
double host_wall_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* Monotonic seconds, for the port's own timing. */
double host_seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
