#!/usr/bin/env python3
"""Re-run the graded classification after the batch reports land, and summarise progress.

usage: aggregate.py [--jobs N]
Writes config/eu_fit.tsv again and prints the tier totals against the starting point.
"""
import argparse, collections, subprocess, sys, pathlib
ROOT = pathlib.Path(__file__).resolve().parents[1]
BASE = dict(exact=8478, addr=7696, immed=5514, close=25, partial=44, far=745)

def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--jobs', default='28'); a = ap.parse_args()
    subprocess.run([sys.executable, 'tools/src_fit.py', 'orig/SCES-53372/CORE.TT.elf',
                    '--splat', 'build/splat/asm/nonmatchings', '--src', 'src',
                    '--out', 'config/eu_fit.tsv', '--jobs', a.jobs, '--include', 'include'],
                   cwd=ROOT, check=True)
    rows = [l.rstrip('\n').split('\t') for l in open(ROOT/'config/eu_fit.tsv')][1:]
    now = collections.Counter(r[3] for r in rows)
    print('\ntier            start     now    delta')
    for k in ('exact','addr','immed','close','partial','far'):
        print(f'{k:<12} {BASE.get(k,0):>7,} {now.get(k,0):>7,} {now.get(k,0)-BASE.get(k,0):>+8,}')
    good = now['exact']+now['addr']
    print(f'\ncarried over without edits: {good:,}/{len(rows):,} = {100*good/len(rows):.1f}%')

if __name__ == '__main__':
    main()
