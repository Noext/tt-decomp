# Working rules for this port

These apply to anyone — person or agent — changing `port/`. They exist because each one cost a
session to learn.

## 1. Rebuild and verify before concluding anything

A claim about behaviour is only true if the binary was rebuilt after the change and the trace shows
it. No "should work", no "the code looks right". Run `make`, run the port, read the output.

## 2. Never stub a function whose return the caller tests

Before stubbing anything, read every caller. If any of them branches on the return value, a stub
that returns 0 is a hang, not a shortcut. `func_500B40` is the canonical example: it fills a table
slot, and its caller loops until the slot is non-zero.

If a function's effect must go away, remove the *reason* it is needed, not its return value.

## 3. One writer per structure

A structure in guest memory — a table, a descriptor, a list — has exactly one writer. If the game's
own code fills it, the HLE must not write it too, even with the same values: two writers means the
next divergence is invisible. The HLE's job is to make the game's writer succeed, not to race it.

## 4. ps2sdk and ps2tek are the specification

Syscall numbers, register addresses, segment maps and structure layouts come from:

- **ps2sdk** (`~/Dev/ps2sdk`, `ee/kernel/include/syscallnr.h`, `kernel.h`, `timer.h`,
  `ee/kernel/src/`) — the EE kernel's own numbers and semantics, and the init code the games reuse;
- **ps2tek** (`https://psi-rockin.github.io/ps2tek/`) — the EE memory map, GS registers, VU1,
  DMA channels.

Never guess a number. A wrong syscall number looks exactly like a wrong implementation, and the
compiler does not catch it (`case 91:` and `case 0x5B:` were the same case for a whole session).

## 5. Replace Sony's libraries; do not emulate their internals

The game ships a copy of the SDK. Running that copy faithfully means emulating a kernel: user mode,
exception vectors, dispatchers copied into RAM, cache flushes that matter. The port should instead
recognise the SDK entry point and provide its effect directly.

That means: identify the function, stub the one whose whole purpose is to patch the kernel, and
implement the behaviour it was trying to install, natively.

## 6. No claim without evidence in the code or the trace

Every statement about the game gets one of: a disassembly excerpt with its address, a line from the
trace, or a citation from ps2sdk/ps2tek. If the evidence is missing, say it is unknown.

Corollary: a previous conclusion is not evidence. When a claim contradicts a later measurement, the
later measurement wins and the claim is retracted.

## 7. Atomic commits, rollback on no progress

One change per commit, with the measurement that justifies it. If a change does not move the trace
forward, revert it rather than building on it — a wrong fix that compiles is more expensive than a
reverted one.
