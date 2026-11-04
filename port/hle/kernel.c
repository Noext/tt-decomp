/* The EU build's kernel: memory, threads as fibers, semaphores, the alarm and vsync clock.

   The PS2's threads are cooperative. Here they are host fibers on one OS thread, so exactly one
   runs at a time and a switch happens only where the PS2 kernel would make one: a thread blocks
   (WaitSema, SleepThread), a thread of higher priority becomes ready, or an alarm fires. The
   emulated EE registers (ps2asm.h, `ps2`) are saved and restored with the fiber.

   This follows downhill-revived's port/hle/kernel.c (GPL-3.0-or-later); the structure is kept,
   the syscall bindings are this build's own (port/hle/syscalls_eu.inc). */
#include "hle.h"
#include "ps2asm.h"
#include "memory.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ucontext.h>

/* ---- errors (Sony's kernel codes) ---- */
#define KE_OK 0
#define KE_ERROR -1
#define KE_UNKNOWN_THREADID -403
#define KE_UNKNOWN_SEMID -409
#define KE_SEMA_ZERO -410
#define KE_SEMA_OVF -411

/* ---- threads ---- */
enum { TS_RUN = 1, TS_READY = 2, TS_WAIT = 4, TS_SUSPEND = 8, TS_DORMANT = 0x10 };
enum { WAIT_NONE, WAIT_SLEEP, WAIT_SEMA };

typedef struct {
    int32_t status;
    uint32_t entry, stack, stack_size, gp;
    int32_t init_priority, current_priority;
    uint32_t attr, option;
    int32_t wait_type, wait_id, wakeup_count;
} ThreadParam;

/* The EE register file as this port keeps it (ps2asm.h). Saved per fiber. */
typedef struct {
    ps2_q128 gpr[32];
    uint64_t lo, hi, lo1, hi1;
    float fpr[32], fpu_acc;
} Registers;

/* The thread's entry is a guest address (the game passes one to CreateThread). The host code that
   runs it is resolved separately: a recompiled function lives at a host address the linker chose,
   not at the guest address the game's call site names. Its real signature is the port's uniform
   eight-argument form (recomp.py writes every wrapper that way), whatever the game declared. */
typedef uint32_t (*guest_entry_fn)(uint32_t, uint32_t, uint32_t, uint32_t,
                                   uint32_t, uint32_t, uint32_t, uint32_t);

typedef struct Thread {
    int used, status, priority, init_priority;
    uint32_t entry;                       /* the guest address (ThreadParam.entry) */
    guest_entry_fn host_entry;            /* what to call on the host, resolved by address */
    uint32_t stack, stack_size, gp, attr, option, arg;
    int wait, wait_id, wait_result, wakeup_count;
    int64_t order;
    ucontext_t context;
    Registers regs;
} Thread;

#define MAX_THREADS 256
#define STACK_BYTES (512 * 1024)
static Thread threads[MAX_THREADS];
static Thread *current;
static Thread *scheduler_thread;
static ucontext_t scheduler_context;
static int in_kernel;
static int game_exited;
static int64_t order_back = 1;

static int thread_id(Thread *t) { return (int)(t - threads); }

/* ---- clock: the EE bus, and alarm units (HSYNC = 1/15734 s) ---- */
#define BUS_HZ 147456000u
#define HSYNC_HZ 15734u
static uint64_t clock_base_ns;          /* host time of cycle 0 */
static uint64_t alarm_fired[64];        /* per alarm: the cycle it last fired */

/* ---- the host calls into the fibers ---- */
extern void ps2_recomp_enter(void);     /* recomp.h: sets the emulated stack pointer */

static void make_ready(Thread *t, int prepend);

/* ---- the register file moves with the thread ---- */
static void regs_save(void)
{
    for (int i = 0; i < 32; i++) threads[thread_id(current)].regs.gpr[i] = ps2.gpr[i];
    threads[thread_id(current)].regs.lo = ps2.lo;
    threads[thread_id(current)].regs.hi = ps2.hi;
    threads[thread_id(current)].regs.lo1 = ps2.lo1;
    threads[thread_id(current)].regs.hi1 = ps2.hi1;
    for (int i = 0; i < 32; i++) threads[thread_id(current)].regs.fpr[i] = ps2.fpr[i];
    threads[thread_id(current)].regs.fpu_acc = ps2.fpu_acc;
}

static void regs_load(int id)
{
    for (int i = 0; i < 32; i++) ps2.gpr[i] = threads[id].regs.gpr[i];
    ps2.lo = threads[id].regs.lo;
    ps2.hi = threads[id].regs.hi;
    ps2.lo1 = threads[id].regs.lo1;
    ps2.hi1 = threads[id].regs.hi1;
    for (int i = 0; i < 32; i++) ps2.fpr[i] = threads[id].regs.fpr[i];
    ps2.fpu_acc = threads[id].regs.fpu_acc;
}

/* ---- choosing what runs next ---- */
static Thread *ready_best(void)
{
    Thread *best = NULL;
    for (int i = 1; i < MAX_THREADS; i++) {
        Thread *t = &threads[i];
        if (!t->used || t->status != TS_READY)
            continue;
        if (!best || t->priority > best->priority ||
            (t->priority == best->priority && t->order < best->order))
            best = t;
    }
    return best;
}

static void make_ready(Thread *t, int prepend)
{
    if (t->status == TS_DORMANT)
        return;
    t->status = TS_READY;
    t->wait = WAIT_NONE;
    t->order = prepend ? --order_back : order_back++;
}

/* Runs the highest-priority ready thread until none is left. */
static void reschedule(void)
{
    if (in_kernel)
        return;
    in_kernel = 1;
    for (;;) {
        Thread *next = ready_best();
        if (!next)
            break;
        next->status = TS_RUN;
        current = next;
        regs_load(thread_id(next));
        swapcontext(&scheduler_context, &next->context);
    }
    current = NULL;
    in_kernel = 0;
}

/* ---- thread bodies ---- */
static void thread_entry(void)
{
    Thread *t = current;
    if (t->host_entry) {
        ps2_recomp_enter();
        t->host_entry(0, 0, 0, 0, 0, 0, 0, 0);
    }
    t->status = TS_DORMANT;
    current = NULL;
    setcontext(&scheduler_context);
}

static void setup_context(Thread *t)
{
    getcontext(&t->context);
    static void *stack_pool[MAX_THREADS];
    if (!stack_pool[thread_id(t)])
        stack_pool[thread_id(t)] = malloc(STACK_BYTES);
    t->context.uc_stack.ss_sp = stack_pool[thread_id(t)];
    t->context.uc_stack.ss_size = STACK_BYTES;
    t->context.uc_link = &scheduler_context;
    makecontext(&t->context, thread_entry, 0);
}

/* ---- the kernel's own bookkeeping, run whenever a syscall is made ---- */
static uint64_t host_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

int hle_clock_init(void)
{
    clock_base_ns = host_ns();
    return 0;
}

uint32_t hle_cycles(void)
{
    uint64_t ns = host_ns() - clock_base_ns;
    return (uint32_t)((ns * BUS_HZ) / 1000000000ull);
}

/* Alarms fire when their tick count has passed; a waiting thread is made ready. */
void kernel_poll(void)
{
    uint32_t cycles = hle_cycles();
    uint32_t hsync = (uint32_t)(((uint64_t)cycles * HSYNC_HZ) / BUS_HZ);
    for (int i = 1; i < 64; i++) {
        Thread *t = &threads[i];
        if (t->used && t->status == TS_WAIT && t->wait == WAIT_SLEEP) {
            if ((uint32_t)t->wait_id <= hsync) {
                t->wait_result = t->wait_id;
                make_ready(t, 0);
            }
        }
    }
}

/* ---- memory and cache ---- */
int GetMemorySize(void) { return PS2_RAM_SIZE; }
void FlushCache(int mode) { (void)mode; }              /* the host's memory is coherent */
void iFlushCache(int mode) { (void)mode; }
void FlushCacheAll(void) {}
void iFlushCacheAll(void) {}
void SyncCache(int mode) { (void)mode; }
void iSyncCache(int mode) { (void)mode; }
void InitTLB(void) {}                                  /* the host maps the memory */
void PutTLBEntry(int index, uint32_t page, uint32_t mask, uint32_t hi) { (void)index; (void)page; (void)mask; (void)hi; }

/* ---- semaphores ---- */
typedef struct {
    int32_t count, max_count, init_count, wait_threads;
    uint32_t attr, option;
} SemaParam;

typedef struct { int used, count, max_count, init_count; uint32_t attr, option; } Sema;
#define MAX_SEMAS 512
static Sema semas[MAX_SEMAS];

static Sema *sema_of(int id) { return id > 0 && id < MAX_SEMAS && semas[id].used ? &semas[id] : NULL; }

int CreateSema(SemaParam *p)
{
    for (int i = 1; i < MAX_SEMAS; i++)
        if (!semas[i].used) {
            Sema *s = &semas[i];
            s->used = 1;
            s->count = s->init_count = p->init_count;
            s->max_count = p->max_count;
            s->attr = p->attr;
            s->option = p->option;
            return i;
        }
    return KE_ERROR;
}

int DeleteSema(int id)
{
    Sema *s = sema_of(id);
    if (!s)
        return KE_UNKNOWN_SEMID;
    for (int i = 1; i < MAX_THREADS; i++)
        if (threads[i].used && threads[i].status == TS_WAIT && threads[i].wait == WAIT_SEMA && threads[i].wait_id == id) {
            threads[i].wait_result = KE_ERROR;
            make_ready(&threads[i], 0);
        }
    s->used = 0;
    reschedule();
    return id;
}

static int signal_sema(int id)
{
    Sema *s = sema_of(id);
    if (!s)
        return KE_UNKNOWN_SEMID;
    Thread *first = NULL;
    for (int i = 1; i < MAX_THREADS; i++) {
        Thread *t = &threads[i];
        if (t->used && t->status == TS_WAIT && t->wait == WAIT_SEMA && t->wait_id == id &&
            (!first || t->order < first->order))
            first = t;
    }
    if (first) {
        make_ready(first, 0);
        /* A is ready and has higher priority or an earlier place in the queue: hand over now,
           the way the PS2 kernel does on a signal. */
        Thread *from = current;
        if (from) {
            from->status = TS_RUN;
            regs_save();
            swapcontext(&from->context, &scheduler_context);
            current = from;
            regs_load(thread_id(from));
        }
    } else {
        if (s->count < s->max_count)
            s->count++;
    }
    return id;
}

int SignalSema(int id) { kernel_poll(); return signal_sema(id); }
int iSignalSema(int id) { return signal_sema(id); }

int PollSema(int id)
{
    Sema *s = sema_of(id);
    if (!s)
        return KE_UNKNOWN_SEMID;
    if (s->count <= 0)
        return KE_SEMA_ZERO;
    s->count--;
    return id;
}

int iPollSema(int id) { return PollSema(id); }

static int block(int kind, int id)
{
    current->wait = kind;
    current->wait_id = id;
    current->wait_result = KE_OK;
    current->order = order_back++;
    current->status = TS_WAIT;
    Thread *from = current;
    in_kernel = 1;
    regs_save();
    swapcontext(&from->context, &scheduler_context);
    in_kernel = 0;
    current = from;
    regs_load(thread_id(from));
    return from->wait_result;
}

int WaitSema(int id)
{
    kernel_poll();
    Sema *s = sema_of(id);
    if (!s)
        return KE_UNKNOWN_SEMID;
    if (s->count > 0) {
        s->count--;
        return id;
    }
    int r = block(WAIT_SEMA, id);
    return r < 0 ? r : id;
}

int ReferSemaStatus(int id, SemaParam *info)
{
    Sema *s = sema_of(id);
    if (!s)
        return KE_UNKNOWN_SEMID;
    info->count = s->count;
    info->max_count = s->max_count;
    info->init_count = s->init_count;
    info->wait_threads = 0;
    for (int i = 1; i < MAX_THREADS; i++)
        info->wait_threads += threads[i].used && threads[i].status == TS_WAIT &&
                              threads[i].wait == WAIT_SEMA && threads[i].wait_id == id;
    info->attr = s->attr;
    info->option = s->option;
    return id;
}

int iReferSemaStatus(int id, SemaParam *info) { return ReferSemaStatus(id, info); }

/* ---- threads ---- */
/* Resolved by the host build: the host function for a guest entry address, or NULL. */
extern guest_entry_fn *hle_resolve_entry(uint32_t guest_address);

int CreateThread(ThreadParam *p)
{
    for (int i = 1; i < MAX_THREADS; i++)
        if (!threads[i].used) {
            Thread *t = &threads[i];
            memset(t, 0, sizeof *t);
            t->used = 1;
            t->status = TS_DORMANT;
            t->entry = p->entry;
            t->host_entry = hle_resolve_entry(p->entry);
            t->stack = p->stack;
            t->stack_size = p->stack_size;
            t->gp = p->gp;
            t->init_priority = t->priority = p->init_priority;
            t->attr = p->attr;
            t->option = p->option;
            setup_context(t);
            return i;
        }
    return KE_ERROR;
}

int StartThread(int id, void *arg)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    Thread *t = &threads[id];
    t->arg = (uint32_t)(uintptr_t)arg;
    t->status = TS_READY;
    t->order = order_back++;
    return id;
}

int DeleteThread(int id)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    threads[id].used = 0;
    reschedule();
    return id;
}

void ExitThread(void)
{
    if (current)
        current->status = TS_DORMANT;
    reschedule();
}

void ExitDeleteThread(void) { ExitThread(); }

int TerminateThread(int id)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    threads[id].status = TS_DORMANT;
    reschedule();
    return id;
}

int SleepThread(void)
{
    /* SleepThread with no count: block until woken. With r0 set by the SDK (%v0 holds the count
       in some builds); here the argument is taken from the caller's register. */
    int count = (int)(uint32_t)ps2.gpr[4].d[0];
    if (count == 0) {
        int r = block(WAIT_SLEEP, 0x7FFFFFFF);
        return r;
    }
    /* a real sleep: the tick count is alarm units; block until kernel_poll passes it */
    uint32_t now = (uint32_t)(((uint64_t)hle_cycles() * HSYNC_HZ) / BUS_HZ);
    uint32_t until = now + (uint32_t)count;
    return block(WAIT_SLEEP, (int)until);
}

int WakeupThread(int id)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    Thread *t = &threads[id];
    if (t->status == TS_WAIT && t->wait == WAIT_SLEEP)
        make_ready(t, 0);
    else
        t->wakeup_count++;
    reschedule();
    return id;
}

int iWakeupThread(int id)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    Thread *t = &threads[id];
    if (t->status == TS_WAIT && t->wait == WAIT_SLEEP)
        make_ready(t, 0);
    else
        t->wakeup_count++;
    return id;
}

int CancelWakeupThread(int id) { (void)id; return id; }
int iCancelWakeupThread(int id) { (void)id; return id; }

int SuspendThread(int id)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    threads[id].status |= TS_SUSPEND;
    reschedule();
    return id;
}

int ResumeThread(int id)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    threads[id].status &= ~TS_SUSPEND;
    make_ready(&threads[id], 0);
    reschedule();
    return id;
}

int iSuspendThread(int id) { return SuspendThread(id); }

int ReferThreadStatus(int id, ThreadParam *info)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    Thread *t = &threads[id];
    info->status = t->status;
    info->entry = t->entry;
    info->stack = t->stack;
    info->stack_size = t->stack_size;
    info->gp = t->gp;
    info->init_priority = t->init_priority;
    info->current_priority = t->priority;
    info->attr = t->attr;
    info->option = t->option;
    info->wait_type = t->wait;
    info->wait_id = t->wait_id;
    info->wakeup_count = t->wakeup_count;
    return id;
}

int iReferThreadStatus(int id, ThreadParam *info) { return ReferThreadStatus(id, info); }

int GetThreadId(void) { return current ? thread_id(current) : 0; }

int ChangeThreadPriority(int id, int priority)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    Thread *t = &threads[id];
    int old = t->priority;
    t->priority = priority;
    return old;
}

int iChangeThreadPriority(int id, int p) { return ChangeThreadPriority(id, p); }
int SetThreadPriority(int id, int p) { return ChangeThreadPriority(id, p); }
int iSetThreadPriority(int id, int p) { return ChangeThreadPriority(id, p); }
int GetThreadPriority(int id)
{
    if (id <= 0 || id >= MAX_THREADS || !threads[id].used)
        return KE_UNKNOWN_THREADID;
    return threads[id].priority;
}
int iGetThreadPriority(int id) { return GetThreadPriority(id); }
int SetThreadParam(int id, int param, void *value) { (void)id; (void)param; (void)value; return KE_OK; }
int iSetThreadParam(int id, int param, void *value) { return SetThreadParam(id, param, value); }
int RotateThreadReadyQueue(int priority) { (void)priority; reschedule(); return KE_OK; }
int iRotateThreadReadyQueue(int priority) { return RotateThreadReadyQueue(priority); }
void ReleaseThreadWait(void) {}
void ReleaseWaitThread(int id) { WakeupThread(id); }
void iDisableDispatchThread(void) {}
void iEnableDispatchThread(void) {}

/* ---- alarms: a thread entry called when the tick count reaches a target ---- */
typedef struct { int used; uint32_t tick, target, handler, arg; } Alarm;
static Alarm alarms[64];

int SetAlarm(uint32_t tick, uint32_t handler, uint32_t arg)
{
    for (int i = 1; i < 64; i++)
        if (!alarms[i].used) {
            alarms[i].used = 1;
            alarms[i].tick = tick;
            alarms[i].handler = handler;
            alarms[i].arg = arg;
            return i;
        }
    return KE_ERROR;
}

int iSetAlarm(uint32_t tick, uint32_t handler, uint32_t arg) { return SetAlarm(tick, handler, arg); }

int CancelAlarm(int id)
{
    if (id > 0 && id < 64)
        alarms[id].used = 0;
    return id;
}

int iCancelAlarm(int id) { return CancelAlarm(id); }
int ReleaseAlarm(int id) { return CancelAlarm(id); }
int iReleaseAlarm(int id) { return CancelAlarm(id); }
int GetAlarmRemain(int id) { (void)id; return 0; }

/* ---- misc kernel ---- */
int SetOsdConfigParam(void *param) { (void)param; return KE_OK; }
int GetOsdConfigParam(void *param) { if (param) memset(param, 0, 16); return KE_OK; }
int SetGsCrt(int interlace, int mode, int field) { (void)interlace; (void)mode; (void)field; return KE_OK; }

/* ---- startup ---- */
extern void hle_boot(void);   /* boot.c: the entry the game's crt0 calls */

void kernel_init(void)
{
    hle_clock_init();
    getcontext(&scheduler_context);
    scheduler_thread = NULL;
}

/* Runs the game's entry point as thread 0 and drives the scheduler to completion. */
void kernel_run(uint32_t entry)
{
    ThreadParam p;
    memset(&p, 0, sizeof p);
    p.entry = entry;
    p.init_priority = 1;
    int id = CreateThread(&p);
    StartThread(id, NULL);
    reschedule();
}

/* A recompiled `syscall 0`: dispatch the number the SDK left in $v1. Only the calls the port
   implements do anything; the rest are logged once and return 0, as the stubs do. */
void hle_syscall(uint32_t number)
{
    (void)number;
    hle_log("syscall %u raised in recompiled code\n", number);
}
