#!/usr/bin/env python3
"""Adopt verified sources into this build, under the EU identities.

A source compiled here and proven identical to an EU function (or differing only in its jump and
`lui` targets) describes that EU function. This tool gives it the EU function's name and place:
it writes the source to the path layout.py plans for the EU address, and adds the symbol to
config/symbol_addrs.txt so `symbols.py` resolves the new name.

The US source is left where it is: it is still the name the US project uses, and its path records
where it came from. Nothing is overwritten — a planned path that already exists is reported and
skipped.

usage: adopt.py PAIRS.tsv [--status exact,addr] [--dry-run] [--one ADDR]
"""
import argparse
import collections
import os
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import layout  # noqa: E402
import symbols  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('pairs')
    ap.add_argument('--status', default='exact')
    ap.add_argument('--dry-run', action='store_true')
    ap.add_argument('--one')
    a = ap.parse_args()

    want = set(a.status.split(','))
    rows = []
    for line in open(a.pairs):
        if line.startswith('#'):
            continue
        src, func, n, st, at = line.rstrip('\n').split('\t')
        if st not in want:
            continue
        rows.append((src, func, int(n), st, int(at, 16)))

    stats = collections.Counter()
    adopted = []
    for src, func, n, st, addr in rows:
        if a.one and int(a.one, 16) != addr:
            continue
        ext = os.path.splitext(src)[1]
        rel = layout.path_for(addr, ext)
        dest = os.path.join(ROOT, rel.replace('/', os.sep))
        if os.path.exists(dest):
            stats['already present'] += 1
            continue
        if not a.dry_run:
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            shutil.copyfile(os.path.join(ROOT, src), dest)
        adopted.append((addr, dest))
        stats['adopted'] += 1

    if not a.dry_run and adopted:
        names = os.path.join(ROOT, 'config/symbol_addrs.txt')
        text = open(names).read()
        new = [f'func_{addr:08X} = 0x{addr:08X}; // type:func' for addr, _ in adopted
               if f'func_{addr:08X} =' not in text]
        if new:
            if not text.endswith('\n'):
                text += '\n'
            open(names, 'w').write(text + '\n'.join(new) + '\n')

    print(dict(stats))
    for addr, dest in adopted[:10]:
        print(f'  {addr:#010x}  {os.path.relpath(dest, ROOT)}')
    if len(adopted) > 10:
        print(f'  ... {len(adopted) - 10} more')


if __name__ == '__main__':
    main()
