# Tourist Trophy (Europe) — project state

Which build, what has been measured, and what to do next. Keep run logs and machine setup out of
this file. Measurements are reproducible with the tools named.

## Target

| | |
|---|---|
| Release | Tourist Trophy — The Real Riding Simulator (Europe), PAL, **SCES-53372** |
| Executable | `CORE.TT`, 1,855,359 bytes, SHA-1 `83591f000897ddd8ddd7050e199919505582268b` |
| Container | Salsa20 (key `PolyphonyDigital`) over raw-deflate; decoded by `tools/core2elf.py` |
| Load image | code `0x00100000–0x0057C5B0` (4,703,664 B), data `0x0057C600` (667,004 B), entry `0x00100008` |
| Compiler | Sony ee-gcc 2.96 (`Libgcc_2_96_ee_001003_1`), `-O2 -G0 -fno-exceptions` |
| Functions (splat) | 31,619 |
| Disc assets | `TT.VOL` 2.95 GB, same container as `GT4.VOL` (magic `ad90b9ac 02000200`) |

The upstream project's USA build (SCUS-97502) has the same layout shifted by +0x6190 at the code
end and +0x6190 at the data start, so the two are the same source tree compiled months apart.

## What carries over from the USA decompilation

`tools/src_map.py` compiles every `src/` source with the original compiler and looks for the
result among this build's splat functions, masking call targets and address-like `lui` halves.

| measurement | result |
|---|---|
| sources that compile to code present in this build | **16,104 / 22,501 = 71.6 %** (8,466 byte-identical) |
| EU code bytes those sources cover | 662,452 / 4,635,264 = 14.3 % |
| EU functions reached | 8,730 / 31,619 = 27.6 % |
| USA→USA control (self-test of the tool) | 100 %, 14,883/14,883 |

Per-source rows: `config/eu_source_map.tsv`.

The gap between 71.6 % of sources and 14.3 % of bytes is a property of the USA project itself: its
remaining work is concentrated in the large engine functions, and its matched sources are mostly
small. The large EU functions are matched next, in increasing size, using the USA source as the
starting point wherever one exists.

## Reusable cross-project tooling

- `tools/core2elf.py` — CORE container → ELF. Salsa20 verified against a reference implementation.
- `tools/src_map.py` — source → address map (this project's tool).
- `tools/eu_map.py` — US symbol file + US ELF → the other build's addresses.
- from `downhill-revived` (GPL-3.0-or-later), usable if a PC port starts: `port/runtime/`
  (MIPS/VU0 interpreter, memory), `port/tools/recomp.py` (static recompilation of functions that
  are still assembly).

## Next steps

1. **Match the large EU functions** with the USA source as a draft. Work from the biggest uncovered
   functions down (`config/eu_source_map.tsv` sorted by size gives the covered ones).
2. **Re-derive the symbol map** for the covered functions: `tools/eu_map.py` once a USA ELF exists,
   otherwise keep `func_<EU address>` names and let the source map supply the US name.
3. **Data**: `.data` and `.rodata` are not covered by any of this; splat split them and they need
   their own pass (`config/eu_source_map.tsv` covers code only).
4. **Port**: only after the code is matched far enough. The USA project has no port yet either;
   `downhill-revived`'s runtime and recompiler are the pieces to start from, not its game code.
