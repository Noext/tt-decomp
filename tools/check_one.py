#!/usr/bin/env python3
"""Check one source against one target function. Independent of file naming.

    check_one.py SOURCE TARGET_ADDR [--elf PATH] [--splat DIR] [--diff] [--no-resolve]

Prints MATCH (exit 0) when the source's function has exactly the target's instructions, else a
side-by-side diff with `!` on the differing lines (exit 1).

The function's own code is compared with the object's relocations applied, the way the linker
would: a `jal func_002E4708` and an instruction that already holds the same address are the same
thing. Without that, every call would look like a difference.

The source's function is the first function it defines, in definition order. The target's length
comes from the splat output (build/splat/asm/nonmatchings, made from your own copy of the game).
"""
import argparse
import os
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pair_sources import load_words, splat_functions, object_functions, COMPILER, shape  # noqa: E402
from rename_pass import object_text_relocs, resolve  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_ELF = os.path.join(ROOT, 'orig/SCES-53372/CORE.TT.elf')
DEFAULT_SPLAT = os.path.join(ROOT, 'build/splat/asm/nonmatchings')


def compile_words(src, include, outdir=None):
    work = outdir or tempfile.mkdtemp(prefix='check-')
    obj = os.path.join(work, 'a.o')
    r = subprocess.run([f'{COMPILER}/bin/ee-gcc', '-c', '-B', f'{COMPILER}/bin/ee-', '-O2', '-G0',
                        '-fno-exceptions', '-fno-strict-aliasing', f'-I{include}', src, '-o', obj],
                       capture_output=True, text=True)
    if r.returncode != 0:
        return None, None, f'compile error: {(r.stderr.strip().splitlines() or ["?"])[-1]}'
    try:
        fns = object_functions(obj)
        words, relocs, _ = object_text_relocs(obj, ('.rodata', '.data'))
    finally:
        try:
            os.remove(obj)
        except OSError:
            pass
    if not fns:
        return None, None, 'the source defines no function'
    return fns[0][1], relocs, None


def disas(w):
    try:
        import rabbitizer
        return [rabbitizer.Instruction(x, 0).disassemble() for x in w]
    except Exception:
        return [f'{x:08x}' for x in w]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('source')
    ap.add_argument('addr')
    ap.add_argument('--elf', default=DEFAULT_ELF)
    ap.add_argument('--splat', default=DEFAULT_SPLAT)
    ap.add_argument('--include', default=os.path.join(ROOT, 'include'))
    ap.add_argument('--diff', action='store_true')
    ap.add_argument('--no-resolve', action='store_true')
    a = ap.parse_args()

    base, words = load_words(a.elf)
    funcs = splat_functions(a.splat)
    addr = int(a.addr, 16)
    n = funcs.get(addr)
    if n is None:
        sys.exit(f'{a.addr} is not a splat function start')
    i = (addr - base) // 4
    target = list(words[i:i + n])

    mine, relocs, err = compile_words(a.source, a.include)
    if mine is None:
        print(err)
        sys.exit(2)
    # The project's rule: a function counts as decompiled only when its source is C/C++, not
    # assembly posing as C (tools/asm_policy.py). A source that writes the original instructions
    # would otherwise "match" without saying anything about the code.
    try:
        import asm_policy
        bad = asm_policy.violations(open(a.source).read())
    except Exception:
        bad = []
    if bad:
        print('REFUSED by the asm policy: ' + '; '.join(bad))
        sys.exit(3)
    if relocs and not a.no_resolve:
        mine = resolve(mine, relocs)

    if list(mine) == target:
        print(f'MATCH ({len(target)} instructions)')
        sys.exit(0)

    diff = sum(1 for j in range(min(len(mine), len(target))) if mine[j] != target[j])
    diff += abs(len(mine) - len(target))
    same_shape = len(mine) == len(target) and [shape(x) for x in mine] == [shape(x) for x in target]
    kind = 'immediate-only' if same_shape else 'structure differs'
    print(f'DIFF ({diff} of {max(len(mine), len(target))} instructions, {kind}; '
          f'mine {len(mine)}, target {len(target)})')
    dm, dt = disas(mine), disas(target)
    for j in range(max(len(dm), len(dt))):
        m = dm[j] if j < len(dm) else '(none)'
        t = dt[j] if j < len(dt) else '(none)'
        mark = ' ' if m == t else '!'
        if a.diff or m != t:
            print(f'{mark} {j:>3}  target {t:<40} | mine {m}')
    sys.exit(1)


if __name__ == '__main__':
    main()
