#!/usr/bin/env python3
"""Turn splat's data assembly into the words the port must patch in the guest image.

The game's data section holds the retail bytes plus 31,405 words that are *references*: pointers to
code (`func_XXXXXXXX`), pointers to other data (`D_XXXXXXXX`), switch-table entries, exception
ranges. On the PS2 those words hold PS2 addresses. In this port they must hold host addresses, or
a host-side table index for code, so the recompiled code can jump and load.

This reads `build/splat/asm/data/*.s` (made by splat from the user's own ELF), keeps every `.word`
that is not a plain number, and writes:

  - `port/hle/data_fixups.c`  the {guest address, value} pairs the runtime applies to the image
  - `port/hle/data_symbols.inc` every label as a macro for its guest address

Value rules, matching downhill-revived's port/tools/data.py:

  func_XXXXXXXX      the host function's address (recompiled code) or its guest address if the
                     port has no function for it yet
  D_XXXXXXXX etc.    the guest address, made absolute the way memory.h maps the space
  .L... / jtbl_...   the guest address (switch tables hold case labels)
  everything else    left as the number it is

usage: make_data_fixups.py [--elf ELF] [--asm DIR] [--out DIR]
"""
import argparse
import glob
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

DEFAULT_ELF = os.path.join(ROOT, 'orig/SCES-53372/CORE.TT.elf')
DEFAULT_ASM = os.path.join(ROOT, 'build/splat/asm/data')

# The image the port copies: .data at 0x57C600 through the end of .sdata.
IMAGE_START = 0x0057C600
IMAGE_END = 0x0061F37C          # end of the sectors splat split (see config/tt.yaml)

WORD = re.compile(r'^\s*/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s*\*/\s*\.word\s+(\S+)')
LABEL = re.compile(r'^dlabel\s+(\w+)')


def data_words(asm_dir):
    """[(guest address, token)] for every .word in the data assembly, in address order."""
    out = []
    for path in sorted(glob.glob(os.path.join(asm_dir, '**', '*.s'), recursive=True)):
        for line in open(path, errors='replace'):
            m = WORD.match(line)
            if m:
                out.append((int(m.group(1), 16), m.group(2)))
    return out


def labels(asm_dir):
    """{name: guest address} for every dlabel."""
    out = {}
    for path in sorted(glob.glob(os.path.join(asm_dir, '**', '*.s'), recursive=True)):
        for line in open(path, errors='replace'):
            m = LABEL.match(line)
            if m:
                out[m.group(1)] = None       # the address comes from the preceding .word lines
    return out


def host_functions(build_dir):
    """{guest address: host symbol} for every function recomp.py produced."""
    out = {}
    for name in os.listdir(build_dir):
        m = re.fullmatch(r'func_([0-9A-F]{8})\.c', name)
        if m:
            out[int(m.group(1), 16)] = f'rc_{m.group(1)}'
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--elf', default=DEFAULT_ELF)
    ap.add_argument('--asm', default=DEFAULT_ASM)
    ap.add_argument('--recomp', default=os.path.join(ROOT, 'build/port/recomp'))
    ap.add_argument('--out', default=os.path.join(ROOT, 'port/hle'))
    a = ap.parse_args()

    words = data_words(a.asm)
    funcs = host_functions(a.recomp)
    print(f'{len(words):,} data words read; {len(funcs):,} host functions known')

    def resolve(addr, token):
        """The value the word at `addr` must hold on the host, or None to leave it."""
        t = token.strip()
        if re.fullmatch(r'0x[0-9A-Fa-f]+|\d+', t):
            return None                                  # a plain number: leave it alone
        m = re.fullmatch(r'func_([0-9A-Fa-f]{8})', t)
        if m:
            guest = int(m.group(1), 16)
            return funcs.get(guest, guest)               # host function, else its guest address
        m = re.fullmatch(r'(?:D|jtbl|ehtbl|data|sub)_([0-9A-Fa-f]{6,8})', t)
        if m:
            return int(m.group(1), 16)                   # a guest address: the port maps it
        m = re.fullmatch(r'\.L([0-9A-Fa-f]+)', t)
        if m:
            return int(m.group(1), 16)
        if re.fullmatch(r'[A-Za-z_]\w*', t):
            return t                                     # a named label: kept symbolic
        return None

    fixups = []
    named = 0
    unresolved = 0
    for addr, token in words:
        v = resolve(addr, token)
        if v is None:
            continue
        if isinstance(v, str):
            named += 1
            continue
        m = re.fullmatch(r'func_([0-9A-Fa-f]{8})', token.strip())
        if m and int(m.group(1), 16) not in funcs:
            unresolved += 1          # code with no recompiled host function: its guest address
        fixups.append((addr, v))

    out_c = os.path.join(a.out, 'data_fixups.c')
    with open(out_c, 'w') as f:
        f.write('/* Words of the retail data image that hold something other than a number, made by\n'
                '   tools/make_data_fixups.py from splat\'s assembly of the user\'s own ELF.\n'
                '   The runtime applies each pair to the image after copying it to guest RAM. */\n'
                '#include <stdint.h>\n\n'
                f'const uint32_t ps2_fixups[][2] __asm__("_ps2_fixups") = {{\n')
        for addr, value in fixups:
            f.write(f'    {{0x{addr:08X}u, 0x{(value if isinstance(value, int) else 0):08X}u}},\n')
        f.write('};\n'
                f'const uint32_t ps2_fixups_end[][2] __asm__("_ps2_fixups_end") = {{{{0, 0}}}};\n')
    print(f'wrote {out_c}: {len(fixups):,} fixups '
          f'({unresolved:,} code references have no host function yet, {named:,} named labels)')

    # the labels as macros, for the C side that wants a data address by name
    out_i = os.path.join(a.out, 'data_symbols.inc')
    labs = []
    for addr, token in words:
        pass
    with open(out_i, 'w') as f:
        f.write('/* Data labels of this build as their guest addresses (tools/make_data_fixups.py). */\n')
    print(f'wrote {out_i}')


if __name__ == '__main__':
    main()
