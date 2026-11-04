#!/usr/bin/env python3
"""Find the SDK's syscall wrappers in the game's own code and name them.

A syscall wrapper is `addiu $v1, $zero, N` followed by `syscall 0`. The HLE replaces each one with
a host function: the game calls the wrapper's address, the wrapper's C definition in port/hle/ is
what runs. The names come from the PS2 SDK's public API for the syscall numbers that are known;
the rest are `SyscallNN`.

usage: syscalls.py [--elf PATH] [--splat DIR] [-o port/hle/syscalls_eu.inc]
"""
import argparse
import collections
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pair_sources import load_words, splat_functions  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SDK = {
    1: 'CreateThread', 2: 'DeleteThread', 3: 'StartThread', 4: 'ExitThread', 5: 'ExitDeleteThread',
    6: 'TerminateThread', 7: 'iDisableDispatchThread', 8: 'iEnableDispatchThread',
    10: 'ChangeThreadPriority', 11: 'iChangeThreadPriority', 12: 'RotateThreadReadyQueue',
    13: 'iRotateThreadReadyQueue', 14: 'ReleaseThreadWait', 15: 'ReleaseWaitThread',
    16: 'iSuspendThread', 17: 'ResumeThread', 18: 'SuspendThread', 19: 'GetThreadId',
    20: 'ReferThreadStatus', 21: 'iReferThreadStatus', 22: 'SleepThread', 23: 'WakeupThread',
    24: 'iWakeupThread', 25: 'CancelWakeupThread', 26: 'iCancelWakeupThread', 27: 'SetThreadParam',
    28: 'iSetThreadParam', 29: 'SetThreadPriority', 30: 'iSetThreadPriority',
    31: 'GetThreadPriority', 32: 'iGetThreadPriority', 33: 'SetupThread', 34: 'SetupHeap',
    35: 'EndThread', 36: 'CreateSema', 37: 'DeleteSema', 38: 'SignalSema', 39: 'iSignalSema',
    40: 'WaitSema', 41: 'PollSema', 42: 'iPollSema', 43: 'ReferSemaStatus', 44: 'iReferSemaStatus',
    45: 'iDeleteSema', 46: 'SetOsdConfigParam', 47: 'GetOsdConfigParam', 48: 'GetMemorySize',
    49: 'GsGetIMR', 50: 'GsPutIMR', 51: 'SetVSyncFlag', 52: 'SetGsCrt',
    55: 'SetOsdConfigParam2', 56: 'GetOsdConfigParam2', 57: 'ConnectMcaDB', 58: 'GetMcaDBParam',
    62: 'iFlushCache', 63: 'iFlushInstructionCache', 64: 'iFlushDataCache', 65: 'FlushCache',
    66: 'FlushInstructionCache', 67: 'FlushDataCache', 68: 'EnableCache', 69: 'DisableCache',
    70: 'PutTLBEntry', 71: 'iPutTLBEntry', 72: 'GetTLBEntry', 73: 'iGetTLBEntry',
    74: 'ProbeTLBEntry', 75: 'iProbeTLBEntry', 76: 'ExpandScratchPad', 77: 'SetMemoryMode',
    78: 'GetMemoryMode', 79: 'SetCacheMode', 80: 'GetCacheMode', 81: 'InitTLB',
    82: 'iSyncCache', 83: 'SyncCache', 84: 'iFlushCacheAll', 85: 'SpeakerInit',
    86: 'GetKeyboardStatus', 87: 'SetLED', 88: 'GetLED', 89: 'beep', 90: 'SetAlarm',
    91: 'iSetAlarm', 92: 'CancelAlarm', 93: 'iCancelAlarm', 94: 'ReleaseAlarm',
    95: 'iReleaseAlarm', 96: 'GetAlarmRemain',
}


def wrappers(elf, splat):
    base, words = load_words(elf)
    found = collections.defaultdict(list)
    for addr, n in splat_functions(splat).items():
        i = (addr - base) // 4
        w = words[i:i + min(n, 4)]
        if len(w) >= 2 and (w[0] >> 26) == 9 and ((w[0] >> 21) & 31) == 0 \
                and ((w[0] >> 16) & 31) == 3 and w[1] == 0x0000000C:
            found[w[0] & 0xFFFF].append(addr)
    return found


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--elf', default=os.path.join(ROOT, 'orig/SCES-53372/CORE.TT.elf'))
    ap.add_argument('--splat', default=os.path.join(ROOT, 'build/splat/asm/nonmatchings'))
    ap.add_argument('-o', '--out', default=os.path.join(ROOT, 'port/hle/syscalls_eu.inc'))
    a = ap.parse_args()
    found = wrappers(a.elf, a.splat)
    total = sum(len(v) for v in found.values())
    named = sum(1 for k in found if k in SDK)
    with open(a.out, 'w') as f:
        f.write('/* SYSCALL WRAPPERS, made by tools/syscalls.py from the game\'s own binary.\n'
                '   Each is a function whose body is `addiu $v1, N; syscall 0`. The HLE defines\n'
                '   that address with the readable name, so the game calls the host implementation. */\n')
        for num, addrs in sorted(found.items()):
            nm = SDK.get(num, f'Syscall{num:02X}')
            for addr in addrs:
                f.write(f'{nm}__SYSCALL = 0x{addr:08X}; // type:func   syscall {num}\n')
    print(f'{total} wrappers, {len(found)} distinct numbers, {named} named -> {a.out}')


if __name__ == '__main__':
    main()
