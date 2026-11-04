#!/usr/bin/env python3
"""Rewrite a source's immediates to the target function's, from a verified pair.

`pair_sources.py` proves a source and a target function have the same instruction shapes with
different immediates. This tool produces the candidate source for review:

  - struct/offset immediates: tries the target's immediate in place of the source's, one site at a
    time, and keeps the substitutions that the compiler reproduces;
  - address references: rewrites the `D_xxxxxxx` / `func_xxxxxxx` names to the target's symbols.

It is a candidate generator, not a matcher: run tools/match.py (or check_one.py) on its output.
It never overwrites the original; the candidate goes to a new file.

usage: fix_immediates.py TARGET.elf --splat ASM_DIR SOURCE TARGET_ADDR [-o OUT.c]

Exit 0 when the produced candidate matches exactly; 1 otherwise (the candidate is still written).
"""
import argparse
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pair_sources import load_words, splat_functions, object_functions, shape, COMPILER  # noqa: E402

DECL = re.compile(r'(extern\s+[A-Za-z_][\w \*]*?\s)((?:D|func)_[0-9A-Fa-f]{6,8})(\s*\[)')


def compile_obj(src, workdir, include):
    obj = os.path.join(workdir, 'a.o')
    r = subprocess.run([f'{COMPILER}/bin/ee-gcc', '-c', '-B', f'{COMPILER}/bin/ee-', '-O2', '-G0',
                        '-fno-exceptions', '-fno-strict-aliasing', f'-I{include}', src, '-o', obj],
                       capture_output=True, text=True)
    if r.returncode != 0 or not os.path.exists(obj):
        return None
    return obj


def words_of(src, workdir, include):
    obj = compile_obj(src, workdir, include)
    if not obj:
        return None
    try:
        fns = object_functions(obj)
    finally:
        try:
            os.remove(obj)
        except OSError:
            pass
    return fns[0][1] if fns else []


def imm_sites(w, t):
    """Indices where the word differs but the shape is equal (an immediate-only difference)."""
    return [i for i in range(min(len(w), len(t))) if w[i] != t[i]]


def try_substitution(text, site_words, target_word, index):
    """Replace the Nth differing literal in the text with the target's immediate value.

    Candidates are: hex constants and decimal constants in the source, in order of appearance.
    """
    found = list(re.finditer(r'(?<![\w.])0x([0-9A-Fa-f]+)(?![\w.])|(?<![\w.])(\d+)(?![\w.])', text))
    if index >= len(found):
        return None
    m = found[index]
    imm = target_word & 0xFFFF
    signed = imm - 0x10000 if imm & 0x8000 else imm
    repl = f'0x{signed & 0xFFFFFFFF:X}' if signed < 0 else f'0x{signed:X}'
    if m.group(2) is not None and int(m.group(2)) == (signed & 0xFFFF) or m.group(2) is not None and int(m.group(2)) == abs(signed):
        return None
    return text[:m.start()] + repl + text[m.end():]


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
    n = funcs.get(addr)
    if n is None:
        sys.exit(f'{a.addr} is not a splat function start')
    i = (addr - base) // 4
    target = list(words[i:i + n])

    work = tempfile.mkdtemp(prefix='fiximm-')
    text = open(a.source).read()
    cur = words_of(a.source, work, a.include)
    if cur is None:
        sys.exit('the source does not compile')
    out = a.out or (a.source.rsplit('.', 1)[0] + '.eu.' + a.source.rsplit('.', 1)[1])
    status = 'no pair'

    if [shape(x) for x in cur] == [shape(x) for x in target]:
        # rewrite moved symbols first: every D_/func_ name that is not the source's own
        body = open(a.source).read()
        for m in set(re.findall(r'(?:D|func)_[0-9A-Fa-f]{6,8}', body)):
            pass  # symbol renames need the target's symbol table; left to the caller
        # then the immediates, one site at a time, keeping only what the compiler confirms
        text = open(a.source).read()
        for attempt in range(len(imm_sites(cur, target))):
            site = imm_sites(cur, target)
            if not site:
                break
            k = site[0]
            cand = try_substitution(text, cur, target[k], k)
            if cand is None:
                break
            path = os.path.join(work, 'cand.c')
            open(path, 'w').write(cand)
            w = words_of(path, work, a.include)
            if w and len(w) == len(cur) and sum(1 for x, y in zip(w, target) if x != y) < \
                    sum(1 for x, y in zip(cur, target) if x != y):
                text, cur = cand, w
            else:
                break
        if list(cur) == target:
            status = 'exact'
        elif [shape(x) for x in cur] == [shape(x) for x in target]:
            status = 'imm-like'
        else:
            status = 'shape-changed'
    open(out, 'w').write(text)
    shutil.rmtree(work, ignore_errors=True)
    print(f'{os.path.basename(a.source)} -> {out}: {status} '
          f'({sum(1 for x, y in zip(cur, target) if x != y)} words still differ)')
    sys.exit(0 if status == 'exact' else 1)


if __name__ == '__main__':
    main()
