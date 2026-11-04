/* The guest address map: every window folds where it should, and anything else aborts.

   These tests are the contract of port/runtime/ps2asm.h's ps2_guest_address. They do not start the
   game and need no disc: the classifier is pure arithmetic.

   usage: make -C port/tests test_map
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../runtime/ps2asm.h"

/* ps2_address_fault aborts in the real runtime. The tests keep it a real abort and check it the
   same way the runtime does: in a forked child, whose exit status says whether it faulted. That
   also proves the abort path itself, which a longjmp would hide. */
void ps2_address_fault(uintptr_t address)
{
    (void)address;
    abort();
}

/* Runs `expr` in a child process; returns 1 when the child aborted on a fault. */
static int faults_on(void (*probe)(void))
{
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        probe();
        _exit(0);              /* no fault: the child finished normally */
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFSIGNALED(status) && (WTERMSIG(status) == SIGABRT || WTERMSIG(status) == SIGSEGV);
}

void (*ps2_graphics_sync)(void);
void (*ps2_intc_read)(void);

static int failures;

#define CHECK(cond, ...) do {                                                  \
        if (!(cond)) {                                                         \
            printf("FAIL %s:%d: ", __FILE__, __LINE__);                        \
            printf(__VA_ARGS__);                                               \
            printf("\n");                                                      \
            failures++;                                                        \
        }                                                                      \
    } while (0)

#define EXPECT_HOST(guest, want) do {                                          \
        ps2_mapping m = ps2_guest_address((uintptr_t)(guest));                 \
        CHECK(m.host == (uintptr_t)(want),                                     \
              "0x%08x -> 0x%08lx, expected 0x%08lx", (unsigned)(guest),        \
              (unsigned long)m.host, (unsigned long)(want));                   \
    } while (0)

static uintptr_t fault_probe_address;
static void fault_probe(void) { ps2_mapping m = ps2_guest_address(fault_probe_address); (void)m; }

#define EXPECT_FAULT(guest) do {                                               \
        fault_probe_address = (uintptr_t)(guest);                              \
        CHECK(faults_on(fault_probe), "0x%08x did not fault", (unsigned)(guest)); \
    } while (0)

/* ---- the six RAM windows fold onto the one mapped block ---- */
static void test_ram_windows(void)
{
    /* The low physical view is the identity onto the block. */
    EXPECT_HOST(0x00000000u, 0x08000000u);
    EXPECT_HOST(0x00076000u, 0x08076000u);
    EXPECT_HOST(0x01FFFFFFu, 0x09FFFFFFu);

    /* The case the port crashed on: the KSEG0 view of physical 0x76100. */
    EXPECT_HOST(0x80076000u, 0x08076000u);
    EXPECT_HOST(0x80076100u, 0x08076100u);

    /* The other four windows map to the same bytes. */
    EXPECT_HOST(0xA0076000u, 0x08076000u);
    EXPECT_HOST(0x20076000u, 0x08076000u);
    EXPECT_HOST(0x30076000u, 0x08076000u);
    EXPECT_HOST(0xC0076000u, 0x08076000u);
    EXPECT_HOST(0x38076000u, 0x08076000u);
}

/* A KSEG address is the low address OR'd with the window bit, and folds the same way. */
static void test_window_bit_is_the_only_difference(void)
{
    for (uint32_t low = 0; low < 0x02000000u; low += 0x100000u) {
        uintptr_t a = ps2_guest_address(low).host;
        uintptr_t b = ps2_guest_address(0x80000000u | low).host;
        CHECK(a == b, "0x%08x and its KSEG0 twin differ", (unsigned)low);
    }
}

/* ---- MMIO keeps its address ---- */
static void test_mmio_cached(void)
{
    ps2_mapping m = ps2_guest_address(0x1000F000u);
    CHECK(m.region == PS2_REGION_MMIO, "0x1000F000 is not MMIO");
    CHECK(m.host == 0x1000F000u, "MMIO must not be folded, got 0x%lx", (unsigned long)m.host);
    CHECK(m.physical == 0x1000F000u, "MMIO physical should be itself");
}

static void test_mmio_uncached_mirror(void)
{
    /* The timers are written through 0xB0000000 + 0x1010 in the game's own wrappers. */
    ps2_mapping m = ps2_guest_address(0xB0001010u);
    CHECK(m.region == PS2_REGION_MMIO, "0xB0001010 is not MMIO");
    CHECK(m.host == 0xB0001010u, "the uncached mirror must keep its address");
    CHECK(m.physical == 0x00001010u, "uncached mirror physical should drop the frame bits, got 0x%x",
          m.physical);
}

static void test_vu_and_gs(void)
{
    ps2_mapping vu0 = ps2_guest_address(0x11000000u);
    ps2_mapping vu0d = ps2_guest_address(0x11004000u);
    ps2_mapping vu1 = ps2_guest_address(0x11008000u);
    ps2_mapping gs = ps2_guest_address(0x12001000u);
    CHECK(vu0.host == 0x11000000u && vu0.region == PS2_REGION_MMIO, "VU0 micro");
    CHECK(vu0d.host == 0x11004000u, "VU0 data");
    CHECK(vu1.host == 0x11008000u, "VU1 micro");
    CHECK(gs.host == 0x12001000u && gs.region == PS2_REGION_MMIO, "GS registers");
}

static void test_scratchpad(void)
{
    ps2_mapping m = ps2_guest_address(0x70000000u);
    CHECK(m.region == PS2_REGION_SCRATCHPAD, "scratchpad region");
    CHECK(m.host == 0x70000000u, "scratchpad keeps its address");
    CHECK(ps2_guest_address(0x70003FFFu).region == PS2_REGION_SCRATCHPAD, "scratchpad last byte");
}

/* ---- everything else is a bug and must abort ---- */
static void test_unmapped_faults(void)
{
    EXPECT_FAULT(0x40000000u);   /* not a RAM window */
    EXPECT_FAULT(0x02000000u);   /* just past RAM */
    EXPECT_FAULT(0x82000000u);   /* just past the KSEG0 block */
    EXPECT_FAULT(0x70004000u);   /* just past the scratchpad */
    EXPECT_FAULT(0x10010000u);   /* just past the EE registers */
    EXPECT_FAULT(0xDEADBEEFu);
}

/* A host pointer that leaked into guest state: above 32 bits, always a fault. */
static void test_host_pointer_faults(void)
{
    EXPECT_FAULT((uintptr_t)0x00007F3A2C1B4C5DULL);
    EXPECT_FAULT((uintptr_t)0x0000000100000000ULL);
}

/* ---- the helpers that everything else uses must agree with the classifier ---- */
static void test_helpers_agree(void)
{
    const uintptr_t probes[] = {0x00076000u, 0x80076000u, 0xA0076000u, 0x1000F000u, 0x70000000u};
    for (unsigned i = 0; i < sizeof probes / sizeof *probes; i++) {
        uintptr_t a = probes[i];
        CHECK(ps2_ram_canonical(a) == ps2_guest_address(a).host, "ps2_ram_canonical disagrees at 0x%lx",
              (unsigned long)a);
        CHECK(ps2_memory_address(a) == ps2_guest_address(a).host, "ps2_memory_address disagrees at 0x%lx",
              (unsigned long)a);
        CHECK(ps2_load_address(a) == ps2_guest_address(a).host, "ps2_load_address disagrees at 0x%lx",
              (unsigned long)a);
    }
}

/* The fail-closed rule: exactly the documented windows are accepted, nothing else. */
static void test_map_is_closed(void)
{
    static const struct { uint32_t base, size; } windows[] = {
        {0x00000000u, 0x02000000u}, {0x20000000u, 0x02000000u}, {0x30000000u, 0x02000000u},
        {0x80000000u, 0x02000000u}, {0xA0000000u, 0x02000000u}, {0xC0000000u, 0x02000000u},
        {0x38000000u, 0x02000000u},
        {0x10000000u, 0x00010000u}, {0xB0000000u, 0x00010000u},
        {0x11000000u, 0x00010000u}, {0x12000000u, 0x00002000u},
        {0x70000000u, 0x00004000u},
    };
    const unsigned n = sizeof windows / sizeof *windows;
    /* Sampled rather than exhaustive (fork per probe): every 16 MiB plus each window's edges. */
    for (uint64_t a = 0; a <= 0xFFFFFFFFull; a += 0x01000000ull) {
        int inside = 0;
        for (unsigned i = 0; i < n; i++)
            inside |= (uint32_t)(a - windows[i].base) < windows[i].size;
        fault_probe_address = (uintptr_t)a;
        int faulted = faults_on(fault_probe);
        CHECK(inside != faulted, "0x%08llx: in a window=%d but faulted=%d", (unsigned long long)a,
              inside, faulted);
    }
}

int main(void)
{
    test_ram_windows();
    test_window_bit_is_the_only_difference();
    test_mmio_cached();
    test_mmio_uncached_mirror();
    test_vu_and_gs();
    test_scratchpad();
    test_unmapped_faults();
    test_host_pointer_faults();
    test_helpers_agree();
    test_map_is_closed();

    if (failures) {
        printf("%d check(s) failed\n", failures);
        return 1;
    }
    printf("guest address map: all checks passed\n");
    return 0;
}
