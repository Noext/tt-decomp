#!/usr/bin/env python3
"""The table that turns a guest code address into the host function that runs it.

The game hands code addresses around as plain numbers: callbacks in data, function pointers in
tables, vtable entries, `jalr` targets. On the PS2 they are addresses in the executable; here the
same code lives in the recompiled functions, at whatever address the host's linker chose. This
table is the bridge: `entry_of(guest)` gives the host function, 0 when there is none.

The data words that hold a code address are rewritten to an index into this table (written as
`data_code_refs.h`): an index is the only thing a 32-bit guest word can hold that means the same
on the host.

usage: make_entry_table.py [--fixups PATH] [--recomp DIR] [-o OUTDIR]
"""
import argparse
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)


def recompiled(dirpath):
    """[(guest address, host C name)] for every function recomp.py produced."""
    out = []
    for path in sorted(glob.glob(os.path.join(dirpath, 'func_*.c'))):
        m = re.fullmatch(r'func_([0-9A-F]{8})\.c', os.path.basename(path))
        if m:
            out.append((int(m.group(1), 16), f'func_{m.group(1)}'))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--recomp', default=os.path.join(ROOT, 'build/port/recomp'))
    ap.add_argument('-o', '--out', default=os.path.join(ROOT, 'port/hle'))
    a = ap.parse_args()

    funcs = recompiled(a.recomp)
    print(f'{len(funcs):,} recompiled functions')

    h = os.path.join(a.out, 'entry_table.h')
    with open(h, 'w') as f:
        f.write('/* Guest code address -> host function, made by tools/make_entry_table.py.\n'
                '   Sorted by guest address; entry_of() binary-searches it. */\n'
                '#ifndef ENTRY_TABLE_H\n#define ENTRY_TABLE_H\n\n#include <stdint.h>\n'
                '#include "recomp_decls.h"\n\n')
        f.write('typedef struct { uint32_t guest; uintptr_t host; } Entry;\n')
        f.write(f'#define ENTRY_COUNT {len(funcs)}\n\n#if defined(ENTRY_TABLE_DEFINE)\n')
        f.write('static const Entry entries[ENTRY_COUNT] = {\n')
        for guest, name in funcs:
            f.write(f'    {{0x{guest:08X}u, (uintptr_t)(void (*)(void)){name}}},\n')
        f.write('};\n#endif\n\n#endif\n')
    print(f'wrote {h}')

    # entry_table.c is hand-written: it only needs the generated header.
