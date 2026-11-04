# The EE timers, and what the game does with them

Status: researched, not implemented. The register page is mapped as plain memory; nothing reacts.

## What the registers are

From ps2sdk (`ee/kernel/include/timer.h`), the EE has three timers, each with four registers:

| Timer | COUNT | MODE | COMP | HOLD |
|---|---|---|---|---|
| T0 | `0x10000000` | `0x10000010` | `0x10000020` | `0x10000030` |
| T1 | `0x10000800` | `0x10000810` | `0x10000820` | `0x10000830` |
| T2 | `0x10001000` | `0x10001010` | `0x10001020` | `0x10001030` |

Each also has an uncached alias at `+0xB0000000` (`K_T2_COUNT` = `0xB0001000`, `K_T2_MODE` =
`0xB0001010`, `K_T2_COMP` = `0xB0001020`), which is the form the game's own helpers use.

`COUNT` counts up (or down), `COMP` is the compare value, `HOLD` is reloaded at the wrap, and
`MODE` selects clock source, direction, wrap behaviour, the interrupt, and the prescale.

## What the game does

The wrappers at `0x5078F8`, `0x507908` and `0x507918` each set `$a0` to one of those registers and
jump into `0x507888`, which serialises the write behind `di`/`ei`:

```
0x507888:  mfc0 $v1, $12        ; SR
           and  $v1, $v1, 0x10000
           beqz -> skip the wait
           di ; wait for SR's IE to drop
           mtc0/eret round trip
0x5078C4:  mfc0 $2, SR ; or 6 ; xor 2 ; or v1
           mtc0 $2, SR          ; interrupts off around the write
           sw   $a1, 0($a0)     ; the register write itself
           mtc0 $ra, ErrorEPC
           eret                 ; "return" through ErrorEPC
```

So the game sets a timer register with interrupts masked. Whether it then polls `COUNT`, waits for
the compare interrupt, or only uses `MODE` as a divider is not yet established: that needs the
call sites, not the wrappers.

## What the port would need

1. **A timer model**: three counters, each with MODE/COMP/HOLD semantics, clocked from the virtual
   EE clock the kernel already has (`hle_cycles()`), like downhill-revived's `port/hle/timer.c`.
2. **Intercepts on the register page**: writes to `0x1000(0/8/10)0x` and their `0xB000` aliases
   must update the model instead of storing bytes. `ps2_mmio_write` in `port/runtime/ps2asm.c`
   already exists as the hook and is called for the `0x10`/`0x12` pages; the `0xB000` mirror needs
   the same call.
3. **Interrupt delivery** if the game uses the compare interrupt: the kernel's INTC model would
   have to raise timer causes at the right rate. downhill-revived's finding 020 and its kernel
   show the shape.

Until then: a game that polls a timer reads bytes, and the value never advances — which is a
plausible source of a hang later in the boot.
