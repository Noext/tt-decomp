/* Semaphores and two threads: A waits, B signals, A resumes. Proves the scheduler. */
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

typedef uint64_t (*guest_entry_fn)(void);
int CreateSema(void *p);
int WaitSema(int id);
int SignalSema(int id);
int CreateThread(void *p);
int StartThread(int id, void *arg);

static int sema, order[8], order_n, a_done, b_done;

static uint64_t thread_a(void)
{
    WaitSema(sema);            /* blocks until B signals */
    a_done = 1;
    order[order_n++] = 1;
    return 0;
}
static uint64_t thread_b(void)
{
    order[order_n++] = 2;
    b_done = 1;
    SignalSema(sema);          /* wakes A, which runs before B returns */
    order[order_n++] = 3;
    return 0;
}
guest_entry_fn hle_resolve_entry(uint32_t g) { return g == 0xA ? thread_a : g == 0xB ? thread_b : 0; }

int main(void)
{
    memset(&ps2, 0, sizeof ps2);
    kernel_init();
    struct { int32_t count, max_count, init_count, wait_threads; uint32_t attr, option; } sp = {0, 1, 0, 0, 0, 0};
    sema = CreateSema(&sp);
    struct { int32_t status; uint32_t entry, stack, stack_size, gp; int32_t init_priority, current_priority; uint32_t attr, option; int32_t wait_type, wait_id, wakeup_count; } t;
    memset(&t, 0, sizeof t);
    t.entry = 0xA; t.init_priority = 10;
    int a = CreateThread(&t);
    memset(&t, 0, sizeof t);
    t.entry = 0xB; t.init_priority = 5;
    int b = CreateThread(&t);
    StartThread(a, 0);
    StartThread(b, 0);
    kernel_run(0);             /* nothing to start; just run what is ready */
    printf("a_done=%d b_done=%d order=", a_done, b_done);
    for (int i = 0; i < order_n; i++) printf("%d ", order[i]);
    printf("\n");
    return a_done && b_done ? 0 : 1;
}
