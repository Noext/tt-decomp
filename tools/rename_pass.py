#!/usr/bin/env python3
"""Rewrite a source's call targets so they point where this build puts them.

A source that differs from its EU function only in jump and `lui` targets needs no logic change:
its calls name US functions, which moved. The compiled object knows exactly which name sits at
each `jal` (the relocation), and the target function's disassembly knows the EU address that call
goes to, at the same instruction index. Rewriting those names, and only those, is precise — the
function's own name and everything else is left alone.

usage: rename_pass.py TARGET.elf --splat DIR SOURCE ADDR [-o OUT]
Exit 0 when the rewritten source compiles to the target's exact words.
"""
import argparse
import os
import re
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pair_sources import load_words, splat_functions, COMPILER  # noqa: E402

R_MIPS_26 = 4
R_MIPS_32 = 2


def object_text_relocs(path, extra_syms=None):
    """(words, {word_index: symbol name}) for the object's .text, from its relocations.

    `extra_syms` names further read-only sections (`.rodata`, `.data`) whose words carry
    relocations too: a jump table or a callback table names the functions it points at.
    """
    d = open(path, 'rb').read()
    e_shoff, = struct.unpack_from('<I', d, 32)
    shentsize, shnum, shstr = struct.unpack_from('<HHH', d, 46)
    S = [struct.unpack_from('<IIIIIIIIII', d, e_shoff + i * shentsize) for i in range(shnum)]
    so = S[shstr][4]
    nm = lambda o: d[so + o:d.index(b'\0', so + o)].decode('latin1')
    names = [nm(s[0]) for s in S]
    ti = names.index('.text')
    text = d[S[ti][4]:S[ti][4] + S[ti][5]]
    words = list(struct.unpack_from('<%dI' % (len(text) // 4), text))
    relocs = {}
    data = {}
    extra = {names.index(x) for x in (extra_syms or ()) if x in names}
    for ti2 in ({ti} | extra):
        sec = S[ti2]
        blob = list(struct.unpack_from('<%dI' % (sec[5] // 4), d[sec[4]:sec[4] + sec[5]]))
        for s in S:
            if s[1] != 9 or s[7] != ti2:      # SHT_REL, sh_info names the section it patches
                continue
            symtab = S[s[6]]
            strtab = S[symtab[6]][4]
            for k in range(s[5] // 8):
                r_off, r_info = struct.unpack_from('<II', d, s[4] + k * 8)
                if r_info & 0xFF not in (R_MIPS_26, R_MIPS_32):
                    continue
                sym = r_info >> 8
                st_name, = struct.unpack_from('<I', d, symtab[4] + sym * 16)
                name = d[strtab + st_name:d.index(b'\0', strtab + st_name)].decode('latin1')
                if ti2 == ti:
                    relocs[r_off // 4] = name
                else:
                    data[(names[ti2], r_off // 4)] = name
    return words, relocs, data


def resolve(words, relocs):
    """Apply the object's R_MIPS_26 relocations as the linker would (the symbol's own address)."""
    out = list(words)
    for k, sym in relocs.items():
        if k >= len(out):
            continue
        addr = None
        m = re.fullmatch(r'func_([0-9A-Fa-f]{8})', sym)
        if m:
            addr = int(m.group(1), 16)
        else:
            try:
                import symbols
                addr = symbols.address_of(sym)
            except Exception:
                addr = None
        if addr is not None:
            out[k] = (out[k] & 0xFC000000) | ((addr >> 2) & 0x3FFFFFF)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('target')
    ap.add_argument('--splat', required=True)
    ap.add_argument('source')
    ap.add_argument('addr')
    ap.add_argument('-o', '--out')
    ap.add_argument('--include', default='include')
    a = ap.parse_args()

    base, words = load_words(a.target)
    funcs = splat_functions(a.splat)
    addr = int(a.addr, 16)
    n = funcs[addr]
    i = (addr - base) // 4
    target = list(words[i:i + n])

    work = tempfile.mkdtemp(prefix='rename-')
    obj = os.path.join(work, 'a.o')
    r = subprocess.run([f'{COMPILER}/bin/ee-gcc', '-c', '-B', f'{COMPILER}/bin/ee-', '-O2', '-G0',
                        '-fno-exceptions', '-fno-strict-aliasing', f'-I{a.include}', a.source,
                        '-o', obj], capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(f'the source does not compile: {(r.stderr.strip().splitlines() or ["?"])[-1]}')
    mine, relocs, data_relocs = object_text_relocs(obj, ('.rodata', '.data'))
    if len(mine) != len(target):
        sys.exit(f'shape mismatch: the source has {len(mine)} instructions, the target {n}')

    text = open(a.source).read()
    renamed = 0
    mapping = {}
    for k, sym in relocs.items():
        w = target[k]
        if w >> 26 not in (2, 3):
            continue
        dest = ((k * 4 + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
        new = f'func_{dest:08X}'
        if sym == new or not sym.isidentifier() or sym.startswith('.'):
            continue
        # The target calls the function at `dest`. When the source named it func_USaddr, the new
        # name is func_EUaddr; when it used a real name (a C++ method), the address is the only
        # thing that changed, so the name stays and only its declaration moves.
        if re.fullmatch(r'func_[0-9A-Fa-f]{6,8}', sym):
            text, count = re.subn(r'(?<![\w])' + re.escape(sym) + r'(?![\w])', new, text)
            renamed += count
        else:
            declared = re.search(r'\b' + re.escape(sym) + r'\b[^;\n]*;', text)
            if declared and f'{sym} = 0x{dest:08X}' not in text:
                mapping.setdefault(sym, dest)
    out = a.out or a.source
    open(out, 'w').write(text)

    obj2 = os.path.join(work, 'b.o')
    r = subprocess.run([f'{COMPILER}/bin/ee-gcc', '-c', '-B', f'{COMPILER}/bin/ee-', '-O2', '-G0',
                        '-fno-exceptions', '-fno-strict-aliasing', f'-I{a.include}', out, '-o', obj2],
                       capture_output=True, text=True)
    if r.returncode != 0:
        print(f'{os.path.basename(a.source)}: {renamed} renames, does not recompile')
        sys.exit(2)
    w2, rel2, _ = object_text_relocs(obj2, ('.rodata', '.data'))
    w2 = resolve(w2, rel2)
    diff = sum(1 for x, y in zip(w2, target) if x != y) + abs(len(w2) - len(target))
    if mapping:
        print('  symbol mappings needed: ' +
              ', '.join(f'{n}=0x{d:08X}' for n, d in sorted(mapping.items())))
    print(f'{os.path.basename(a.source)} -> {out}: {renamed} renames, {diff} words differ')
    sys.exit(0 if diff == 0 else 1)


if __name__ == '__main__':
    main()
