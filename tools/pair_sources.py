#!/usr/bin/env python3
"""Pair compiled decompilation sources with the target build's functions, strictly.

The earlier graders under `tools/` accepted two different things as a match:

  * the same instruction words (a real carry-over), and
  * the same *opcode multiset*, which is not a match at all: `sw $v0,0x10($a0)` and
    `sw $v0,0xC($a0)` are genuinely different code with the same opcodes.

This tool pairs on the instruction *shape* instead: opcode, source and destination registers, and
the 6-bit function field must be equal, and only the immediate may differ. A pair is then

  exact   every word equal
  imm     every shape equal; only immediates differ (struct offsets, constants, frame sizes)
  shape   every shape equal but the immediate masks differ (address fields), reported separately

Functions are the target's splat boundaries (build/splat/asm/nonmatchings, from your own copy of
the game) or, with --calls, `jal` targets. A source's function is matched by shape against
functions of the SAME instruction count, so a different-length function is never reported as a
near miss.

usage: pair_sources.py TARGET.elf --splat ASM_DIR --src DIR --out pairs.tsv [--jobs N] [--limit N]
"""
import argparse
import collections
import concurrent.futures
import glob
import os
import re
import struct
import subprocess
import tempfile

COMPILER = os.path.expanduser('~/.local/share/gt4/ee-gcc2.96')


def load_words(path):
    d = open(path, 'rb').read()
    e_shoff, = struct.unpack_from('<I', d, 32)
    shentsize, shnum = struct.unpack_from('<HH', d, 46)
    S = [struct.unpack_from('<IIIIIIIIII', d, e_shoff + i * shentsize) for i in range(shnum)]
    secs = [dict(type=s[1], addr=s[3], off=s[4], size=s[5]) for s in S]
    code = max((s for s in secs if s['type'] == 1 and s['size'] > 1 << 20), key=lambda s: s['size'])
    return code['addr'], list(struct.unpack_from('<%dI' % (code['size'] // 4),
                                                 d[code['off']:code['off'] + code['size']]))


def splat_functions(asm_dir):
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


NORMAL = set(range(8, 16)) | {32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 46, 48, 49, 50,
                              51, 52, 53, 54, 55, 56, 57, 58, 62}
BRANCH = {1, 4, 5, 6, 7}            # beq/bne/blez/bgtz/beql/...: 16-bit offset


def shape(w):
    """(opcode-ish identity, immediate-present) for one instruction."""
    op = w >> 26
    if op in (2, 3):                                  # j / jal: 26-bit target
        return (op,), True
    if op == 0:                                       # SPECIAL: rs,rt,rd,shamt,funct
        return (0, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31, (w >> 6) & 31, w & 63), False
    if op == 1:                                       # REGIMM: rs, rt selects the operation
        return (1, (w >> 21) & 31, (w >> 16) & 31), True
    if op in BRANCH:                                  # rs, rt, 16-bit offset
        return (op, (w >> 21) & 31, (w >> 16) & 31), True
    if op == 15:                                      # lui: rt only
        return (op, (w >> 16) & 31), True
    if op in NORMAL:                                  # rs, rt, 16-bit immediate
        return (op, (w >> 21) & 31, (w >> 16) & 31), True
    if op == 16:                                      # cop0 rs,rt
        return (op, (w >> 21) & 31, (w >> 16) & 31), False
    if op == 17:                                      # cop1: rs,ft; cfc1/ctc1 have an rd
        rs = (w >> 21) & 31
        if rs in (2, 6):
            return (op, rs, (w >> 11) & 31, (w >> 16) & 31), False
        return (op, rs, (w >> 16) & 31), True
    if op == 18:                                      # cop2 (VU0 macro): rs, rt-ish
        return (op, (w >> 21) & 31, (w >> 16) & 31), True
    if op in (22, 23):                                # LQ/SQ: 16-bit offset
        return (op, (w >> 21) & 31, (w >> 16) & 31), True
    if op in (28, 30, 31):                            # MMI / SPECIAL2 / SPECIAL3
        return (op, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31, w & 63), False
    return (op, w & 0x3FFFFFF), True


def object_functions(path):
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
    words = list(struct.unpack_from('<%dI' % (tsize // 4), d[toff:toff + tsize]))
    out = []
    for s in S:
        if s[1] != 2:
            continue
        strtab = S[s[6]][4]
        for k in range(s[5] // 16):
            name, value, size, info, _, shndx = struct.unpack_from('<IIIBBH', d, s[4] + k * 16)
            if info & 0xF != 2 or shndx != ti or not size:
                continue
            off = (value - taddr) // 4
            n = size // 4
            if off < 0 or off + n > len(words):
                continue
            out.append((d[strtab + name:d.index(b'\0', strtab + name)].decode('latin1'), words[off:off + n]))
    return out or [('(whole)', words)]


def mask_addr(w):
    """Address-bearing fields only: jump/call targets and address-style `lui` halves."""
    op = w >> 26
    if op in (2, 3):
        return w & 0xFC000000
    if op == 15:
        return w & 0xFFFF0000 if 0x00100000 <= ((w & 0xFFFF) << 16) <= 0x00900000 else w
    return w


def classify(w, t):
    """exact: same words. addr: only jump/lui address fields differ. imm: same instruction shapes,
    real immediates differ (struct offsets, constants, frame sizes). None: not the same code."""
    if list(w) == list(t):
        return 'exact'
    if [mask_addr(x) for x in w] == [mask_addr(x) for x in t]:
        return 'addr'
    if [shape(x) for x in w] == [shape(x) for x in t]:
        return 'imm'
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('target')
    ap.add_argument('--splat', required=True)
    ap.add_argument('--src', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--jobs', type=int, default=(os.cpu_count() or 4))
    ap.add_argument('--limit', type=int, default=0)
    ap.add_argument('--compiler', default=COMPILER)
    ap.add_argument('--include', default='include')
    a = ap.parse_args()

    base, words = load_words(a.target)
    funcs = splat_functions(a.splat)
    by_len = collections.defaultdict(list)
    for addr, n in funcs.items():
        i = (addr - base) // 4
        if i + n <= len(words):
            by_len[n].append(addr)
    print(f'target: {len(funcs):,} functions, {len(words):,} words', flush=True)

    # shape index: (length, shape of every word) -> addresses. Exact by construction.
    index = collections.defaultdict(list)
    for n, addrs in by_len.items():
        for addr in addrs:
            i = (addr - base) // 4
            index[(n, tuple(shape(words[i + j]) for j in range(n)))].append(addr)
    print(f'shape keys: {len(index):,}', flush=True)

    srcs = sorted(p for p in glob.glob(os.path.join(a.src, '**', '*'), recursive=True)
                  if p.endswith(('.cpp', '.c', '.cc')))
    if a.limit:
        srcs = srcs[:a.limit]
    print(f'sources: {len(srcs):,}', flush=True)
    tmp = tempfile.mkdtemp(prefix='pair-')

    def one(items):
        i, src = items
        obj = os.path.join(tmp, f'{i}.o')
        r = subprocess.run([f'{a.compiler}/bin/ee-gcc', '-c', '-B', f'{a.compiler}/bin/ee-', '-O2',
                            '-G0', '-fno-exceptions', '-fno-strict-aliasing', f'-I{a.include}', src,
                            '-o', obj], capture_output=True, text=True)
        if r.returncode != 0 or not os.path.exists(obj):
            return [(src, '?', 0, 'compile error', None)]
        try:
            fns = object_functions(obj)
        except Exception:
            return [(src, '?', 0, 'compile error', None)]
        finally:
            try:
                os.remove(obj)
            except OSError:
                pass
        rows = []
        for name, w in fns:
            n = len(w)
            if n == 0:
                rows.append((src, name, 0, 'empty', None))
                continue
            key = (n, tuple(shape(x) for x in w))
            best = None
            for addr in index.get(key, ()):
                i2 = (addr - base) // 4
                c = classify(w, words[i2:i2 + n])
                if c:
                    best = (addr, c)
                    if c == 'exact':
                        break
            if best:
                rows.append((src, name, n, best[1], best[0]))
            else:
                rows.append((src, name, n, 'no pair', None))
        return rows

    def collect():
        stats = collections.Counter()
        found = []
        with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as ex:
            for k, rows in enumerate(ex.map(one, enumerate(srcs))):
                for src, name, n, st, at in rows:
                    stats[st] += 1
                    if at is not None:
                        found.append((src, name, n, st, at))
                if (k + 1) % 2000 == 0:
                    print(f'  {k + 1:>6,}/{len(srcs):,}', flush=True)
        return stats, found

    def source_order(src):
        """The US address in the file name, or the address in its own function name."""
        m = re.search(r'func_([0-9A-Fa-f]{8})', os.path.basename(src))
        return int(m.group(1), 16) if m else None

    def resolve(found):
        """Report only pairs where the evidence names one source and one target.

        The instruction shapes are a coarse fingerprint: every constructor with the same members
        has the same shape, so a shape can have hundreds of sources and hundreds of targets. When a
        shape group holds more than one source OR more than one target, the scan cannot say which
        goes with which, and assigning them by order would be a guess. Those go to `ambiguous` and
        are left for a check that can afford to test each source against each candidate.
        """
        by_shape = collections.defaultdict(list)
        for src, name, n, st, at in found:
            i2 = (at - base) // 4
            by_shape[(n, tuple(shape(x) for x in words[i2:i2 + n]))].append((src, name, n, st, at))
        accepted, ambiguous = [], []
        for shp, members in by_shape.items():
            sources = {r[0] for r in members}
            targets = {r[4] for r in members}
            if len(sources) == 1 and len(targets) == 1:
                accepted.append(members[0])
            else:
                ambiguous.extend(members)
        return accepted, ambiguous

    stats, found = collect()
    accepted, ambiguous = resolve(found)
    # A target claimed by two accepted pairs would still be a contradiction: drop that target.
    claims = collections.Counter(r[4] for r in accepted)
    accepted = [r for r in accepted if claims[r[4]] == 1]
    with open(a.out, 'w') as out:
        out.write('# source\tfunction\twords\tstatus\ttarget_addr\n')
        for src, name, n, st, at in sorted(accepted):
            out.write(f'{src}\t{name}\t{n}\t{st}\t{hex(at)}\n')
        for src, name, n, st, at in sorted(ambiguous):
            out.write(f'{src}\t{name}\t{n}\tambiguous\t{hex(at)}\n')

    total = sum(stats.values())
    print(f'\nfunctions: {total:,}')
    for k, v in stats.most_common():
        print(f'  {k:<14} {v:>7,}')
    n_acc = len(accepted)
    print(f'\nuniquely paired: {n_acc:,} ({100*n_acc/total:.1f}%)')
    print(f'ambiguous (a shape shared by several sources/targets): {len(ambiguous):,}')
    c = collections.Counter(r[3] for r in accepted)
    print('  exact {exact:,}, address-only {addr:,}, immediate-difference {imm:,}'.format(
        exact=c['exact'], addr=c['addr'], imm=c['imm']))


if __name__ == '__main__':
    main()
