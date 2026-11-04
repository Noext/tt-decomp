#!/usr/bin/env python3
"""Classify every US decompilation source against the target build, with graded tolerance.

`src_map.py` answers yes/no after masking only call targets and address-like `lui` halves. This
answers *how far* each source is from the target, which separates three different jobs:

  exact     byte-identical: nothing to do
  addr      equal once call/jump targets and address-style `lui` are ignored: only symbols move
  immed     equal once *every* immediate is ignored too: the instruction sequence is the same,
            the struct offsets, frame sizes and constants differ (the PAL build's 512-line layout,
            its five-language tables, structs that gained members)
  close     >= CLOSE of the instructions agree, but not all: a few edits
  partial   >= PARTIAL: a rewrite of part of the function
  far       below PARTIAL: a different function
  length    no target function has this many instructions, so nothing to compare against

Objects may define several functions; each is classified on its own.

usage: src_fit.py TARGET.elf --splat ASM_DIR --src DIR --out fit.tsv [--jobs N]
"""
import argparse
import collections
import concurrent.futures
import glob
import os
import struct
import subprocess
import tempfile

COMPILER = os.path.expanduser('~/.local/share/gt4/ee-gcc2.96')
NONMATCHING = None
CLOSE = 0.90
PARTIAL = 0.70

IMM_OPS = set(range(8, 16)) | set(range(32, 63))   # addi..lui, loads/stores, cache, lwc2/sq/ld/sd


def load_words(path):
    d = open(path, 'rb').read()
    e_shoff, = struct.unpack_from('<I', d, 32)
    shentsize, shnum = struct.unpack_from('<HH', d, 46)
    S = [struct.unpack_from('<IIIIIIIIII', d, e_shoff + i * shentsize) for i in range(shnum)]
    secs = [dict(type=s[1], addr=s[3], off=s[4], size=s[5]) for s in S]
    code = max((s for s in secs if s['type'] == 1 and s['size'] > 1 << 20), key=lambda s: s['size'])
    words = list(struct.unpack_from('<%dI' % (code['size'] // 4), d[code['off']:code['off'] + code['size']]))
    return code['addr'], words


def splat_functions(asm_dir):
    import re
    pat = re.compile(r'\s*nonmatching\s+\w+,\s*(0x[0-9A-Fa-f]+)')
    out = {}
    for p in glob.glob(os.path.join(asm_dir, '**', '*.s'), recursive=True):
        with open(p, errors='replace') as f:
            for _ in range(4):
                m = pat.match(f.readline() or '')
                if m:
                    out[int(os.path.basename(p)[5:-2], 16)] = max(1, int(m.group(1), 16) // 4)
                    break
    return out


def mask_addr(w):
    op = w >> 26
    if op in (2, 3):
        return w & 0xFC000000
    if op == 15:
        return w & 0xFFFF0000 if 0x00100000 <= ((w & 0xFFFF) << 16) <= 0x00900000 else w
    return w


def mask_all(w):
    m = mask_addr(w)
    if (w >> 26) in IMM_OPS:
        return m & 0xFFFF0000
    return m


def object_functions(path):
    """[(symbol, words)] for every function defined in the object's .text."""
    d = open(path, 'rb').read()
    e_shoff, = struct.unpack_from('<I', d, 32)
    shentsize, shnum, shstr = struct.unpack_from('<HHH', d, 46)
    S = [struct.unpack_from('<IIIIIIIIII', d, e_shoff + i * shentsize) for i in range(shnum)]
    so = S[shstr][4]
    nm = lambda o: d[so + o:d.index(b'\0', so + o)].decode('latin1')
    names = [nm(s[0]) for s in S]
    if '.text' not in names:
        return []
    ti = names.index('.text')
    taddr, toff, tsize = S[ti][3], S[ti][4], S[ti][5]
    text = d[toff:toff + tsize]
    words = list(struct.unpack_from('<%dI' % (len(text) // 4), text))
    out = []
    for i, s in enumerate(S):
        if s[1] != 2:                      # SHT_SYMTAB
            continue
        strtab = S[s[6]][4]
        for k in range(s[5] // 16):
            st_name, st_value, st_size, info, other, shndx = struct.unpack_from('<IIIBBH', d, s[4] + k * 16)
            if info & 0xF != 2 or shndx != ti or not st_size:
                continue
            off = st_value - taddr
            n = st_size // 4
            if off < 0 or off // 4 + n > len(words):
                continue
            out.append((d[strtab + st_name:d.index(b'\0', strtab + st_name)].decode('latin1'), words[off // 4:off // 4 + n]))
    return out or [('(all)', words)]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('target')
    ap.add_argument('--splat', required=True)
    ap.add_argument('--src', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--jobs', type=int, default=(os.cpu_count() or 4))
    ap.add_argument('--compiler', default=COMPILER)
    ap.add_argument('--include', action='append', default=[])
    a = ap.parse_args()

    base, words = load_words(a.target)
    funcs = splat_functions(a.splat)
    by_len = collections.defaultdict(list)
    for addr, n in funcs.items():
        i = (addr - base) // 4
        if i + n <= len(words):
            by_len[n].append(addr)
    for v in by_len.values():
        v.sort()
    print(f'target: {len(funcs):,} functions, {len(words):,} words', flush=True)

    # two prefix indices: after address masking (tier "addr") and after all-immediate masking
    ia, im = collections.defaultdict(list), collections.defaultdict(list)
    for n, addrs in by_len.items():
        k = min(n, 4)
        for addr in addrs:
            i = (addr - base) // 4
            ia[(n, tuple(mask_addr(words[i + j]) for j in range(k)))].append(addr)
            im[(n, tuple(mask_all(words[i + j]) for j in range(k)))].append(addr)

    srcs = sorted(p for p in glob.glob(os.path.join(a.src, '**', '*'), recursive=True)
                  if p.endswith(('.cpp', '.c', '.cc')))
    includes = a.include or [os.path.join(a.src, '..', 'include')]
    tmp = tempfile.mkdtemp(prefix='fit-')
    print(f'sources: {len(srcs):,}', flush=True)

    def one(items):
        i, src = items
        obj = os.path.join(tmp, f'{i}.o')
        r = subprocess.run([f'{a.compiler}/bin/ee-gcc', '-c', '-B', f'{a.compiler}/bin/ee-', '-O2',
                            '-G0', '-fno-exceptions', '-fno-strict-aliasing',
                            *[f'-I{x}' for x in includes], src, '-o', obj],
                           capture_output=True, text=True)
        if r.returncode != 0 or not os.path.exists(obj):
            return [(src, '?', 0, 'compile error', None, 0.0)]
        try:
            fns = object_functions(obj)
        except Exception:
            fns = []
        finally:
            try:
                os.remove(obj)
            except OSError:
                pass
        rows = []
        for name, w in fns:
            n = len(w)
            if n == 0:
                rows.append((src, name, 0, 'empty', None, 0.0))
                continue
            k = min(n, 4)
            ka = (n, tuple(mask_addr(x) for x in w[:k]))
            km = (n, tuple(mask_all(x) for x in w[:k]))
            tier, at, score = None, None, 0.0
            for addr in ia.get(ka, ()):
                i2 = (addr - base) // 4
                if list(w) == words[i2:i2 + n]:
                    tier, at, score = 'exact', addr, 1.0
                    break
            if tier is None:
                for addr in ia.get(ka, ()):
                    i2 = (addr - base) // 4
                    if [mask_addr(x) for x in w] == [mask_addr(words[i2 + j]) for j in range(n)]:
                        tier, at, score = 'addr', addr, 1.0
                        break
            if tier is None:
                for addr in im.get(km, ()):
                    i2 = (addr - base) // 4
                    if [mask_all(x) for x in w] == [mask_all(words[i2 + j]) for j in range(n)]:
                        tier, at, score = 'immed', addr, 1.0
                        break
            if tier is None:
                target = by_len.get(n)
                if not target:
                    tier = 'length'
                else:
                    best, bat = 0.0, None
                    for addr in im.get(km, ()):
                        i2 = (addr - base) // 4
                        same = sum(1 for j in range(n) if mask_all(w[j]) == mask_all(words[i2 + j]))
                        sc = same / n
                        if sc > best:
                            best, bat = sc, addr
                    score = best
                    at = bat
                    tier = 'close' if best >= CLOSE else 'partial' if best >= PARTIAL else 'far'
            rows.append((src, name, n, tier, at, score))
        return rows

    stats = collections.Counter()
    with open(a.out, 'w') as out:
        out.write('# source\tfunction\twords\ttier\ttarget_addr\tscore\n')
        done = 0
        with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as ex:
            for rows in ex.map(one, enumerate(srcs)):
                done += 1
                for src, name, n, tier, at, score in rows:
                    out.write(f'{src}\t{name}\t{n}\t{tier}\t'
                              f'{at if at is None else hex(at)}\t{score:.3f}\n')
                    stats[tier] += 1
                if done % 2000 == 0:
                    print(f'  {done:>6,}/{len(srcs):,}', flush=True)

    total = sum(stats.values())
    print(f'\nfunctions: {total:,}')
    for k, v in stats.most_common():
        print(f'  {k:<14} {v:>7,}  {100*v/total:5.1f}%')
    fixed = stats['exact'] + stats['addr']
    print(f'\ncarried over as they stand: {fixed:,} ({100*fixed/total:.1f}%)')
    fixable = fixed + stats['immed'] + stats['close']
    print(f'plus immediate-only and close: {fixable:,} ({100*fixable/total:.1f}%)')


if __name__ == '__main__':
    main()
