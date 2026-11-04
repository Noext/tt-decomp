# The EE address space, and how this port maps it

Every memory access the game's code makes goes through **one** function,
`ps2_guest_address()` in `port/runtime/ps2asm.h`. It classifies the address, returns the host
address to use, and **aborts** on anything it cannot classify. There is no second path: the
recompiled code's load/store macros (`M8`…`M128` in `port/runtime/recomp.h`), the inline-asm
runtime (`ps2_store32/64/128` in `port/runtime/ps2asm.c`) and the HLE all end up there.

That single choke point is deliberate. Two of the bugs that cost the most time in this port were
the same mistake in different places — a 64-bit host pointer leaking into guest state — and the
earlier per-access "if it looks like RAM, mask it" scattered through the code is what let them
hide.

## The map

The EE's 32-bit space is not flat. The same 32 MiB of RAM appears through several windows, the
hardware registers through two, and the scratchpad sits at a fixed address.

| Guest range | Size | What it is | Host mapping |
|---|---|---|---|
| `0x00000000-0x01FFFFFF` | 32 MiB | RAM, physical / KUSEG | `0x08000000 + addr` |
| `0x10000000-0x1000FFFF` | 64 KiB | EE hardware registers, cached | its own address |
| `0x11000000-0x1100FFFF` | 64 KiB | VU0 and VU1 micro/data memory | its own address |
| `0x12000000-0x12001FFF` | 8 KiB | GS registers | its own address |
| `0x20000000-0x21FFFFFF` | 32 MiB | RAM, uncached (KSEG1) | `0x08000000 + (addr & 0x1FFFFFF)` |
| `0x30000000-0x31FFFFFF` | 32 MiB | RAM, uncached accelerated | `0x08000000 + (addr & 0xFFFFFFF)` |
| `0x70000000-0x70003FFF` | 16 KiB | scratchpad | its own address |
| `0x80000000-0x81FFFFFF` | 32 MiB | RAM, KSEG0 (kernel view) | `0x08000000 + (addr & 0x1FFFFFF)` |
| `0xA0000000-0xA1FFFFFF` | 32 MiB | RAM, KSEG1 (kernel uncached) | `0x08000000 + (addr & 0x1FFFFFF)` |
| `0xB0000000-0xB000FFFF` | 64 KiB | EE hardware registers, uncached | its own address |

Cached code (`KUSEG`, `KSEG0`), uncached code (`KSEG1`) and the accelerated views differ only in
which bits select the window; the bytes behind them are the same. The port keeps **one** copy of
RAM, so all of those windows must fold onto it — otherwise a write through one view would not be
seen through another, and the game compares addresses taken from both.

Not mapped, and therefore a fault: `0x40000000-0x6FFFFFFF`, `0x02000000-0x0FFFFFFF`,
`0x82000000-0x9FFFFFFF`, `0xC0000000-0xFFFFFFFF` (KSSEG and KSEG3, see below), `0x70004000` and
up, anything above 32 bits, and — importantly — `0x08000000-0x09FFFFFF`, which is the host's own
address for the RAM block.

## Two ranges that are deliberately NOT mapped

### `0xC0000000`: KSSEG, a TLB-mapped segment

Per the EE's MIPS segment layout (ps2tek, "EE segment layout"), the segments are:

| Segment | Range | Translation |
|---|---|---|
| KUSEG | `0x00000000-0x7FFFFFFF` | user, **TLB** |
| KSEG0 | `0x80000000-0x9FFFFFFF` | kernel, direct-mapped, cached |
| KSEG1 | `0xA0000000-0xBFFFFFFF` | kernel, direct-mapped, uncached |
| KSSEG | `0xC0000000-0xDFFFFFFF` | supervisor, **TLB** |
| KSEG3 | `0xE0000000-0xFFFFFFFF` | kernel, **TLB** |

Only KSEG0 and KSEG1 are direct-mapped (`physical = virtual & 0x1FFFFFFF`). `0xC0000000` has **no
fixed physical target**: it is whatever the TLB says, and the port has no TLB, so mapping it onto
RAM would be a guess that happens to work until the game programs a mapping. It faults instead.

That is safe here because the game does not use it: of the 20 `lui 0xC000` sites in the image, 19
feed `mtc1` (they load a float constant `0xC0000000` = −2.0), and the one left is not an address.
The 999 data words in that numeric range are floats too (`0xc3a00000` = −320.0, `0xc2000000` =
−32.0). No `mtc0` writes a TLB entry, and the game makes no `PutTLBEntry` call.

### `0x38000000`: one host's private mirror, not a PS2 address

Earlier versions of this port accepted `0x38000000-0x39FFFFFF` and folded it by subtracting
`0x30000000`. That was wrong twice:

- **It is not a PS2 region.** Nothing in the EE's map uses it, and the game never emits it as an
  address: its two `lui 0x3800` sites load a float, and the 913 data words in that range are
  input-encoder bit patterns (`0x38520001`, `0x38420003` — a button and a value), not pointers.
- **Its fold produced a host address.** `addr - 0x30000000` is exactly `0x08000000 + offset`, the
  host's own pointer for the block. Accepting it meant a truncated 64-bit host pointer landing
  there would have been treated as valid guest memory and silently written.

The range is removed, and its tests now assert that `0x38000000`, `0x38076000` and `0x38520001`
all fault.

### The host image address must fault

`0x08076000` is *this host's* address for guest `0x00076000`. A 64-bit host pointer cut down to 32
bits can look exactly like it, so the map must reject it by range: `0x08000000-0x09FFFFFF` is
outside every guest window, and `test_host_image_address_faults` pins that down.

### Hardware registers, VU memory and the scratchpad keep their address

Those pages are mapped **at their PS2 addresses** by `ps2_memory_init()`
(`port/runtime/memory.c`), so the classifier returns them unchanged. This is not a convenience: the
game takes pointers to them, does arithmetic on those pointers, and hands them to DMA. Folding them
somewhere else would break that arithmetic, and the DMA tags hold guest addresses as data.

The EE's register page has two windows because the game uses both: `0x10000000` for the cached one
and `0xB0000000` for the uncached one, which is how the SDK's timer and DMA wrappers reach it
(`0xB0001010`, `0xB0001000`, …).

## Why `0x80076000` becomes `0x08076000` and not `0x00076000`

`0x80076000` is the kernel's view of **physical** `0x00076000`: the same byte the game reaches at
`0x00076000`. The host keeps exactly one image of RAM, mapped at `0x08000000 + physical`, because
`0x08000000` is what the project's data stage chose (`PS2_RAM_BASE` in `port/tools/data.py`), and
the game's data files hold absolute PS2 pointers that are relocated to land there.

The fold is

```
host = 0x08000000 + (guest & 0x01FFFFFF)
```

The mask is what removes the window: `0x80076000 & 0x01FFFFFF = 0x00076000`, and
`+ 0x08000000` gives `0x08076000`. The same arithmetic works for all six plain RAM windows, which
is why one mask covers them; the two accelerated views use `0x0FFFFFFF` instead because their
window bit overlaps the mask.

Folding to `0x00076000` would be wrong twice:

1. **Nothing is mapped there.** `0x00000000-0x003FFFFF` belongs to the loader and, in a non-PIE
   binary, to the program itself. A dereference would fault — which is exactly what happened
   before the fold existed: the crash was `0x08076000` unmapped because the value written was the
   guest address itself.
2. **If something were mapped there, it would be a second copy.** The host would then have two
   images of the same 32 MiB that diverge on the first write: the game writes through the KSEG0
   view, reads through the physical view, and sees stale bytes. The failure would look like data
   corruption deep inside the game, not like an addressing bug.

The port also maps the block at `0x08000000` (the cached view), `0x28000000` and `0x38000000`
(uncached and accelerated), so code that computes those addresses directly still lands on the same
bytes.

## The rules the classifier enforces

- **One classification function.** `ps2_guest_address()` is the only place that knows the table.
  `ps2_ram_canonical()`, `ps2_memory_address()` and `ps2_load_address()` are thin wrappers over it.
- **Fail closed.** An address outside the table calls `ps2_address_fault()`, which prints the
  address and the caller and aborts. It is never "possibly RAM, try it".
- **No 32-bit truncation of host pointers.** An address above `0xFFFFFFFF` is a host pointer that
  leaked into guest state; it faults rather than being masked down to something that looks valid.
  `PS2_AUDIT=1` reports these at the top of the call stack instead of only at the dereference.
- **MMIO is fenced before translation.** `ps2_memory_address()` runs `ps2_graphics_sync()` for the
  VU1/GS ranges the EE can read back, then classifies — so a read after a write sees the write.

## Tests

`port/tests/test_map.c` holds the contract, and needs no disc image:

| Test | What it pins down |
|---|---|
| `test_ram_windows` | all six RAM windows (and `0x38000000`) fold to the one mapped block |
| `test_window_bit_is_the_only_difference` | a window bit changes nothing about the bytes |
| `test_mmio_cached` / `test_mmio_uncached_mirror` | `0x1000F000` and `0xB0001010` keep their address |
| `test_vu_and_gs` | VU0/VU1 memory and the GS registers keep their address |
| `test_scratchpad` | `0x70000000`…`0x70003FFF` |
| `test_unmapped_faults` | gaps, block ends and `0xDEADBEEF` abort |
| `test_host_pointer_faults` | a 64-bit pointer aborts instead of being truncated |
| `test_helpers_agree` | the three wrappers return what the classifier says |
| `test_host_image_address_faults` | `0x08076000` (the host's own pointer) faults |
| `test_no_port_private_windows` | `0x38000000` and encoder patterns like `0x38520001` fault |
| `test_ksseg_is_not_ram` | `0xC0000000` (KSSEG) faults until a TLB exists |
| `test_map_is_closed` | sampling the whole 32-bit space: a fault exactly outside the windows |

Faults are checked in a forked child, so the test proves the real `abort()` path instead of
replacing it.

    cd port/tests && make test_map && ./test_map

## MMIO pages are plain memory today

The register pages are mapped, so writes land somewhere and nothing crashes. They are **not**
emulated: a write to `0xB0001010` (`T2_MODE`, the EE's Timer 2 mode register) stores a number into
a byte of RAM and no timer reacts. What the game does with those registers and what the port would
have to implement is recorded in `docs/findings/ee-timers.md`.

## Adding a range

Add it to `ps2_guest_address()`, to the table in this file, and to `test_map_is_closed()`'s window
list. If it needs a page of its own, map it in `ps2_memory_init()`. Never add a per-access special
case somewhere else: that is how the two host-pointer leaks got in.
