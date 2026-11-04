#!/usr/bin/env python3
"""Merge the per-batch reports in config/reports/*.md into one table plus totals.

usage: report_merge.py
Reads the markdown tables the batch agents wrote (columns: source | target_addr | words | result | note).
Missing or malformed reports are reported, not fatal.
"""
import collections, glob, os, pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
ROW = re.compile(r'^\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*([^|]*?)\s*\|\s*$')

def main():
    total = collections.Counter()
    rows = []
    for p in sorted(glob.glob(str(ROOT / 'config/reports/*.md'))):
        n = 0
        for line in open(p):
            m = ROW.match(line.rstrip('\n'))
            if not m or m.group(1).lower() in ('source', '---') or set(m.group(1)) <= set('-: '):
                continue
            result = m.group(4).strip().lower()
            if result not in ('match', 'cannot-match', 'skipped'):
                continue
            rows.append((os.path.basename(p),) + tuple(g.strip() for g in m.groups()))
            total[result] += 1
            n += 1
        print(f'{os.path.basename(p):<24} {n:>4} rows')
    with open(ROOT / 'config/eu_match_report.tsv', 'w') as f:
        f.write('# report\tsource\tfunction\twords\tresult\tnote\n')
        for r in rows:
            f.write('\t'.join(r) + '\n')
    print('\n' + ', '.join(f'{k}: {v}' for k, v in total.most_common()) or 'no rows')
    print(f'\nmerged {len(rows)} rows -> config/eu_match_report.tsv')

if __name__ == '__main__':
    main()
