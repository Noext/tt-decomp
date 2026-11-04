# Roadmap: from "the code runs" to "the game runs"

Where the port stands: the EU image loads, its own `_start` runs to completion, the SDK's startup
loop executes, and the guest decodes 31,416 recompiled functions. What does not exist yet is
everything the game talks to: interrupts, the IOP, the GS, the VU1 draw path, and input/audio.

Five milestones, in dependency order. Each has a measurable criterion, and each lists what it
touches, with the numbers taken from ps2sdk (`ee/kernel/include/syscallnr.h`, `kernel.h`) and
ps2tek (EE memory map, GS registers, VU1, DMA channels).


---

## PAUSE (2026-11-04): this runtime is halted in favour of ps2-web-recomp

**Decision.** Work on the home-grown port (`port/`) stops here. Tourist Trophy (Europe) runs today
on `snuri00/ps2-web-recomp` (MIT), driven by its own `scripts/port.sh`, without a line of
game-specific code: 288 translated units, 2 VU1 programs, 3,600 frames, 1,092,936 primitives drawn,
its menus and name entry rendering in a browser. See "Why" below for the measurement.

**What this port keeps.** The decompilation work is not lost and is not part of the pause:

| Asset | Where | Still useful for |
|---|---|---|
| 1,465 verified EU sources | `src/` | naming, patching, decompilation progress |
| `config/eu_source_map.tsv` | 16,104 US source → EU address pairs | knowing which US function is which EU one |
| `config/eu_pairs.tsv` | strict masked-hash pairing | same |
| `docs/memory-model.md`, `port/tests/test_map.c` | the EE address map and its contract | re-checking any runtime's memory model |
| `port/AGENTS.md` | seven working rules | any port work, ours or theirs |

**What stops.** The milestones M1-M5 below describe the runtime we were building. They are kept as
a record of what a hand-written PS2 system layer needs, and as the checklist for reading
ps2-web-recomp's runtime (`runtime/src/`, 369 KB). They are no longer a work plan for `port/`.

**Why, measured (2026-11-04).** The same EU executable (`CORE.TT`, SHA-1 `83591f00…`) through
`scripts/port.sh tt`:

| | our port | ps2-web-recomp |
|---|---|---|
| Reaches the game's menus | no (blocked in the SDK's SIF init) | yes |
| Frames rendered | 0 | 3,600 |
| GS primitives drawn | 0 | 1,092,936 |
| VU1 programs executing | interpreter present, none run | 2 AOT-compiled |
| IOP modules | none | 14 IRX, 45 threads |
| Unhandled syscalls | — | 0 |
| Missing IOP services | — | 0 |

**What to do instead.** Extend ps2-web-recomp for TT: the native Linux/Windows executable (plan in
the following section), then the TT-specific pieces (the `TT.VOL` asset container, PAL timing
already handled, any patch the game needs).

---

## M1 — The EE kernel's interrupt subsystem

**Why first.** The game registers interrupt handlers before it does anything else, and its own
init (`0x5012E8`) fills a handler table at `0x790058` through `func_500E70`. Without delivery, that
table stays empty, `func_00500B40` reads zeros, and the boot loops. Everything later — VBlank for
frame pacing, DMA completion, timer wakes — rides on this.

**Criterion (measurable):**

1. Registering a handler for cause *n* and raising *n* calls it, with `$a0 = n`; a handler that
   returns non-zero stops the chain.
2. The game's 7 registrations are visible in a trace with their causes:
   `0x80000008, 0x80000009, 0x8000000A, 0x8000000B, 0x80000011, 0x80000012, 0x80000000`
   (the low nine bits are the INTC number: 8 = `INTC_IPU`, 9–11 = `INTC_TIM0/1/2`,
   17 = `INTC_VBLANK_S`, 18 = `INTC_VBLANK_E`, 0 = `INTC_GS`).
3. **VBlank is delivered at 59.94 Hz of virtual time** and the guest's VBlank handler runs, at a
   point where no recompiled function is mid-instruction.
4. `func_00500B40` returns non-zero after the first VBlank (the table is populated).

**Syscalls** (`syscallnr.h`): `AddIntcHandler 0x10`, `RemoveIntcHandler 0x11`, `AddDmacHandler 0x12`,
`RemoveDmacHandler 0x13`, `_EnableIntc 0x14`, `_DisableIntc 0x15`, `_EnableDmac 0x16`,
`_DisableDmac 0x17`, `iEnableIntc -0x1A`, `iDisableIntc -0x1B`, `iEnableDmac -0x1C`,
`iDisableDmac -0x1D`, `EnableIntcHandler 0x5C`, `DisableIntcHandler 0x5D`, `EnableDmacHandler 0x5E`,
`DisableDmacHandler 0x5F`, `SetVSyncFlag 0x73`, `SetGsCrt 0x2`.

**Causes** (`kernel.h` enum): 0 `INTC_GS`, 1 `INTC_SBUS`, 2 `INTC_VBLANK_S`, 3 `INTC_VBLANK_E`,
4 `INTC_VIF0`, 5 `INTC_VIF1`, 6 `INTC_VU0`, 7 `INTC_VU1`, 8 `INTC_IPU`, 9/10/11 `INTC_TIM0/1/2`,
13 `INTC_SFIFO`, 14 `INTC_VU0WD`.

**State**: `INTC_STAT 0x1000F000`, `INTC_MASK 0x1000F010`; the guest also reads them through the
uncached mirror at `0xB000F000`/`0xB000F010`.

---

## M2 — The IOP, through SIF RPC

**Why.** Everything the game reads comes from the IOP: the CDVD driver (level and texture files),
`LIBSD` (sound), `PADMAN` (input), `MCMAN` (saves), `MCSERV`. The EE reaches them with SIF RPC
packets over DMA channels 5 (SIF0) and 6 (SIF1); the disc has 17 IRX modules under `IRX/`.

**Criterion:** the game's first `sceSifBindRpc` completes and the library's service list is
returned; then `CdRead` for a file on `TT.VOL` returns its first sector with the bytes matching a
direct ISO read.

**Syscalls / registers**: `SifSetDma 0x77`, `SifDmaStat 0x76`, `SifSetDChain 0x78`, `SifSetReg 0x79`,
`SifGetReg 0x7A`, `SifStopDma 0x6B`; DMA channel registers `0x1000C000`+ (SIF0) and `0x1000C400`+
(SIF1); `SBUS` interrupt cause 1.

**Approach**: implement the SIF registers and the RPC packet format (ps2sdk `ee/rpc/`, `ee/sif`),
then serve the six services natively instead of running the IRX modules — the modules are R5900
code for the IOP, a second CPU this port does not emulate.

---

## M3 — The GS

**Why.** Nothing appears on screen without it. The game sends GIF packets (DMA channel 2) that set
registers and write vertices; the port already has `port/gs/gs.c` (from downhill-revived) for the
register store and screen-space resolution.

**Criterion:** a frame rendered by the port produces a picture byte-identical to PCSX2's for the
same scripted input and the same virtual time, at 640×448.

**Registers** (ps2tek): privileged block at `0x12000000` (`PMODE`, `SMODE2`, `DISPFB1/2`,
`DISPLAY1/2`, `BGCOLOR`, `CSR`, `IMR`, `SIGLBLID`); the GIF-indexed block from `0x00 PRIM` through
`TEX0-2`, `CLUT`, `FRAME`, `ZBUF`, `BITBLTBUF`, `TRXPOS`, `TRXREG`, `TRXDIR`, `SCISSOR`, `TEST`,
`ALPHA`, `FOG`, `SIGNAL`, `FINISH`. DMA channel 2.

**Approach**: feed `port/gs/gs.c` from the VIF1 path (M4), present through the existing renderer,
and use PCSX2's GS as the behavioural oracle.

---

## M4 — The VU1 draw path

**Why.** The GS is a rasteriser with no transform stage: all geometry, clipping and lighting is VU1
microcode that the game uploads through VIF1.

**Criterion:** for a captured draw list, VU1 output (XGKICK packets) matches PCSX2's under the same
virtual clock; a frame at M3's criterion is reachable through this path.

**Registers / memory**: VU1 micro memory `0x11008000` (16 KiB), data memory `0x1100C000` (16 KiB);
VIF1 registers `0x10009000`+; DMA channel 1. The unpacker and the interpreter are already in the
port (`port/hle/vif.c`, `port/runtime/vu0_interp.c`).

**Approach**: capture the microprogram at boot (VIF1 `MPG`), translate its straight-line blocks to C
the way `port/tools/vu1_aot.py` does for downhill-revived, keep the interpreter for the rest, and
compare against PCSX2 on a recorded pad script.

---

## M5 — Pad and sound

**Why.** The game cannot be played without input, and many boot paths wait for a pad reply.

**Criterion:** a scripted pad sequence produces the same guest state as PCSX2; sound output
(SPU2 through `LIBSD`) matches the reference WAV for a fixed scripted run.

**Pad**: `PADMAN` service through SIF RPC (M2), `DMA channel 9`/`SIF0` for the reply, interrupt
cause 1 (`SBUS`) — the game also reads `0x1000F000` for the handshake.
**Sound**: `LIBSD` service, SPU2 registers at `0x1F801C00` (IOP side), DMA channel 4.

---

## What is already true and must stay true

- `check_image`-style proof: every `src/` unit still matches the retail bytes; the port never
  changes `src/`.
- One guest-address translation (`ps2_guest_address`), closed map, tested.
- 31,416 recompiled functions, 162 syscall wrappers bound by address.

---

## Order and why

```
M1 interrupts  ──> M2 IOP/SIF ──> M5 pad, sound
       │                │
       └──────────────> M3 GS ──> M4 VU1
```

M1 is a prerequisite for all four: SIF replies arrive as interrupts, GS completion is an interrupt,
VU1 raises INTC_VU1, and the pad handshake is interrupt-driven. That is why it is first and why it
gets its own tests before the boot is touched again.
