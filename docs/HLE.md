# The HLE for SCES-53372

What the port replaces of the PS2 system, and what it still needs.

## Why the syscalls are the seam

The PS2 SDK is **inside the game's own executable** in this build: the code that calls the kernel
is a wrapper function per syscall, `addiu $v1, N; syscall 0`. Those 162 wrappers are exactly the
functions `recomp.py` cannot translate (there is no native `syscall`), so they are the natural
place to put the host implementation — the game calls the address, the HLE's C function runs.

`tools/syscalls.py` finds them and writes `port/hle/syscalls_eu.inc`:

| | |
|---|---|
| Wrappers | 162 |
| Distinct syscall numbers | 142 |
| Named from the public SDK API | 70 |

`port/hle/syscalls.c` defines every wrapper's address. Today each is a stub that logs and returns
0 (`hle_unknown_syscall`), which is the honest starting point: the game runs until it depends on a
syscall that has not been implemented yet, and the log says which.

## What has to be implemented, in the order a boot needs it

1. **Memory and cache** — `FlushCache`, `SyncCache`, `GetMemorySize`: the host maps the memory, so
   these are near no-ops (`GetMemorySize` must report 32 MB).
2. **Threads** — `CreateThread`, `StartThread`, `SleepThread`, `WakeupThread`, `ExitThread`,
   `ReferThreadStatus`, `ChangeThreadPriority`. The PS2's threads are cooperative; the port runs
   them as fibers (downhill-revived's `port/hle/kernel.c` is the model: one OS thread, a ready
   queue by priority, switches only where the PS2 kernel would).
3. **Semaphores** — `CreateSema`, `WaitSema`, `SignalSema`, `PollSema`, `ReferSemaStatus`.
4. **Timers** — `SetAlarm`, `CancelAlarm`, and the VSYNC tick at 59.94 Hz (PAL build: 50 Hz).
5. **I/O** — `sceCd*` through the CDVD/IOP protocol, `scePad*`, `sceMc*`, `sceSif*` RPC.
6. **Graphics** — `sceGs*` and the VU1 microcode; see the note below.

## What is not settled

- **VU1.** The game's rendering microcode is in the `.data` half of `CORE.TT`; downhill-revived
  translates its own microcode to C (`port/tools/vu1_aot.py`) because it can read the words. The
  same approach applies here, but the microcode has not been located yet.
- **`TT.VOL`.** 2.95 GB, same container as `GT4.VOL` (`ad90b9ac 02000200`). No reader written.
- **The frame rate.** This is the PAL build: 50 Hz. The port should run it at 60 Hz with the
  NTSC constants, as the DHD port does for its own release (finding 100 there).

## Status

| Part | State |
|---|---|
| Syscall seam, all 162 bound | done |
| Syscall implementations | stubs only |
| Memory map | not started |
| Threads and semaphores | not started |
| Timers | not started |
| I/O (CDVD, pad, MC, SIF) | not started |
| Graphics | not started |

## Implemented (2026-11-04)

`port/hle/kernel.c` — memory and cache (no-ops, the host's memory is coherent; `GetMemorySize`
reports 32 MB), threads as host fibers with one OS thread and a priority ready queue, semaphores
(create/delete/signal/wait/poll/status), sleeps and thread wakeups, alarms, and the EE clock.
The register file (`ps2asm.h`'s `ps2`) is saved and restored per fiber.

`port/tests/` holds the HLE's own tests, which need no game data:

| test | proves |
|---|---|
| `test_thread` | a thread's entry runs, its result lands in `$v0`, the fiber returns to the scheduler |
| `test_sync` | two threads with different priorities; A waits on a semaphore, B signals, A resumes before B continues (`order=2 1`) |

    cd port/tests && make test
