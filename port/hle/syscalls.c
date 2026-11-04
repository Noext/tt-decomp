/* The EU build's syscall wrappers (tools/syscalls.py). Each C definition is bound to the
   wrapper's address, so the game calls the host implementation instead of raising syscall 0.
   A wrapper without an implementation logs and returns 0: the PS2's answer for a call this
   port does not serve yet. */
#include "hle.h"
#include <stdio.h>

int hle_verbose;

int hle_unknown_syscall(int number)
{
    if (hle_verbose)
        fprintf(stderr, "syscall %d: not implemented\n", number);
    return 0;
}

int eu_syscall_004FDB40(void) RETAIL(eu_syscall_004FDB40);
int eu_syscall_004FDB40(void) { return hle_unknown_syscall(0); }  /* Syscall00 */
int eu_syscall_004FDB50(void) RETAIL(eu_syscall_004FDB50);
int eu_syscall_004FDB50(void) { return hle_unknown_syscall(1); }  /* CreateThread */
int eu_syscall_004FDB60(void) RETAIL(eu_syscall_004FDB60);
int eu_syscall_004FDB60(void) { return hle_unknown_syscall(2); }  /* DeleteThread */
int eu_syscall_004FDB70(void) RETAIL(eu_syscall_004FDB70);
int eu_syscall_004FDB70(void) { return hle_unknown_syscall(3); }  /* StartThread */
int eu_syscall_004FDB80(void) RETAIL(eu_syscall_004FDB80);
int eu_syscall_004FDB80(void) { return hle_unknown_syscall(4); }  /* ExitThread */
int eu_syscall_004FDB90(void) RETAIL(eu_syscall_004FDB90);
int eu_syscall_004FDB90(void) { return hle_unknown_syscall(5); }  /* ExitDeleteThread */
int eu_syscall_004FDBA0(void) RETAIL(eu_syscall_004FDBA0);
int eu_syscall_004FDBA0(void) { return hle_unknown_syscall(6); }  /* TerminateThread */
int eu_syscall_004FDBB0(void) RETAIL(eu_syscall_004FDBB0);
int eu_syscall_004FDBB0(void) { return hle_unknown_syscall(7); }  /* iDisableDispatchThread */
int eu_syscall_004FDBC0(void) RETAIL(eu_syscall_004FDBC0);
int eu_syscall_004FDBC0(void) { return hle_unknown_syscall(8); }  /* iEnableDispatchThread */
int eu_syscall_004FDBD0(void) RETAIL(eu_syscall_004FDBD0);
int eu_syscall_004FDBD0(void) { return hle_unknown_syscall(9); }  /* Syscall09 */
int eu_syscall_004FDBE0(void) RETAIL(eu_syscall_004FDBE0);
int eu_syscall_004FDBE0(void) { return hle_unknown_syscall(10); }  /* ChangeThreadPriority */
int eu_syscall_004FDBF0(void) RETAIL(eu_syscall_004FDBF0);
int eu_syscall_004FDBF0(void) { return hle_unknown_syscall(11); }  /* iChangeThreadPriority */
int eu_syscall_004FDC00(void) RETAIL(eu_syscall_004FDC00);
int eu_syscall_004FDC00(void) { return hle_unknown_syscall(12); }  /* RotateThreadReadyQueue */
int eu_syscall_004FDC10(void) RETAIL(eu_syscall_004FDC10);
int eu_syscall_004FDC10(void) { return hle_unknown_syscall(13); }  /* iRotateThreadReadyQueue */
int eu_syscall_004FDC20(void) RETAIL(eu_syscall_004FDC20);
int eu_syscall_004FDC20(void) { return hle_unknown_syscall(14); }  /* ReleaseThreadWait */
int eu_syscall_004FDC30(void) RETAIL(eu_syscall_004FDC30);
int eu_syscall_004FDC30(void) { return hle_unknown_syscall(15); }  /* ReleaseWaitThread */
int eu_syscall_004FDC50(void) RETAIL(eu_syscall_004FDC50);
int eu_syscall_004FDC50(void) { return hle_unknown_syscall(16); }  /* iSuspendThread */
int eu_syscall_004FDC40(void) RETAIL(eu_syscall_004FDC40);
int eu_syscall_004FDC40(void) { return hle_unknown_syscall(16); }  /* iSuspendThread */
int eu_syscall_004FDC60(void) RETAIL(eu_syscall_004FDC60);
int eu_syscall_004FDC60(void) { return hle_unknown_syscall(17); }  /* ResumeThread */
int eu_syscall_004FDC70(void) RETAIL(eu_syscall_004FDC70);
int eu_syscall_004FDC70(void) { return hle_unknown_syscall(18); }  /* SuspendThread */
int eu_syscall_004FDC80(void) RETAIL(eu_syscall_004FDC80);
int eu_syscall_004FDC80(void) { return hle_unknown_syscall(18); }  /* SuspendThread */
int eu_syscall_004FDC90(void) RETAIL(eu_syscall_004FDC90);
int eu_syscall_004FDC90(void) { return hle_unknown_syscall(19); }  /* GetThreadId */
int eu_syscall_004FDCA0(void) RETAIL(eu_syscall_004FDCA0);
int eu_syscall_004FDCA0(void) { return hle_unknown_syscall(20); }  /* ReferThreadStatus */
int eu_syscall_004FDCB0(void) RETAIL(eu_syscall_004FDCB0);
int eu_syscall_004FDCB0(void) { return hle_unknown_syscall(21); }  /* iReferThreadStatus */
int eu_syscall_004FDCC0(void) RETAIL(eu_syscall_004FDCC0);
int eu_syscall_004FDCC0(void) { return hle_unknown_syscall(22); }  /* SleepThread */
int eu_syscall_004FDCD0(void) RETAIL(eu_syscall_004FDCD0);
int eu_syscall_004FDCD0(void) { return hle_unknown_syscall(23); }  /* WakeupThread */
int eu_syscall_004FDD60(void) RETAIL(eu_syscall_004FDD60);
int eu_syscall_004FDD60(void) { return hle_unknown_syscall(32); }  /* iGetThreadPriority */
int eu_syscall_004FDD70(void) RETAIL(eu_syscall_004FDD70);
int eu_syscall_004FDD70(void) { return hle_unknown_syscall(33); }  /* SetupThread */
int eu_syscall_004FDD80(void) RETAIL(eu_syscall_004FDD80);
int eu_syscall_004FDD80(void) { return hle_unknown_syscall(34); }  /* SetupHeap */
int eu_syscall_00100228(void) RETAIL(eu_syscall_00100228);
int eu_syscall_00100228(void) { return hle_unknown_syscall(35); }  /* EndThread */
int eu_syscall_004FDD90(void) RETAIL(eu_syscall_004FDD90);
int eu_syscall_004FDD90(void) { return hle_unknown_syscall(35); }  /* EndThread */
int eu_syscall_004FDDA0(void) RETAIL(eu_syscall_004FDDA0);
int eu_syscall_004FDDA0(void) { return hle_unknown_syscall(36); }  /* CreateSema */
int eu_syscall_004FDDB0(void) RETAIL(eu_syscall_004FDDB0);
int eu_syscall_004FDDB0(void) { return hle_unknown_syscall(37); }  /* DeleteSema */
int eu_syscall_004FDDD0(void) RETAIL(eu_syscall_004FDDD0);
int eu_syscall_004FDDD0(void) { return hle_unknown_syscall(39); }  /* iSignalSema */
int eu_syscall_004FDDE0(void) RETAIL(eu_syscall_004FDDE0);
int eu_syscall_004FDDE0(void) { return hle_unknown_syscall(40); }  /* WaitSema */
int eu_syscall_004FDDF0(void) RETAIL(eu_syscall_004FDDF0);
int eu_syscall_004FDDF0(void) { return hle_unknown_syscall(41); }  /* PollSema */
int eu_syscall_004FDE10(void) RETAIL(eu_syscall_004FDE10);
int eu_syscall_004FDE10(void) { return hle_unknown_syscall(43); }  /* ReferSemaStatus */
int eu_syscall_004FDE30(void) RETAIL(eu_syscall_004FDE30);
int eu_syscall_004FDE30(void) { return hle_unknown_syscall(45); }  /* iDeleteSema */
int eu_syscall_004FDE50(void) RETAIL(eu_syscall_004FDE50);
int eu_syscall_004FDE50(void) { return hle_unknown_syscall(47); }  /* GetOsdConfigParam */
int eu_syscall_004FDE60(void) RETAIL(eu_syscall_004FDE60);
int eu_syscall_004FDE60(void) { return hle_unknown_syscall(48); }  /* GetMemorySize */
int eu_syscall_004FDE80(void) RETAIL(eu_syscall_004FDE80);
int eu_syscall_004FDE80(void) { return hle_unknown_syscall(50); }  /* GsPutIMR */
int eu_syscall_004FDE90(void) RETAIL(eu_syscall_004FDE90);
int eu_syscall_004FDE90(void) { return hle_unknown_syscall(51); }  /* SetVSyncFlag */
int eu_syscall_004FDEB0(void) RETAIL(eu_syscall_004FDEB0);
int eu_syscall_004FDEB0(void) { return hle_unknown_syscall(53); }  /* Syscall35 */
int eu_syscall_004FDED0(void) RETAIL(eu_syscall_004FDED0);
int eu_syscall_004FDED0(void) { return hle_unknown_syscall(55); }  /* SetOsdConfigParam2 */
int eu_syscall_004FDEF0(void) RETAIL(eu_syscall_004FDEF0);
int eu_syscall_004FDEF0(void) { return hle_unknown_syscall(57); }  /* ConnectMcaDB */
int eu_syscall_004FDF10(void) RETAIL(eu_syscall_004FDF10);
int eu_syscall_004FDF10(void) { return hle_unknown_syscall(59); }  /* Syscall3B */
int eu_syscall_004FDF20(void) RETAIL(eu_syscall_004FDF20);
int eu_syscall_004FDF20(void) { return hle_unknown_syscall(60); }  /* Syscall3C */
int eu_syscall_004FDF30(void) RETAIL(eu_syscall_004FDF30);
int eu_syscall_004FDF30(void) { return hle_unknown_syscall(61); }  /* Syscall3D */
int eu_syscall_004FDF40(void) RETAIL(eu_syscall_004FDF40);
int eu_syscall_004FDF40(void) { return hle_unknown_syscall(62); }  /* iFlushCache */
int eu_syscall_004FDF50(void) RETAIL(eu_syscall_004FDF50);
int eu_syscall_004FDF50(void) { return hle_unknown_syscall(63); }  /* iFlushInstructionCache */
int eu_syscall_004FDF60(void) RETAIL(eu_syscall_004FDF60);
int eu_syscall_004FDF60(void) { return hle_unknown_syscall(64); }  /* iFlushDataCache */
int eu_syscall_004FDF70(void) RETAIL(eu_syscall_004FDF70);
int eu_syscall_004FDF70(void) { return hle_unknown_syscall(65); }  /* FlushCache */
int eu_syscall_004FDF80(void) RETAIL(eu_syscall_004FDF80);
int eu_syscall_004FDF80(void) { return hle_unknown_syscall(66); }  /* FlushInstructionCache */
int eu_syscall_004FDFA0(void) RETAIL(eu_syscall_004FDFA0);
int eu_syscall_004FDFA0(void) { return hle_unknown_syscall(68); }  /* EnableCache */
int eu_syscall_004FDFB0(void) RETAIL(eu_syscall_004FDFB0);
int eu_syscall_004FDFB0(void) { return hle_unknown_syscall(69); }  /* DisableCache */
int eu_syscall_004FDFD0(void) RETAIL(eu_syscall_004FDFD0);
int eu_syscall_004FDFD0(void) { return hle_unknown_syscall(71); }  /* iPutTLBEntry */
int eu_syscall_004FDFF0(void) RETAIL(eu_syscall_004FDFF0);
int eu_syscall_004FDFF0(void) { return hle_unknown_syscall(73); }  /* iGetTLBEntry */
int eu_syscall_004FE000(void) RETAIL(eu_syscall_004FE000);
int eu_syscall_004FE000(void) { return hle_unknown_syscall(74); }  /* ProbeTLBEntry */
int eu_syscall_004FE010(void) RETAIL(eu_syscall_004FE010);
int eu_syscall_004FE010(void) { return hle_unknown_syscall(75); }  /* iProbeTLBEntry */
int eu_syscall_004FE020(void) RETAIL(eu_syscall_004FE020);
int eu_syscall_004FE020(void) { return hle_unknown_syscall(76); }  /* ExpandScratchPad */
int eu_syscall_004FE030(void) RETAIL(eu_syscall_004FE030);
int eu_syscall_004FE030(void) { return hle_unknown_syscall(77); }  /* SetMemoryMode */
int eu_syscall_004FE040(void) RETAIL(eu_syscall_004FE040);
int eu_syscall_004FE040(void) { return hle_unknown_syscall(78); }  /* GetMemoryMode */
int eu_syscall_004FE050(void) RETAIL(eu_syscall_004FE050);
int eu_syscall_004FE050(void) { return hle_unknown_syscall(79); }  /* SetCacheMode */
int eu_syscall_004FE060(void) RETAIL(eu_syscall_004FE060);
int eu_syscall_004FE060(void) { return hle_unknown_syscall(80); }  /* GetCacheMode */
int eu_syscall_004FE070(void) RETAIL(eu_syscall_004FE070);
int eu_syscall_004FE070(void) { return hle_unknown_syscall(81); }  /* InitTLB */
int eu_syscall_004FE080(void) RETAIL(eu_syscall_004FE080);
int eu_syscall_004FE080(void) { return hle_unknown_syscall(82); }  /* iSyncCache */
int eu_syscall_004FE0A0(void) RETAIL(eu_syscall_004FE0A0);
int eu_syscall_004FE0A0(void) { return hle_unknown_syscall(84); }  /* iFlushCacheAll */
int eu_syscall_00509CB0(void) RETAIL(eu_syscall_00509CB0);
int eu_syscall_00509CB0(void) { return hle_unknown_syscall(85); }  /* SpeakerInit */
int eu_syscall_004FE0C0(void) RETAIL(eu_syscall_004FE0C0);
int eu_syscall_004FE0C0(void) { return hle_unknown_syscall(86); }  /* GetKeyboardStatus */
int eu_syscall_00509CD0(void) RETAIL(eu_syscall_00509CD0);
int eu_syscall_00509CD0(void) { return hle_unknown_syscall(86); }  /* GetKeyboardStatus */
int eu_syscall_00509D20(void) RETAIL(eu_syscall_00509D20);
int eu_syscall_00509D20(void) { return hle_unknown_syscall(87); }  /* SetLED */
int eu_syscall_004FE0D0(void) RETAIL(eu_syscall_004FE0D0);
int eu_syscall_004FE0D0(void) { return hle_unknown_syscall(87); }  /* SetLED */
int eu_syscall_00509D40(void) RETAIL(eu_syscall_00509D40);
int eu_syscall_00509D40(void) { return hle_unknown_syscall(88); }  /* GetLED */
int eu_syscall_00509D60(void) RETAIL(eu_syscall_00509D60);
int eu_syscall_00509D60(void) { return hle_unknown_syscall(89); }  /* beep */
int eu_syscall_004FE0F0(void) RETAIL(eu_syscall_004FE0F0);
int eu_syscall_004FE0F0(void) { return hle_unknown_syscall(89); }  /* beep */
int eu_syscall_00509B80(void) RETAIL(eu_syscall_00509B80);
int eu_syscall_00509B80(void) { return hle_unknown_syscall(90); }  /* SetAlarm */
int eu_syscall_005074A8(void) RETAIL(eu_syscall_005074A8);
int eu_syscall_005074A8(void) { return hle_unknown_syscall(90); }  /* SetAlarm */
int eu_syscall_00507338(void) RETAIL(eu_syscall_00507338);
int eu_syscall_00507338(void) { return hle_unknown_syscall(90); }  /* SetAlarm */
int eu_syscall_00507758(void) RETAIL(eu_syscall_00507758);
int eu_syscall_00507758(void) { return hle_unknown_syscall(90); }  /* SetAlarm */
int eu_syscall_005070F0(void) RETAIL(eu_syscall_005070F0);
int eu_syscall_005070F0(void) { return hle_unknown_syscall(90); }  /* SetAlarm */
int eu_syscall_004FE110(void) RETAIL(eu_syscall_004FE110);
int eu_syscall_004FE110(void) { return hle_unknown_syscall(91); }  /* iSetAlarm */
int eu_syscall_005077A0(void) RETAIL(eu_syscall_005077A0);
int eu_syscall_005077A0(void) { return hle_unknown_syscall(91); }  /* iSetAlarm */
int eu_syscall_00509BC8(void) RETAIL(eu_syscall_00509BC8);
int eu_syscall_00509BC8(void) { return hle_unknown_syscall(91); }  /* iSetAlarm */
int eu_syscall_00507380(void) RETAIL(eu_syscall_00507380);
int eu_syscall_00507380(void) { return hle_unknown_syscall(91); }  /* iSetAlarm */
int eu_syscall_004FE120(void) RETAIL(eu_syscall_004FE120);
int eu_syscall_004FE120(void) { return hle_unknown_syscall(92); }  /* CancelAlarm */
int eu_syscall_004FE140(void) RETAIL(eu_syscall_004FE140);
int eu_syscall_004FE140(void) { return hle_unknown_syscall(93); }  /* iCancelAlarm */
int eu_syscall_004FE160(void) RETAIL(eu_syscall_004FE160);
int eu_syscall_004FE160(void) { return hle_unknown_syscall(94); }  /* ReleaseAlarm */
int eu_syscall_004FE180(void) RETAIL(eu_syscall_004FE180);
int eu_syscall_004FE180(void) { return hle_unknown_syscall(95); }  /* iReleaseAlarm */
int eu_syscall_004FE1A0(void) RETAIL(eu_syscall_004FE1A0);
int eu_syscall_004FE1A0(void) { return hle_unknown_syscall(96); }  /* GetAlarmRemain */
int eu_syscall_004FE1B0(void) RETAIL(eu_syscall_004FE1B0);
int eu_syscall_004FE1B0(void) { return hle_unknown_syscall(97); }  /* Syscall61 */
int eu_syscall_004FE1C0(void) RETAIL(eu_syscall_004FE1C0);
int eu_syscall_004FE1C0(void) { return hle_unknown_syscall(98); }  /* Syscall62 */
int eu_syscall_004FE1D0(void) RETAIL(eu_syscall_004FE1D0);
int eu_syscall_004FE1D0(void) { return hle_unknown_syscall(99); }  /* Syscall63 */
int eu_syscall_004FE1E0(void) RETAIL(eu_syscall_004FE1E0);
int eu_syscall_004FE1E0(void) { return hle_unknown_syscall(100); }  /* Syscall64 */
int eu_syscall_004FE1F0(void) RETAIL(eu_syscall_004FE1F0);
int eu_syscall_004FE1F0(void) { return hle_unknown_syscall(102); }  /* Syscall66 */
int eu_syscall_004FE230(void) RETAIL(eu_syscall_004FE230);
int eu_syscall_004FE230(void) { return hle_unknown_syscall(107); }  /* Syscall6B */
int eu_syscall_004FE240(void) RETAIL(eu_syscall_004FE240);
int eu_syscall_004FE240(void) { return hle_unknown_syscall(108); }  /* Syscall6C */
int eu_syscall_004FE250(void) RETAIL(eu_syscall_004FE250);
int eu_syscall_004FE250(void) { return hle_unknown_syscall(109); }  /* Syscall6D */
int eu_syscall_004FE260(void) RETAIL(eu_syscall_004FE260);
int eu_syscall_004FE260(void) { return hle_unknown_syscall(110); }  /* Syscall6E */
int eu_syscall_004FE270(void) RETAIL(eu_syscall_004FE270);
int eu_syscall_004FE270(void) { return hle_unknown_syscall(111); }  /* Syscall6F */
int eu_syscall_004FE280(void) RETAIL(eu_syscall_004FE280);
int eu_syscall_004FE280(void) { return hle_unknown_syscall(112); }  /* Syscall70 */
int eu_syscall_004FE2A0(void) RETAIL(eu_syscall_004FE2A0);
int eu_syscall_004FE2A0(void) { return hle_unknown_syscall(113); }  /* Syscall71 */
int eu_syscall_004FE2C0(void) RETAIL(eu_syscall_004FE2C0);
int eu_syscall_004FE2C0(void) { return hle_unknown_syscall(114); }  /* Syscall72 */
int eu_syscall_004FE2D0(void) RETAIL(eu_syscall_004FE2D0);
int eu_syscall_004FE2D0(void) { return hle_unknown_syscall(115); }  /* Syscall73 */
int eu_syscall_005072C0(void) RETAIL(eu_syscall_005072C0);
int eu_syscall_005072C0(void) { return hle_unknown_syscall(116); }  /* Syscall74 */
int eu_syscall_005074E8(void) RETAIL(eu_syscall_005074E8);
int eu_syscall_005074E8(void) { return hle_unknown_syscall(116); }  /* Syscall74 */
int eu_syscall_00507328(void) RETAIL(eu_syscall_00507328);
int eu_syscall_00507328(void) { return hle_unknown_syscall(116); }  /* Syscall74 */
int eu_syscall_004FE2E0(void) RETAIL(eu_syscall_004FE2E0);
int eu_syscall_004FE2E0(void) { return hle_unknown_syscall(116); }  /* Syscall74 */
int eu_syscall_00509BD8(void) RETAIL(eu_syscall_00509BD8);
int eu_syscall_00509BD8(void) { return hle_unknown_syscall(116); }  /* Syscall74 */
int eu_syscall_00507748(void) RETAIL(eu_syscall_00507748);
int eu_syscall_00507748(void) { return hle_unknown_syscall(116); }  /* Syscall74 */
int eu_syscall_004FE2F0(void) RETAIL(eu_syscall_004FE2F0);
int eu_syscall_004FE2F0(void) { return hle_unknown_syscall(117); }  /* Syscall75 */
int eu_syscall_004FE300(void) RETAIL(eu_syscall_004FE300);
int eu_syscall_004FE300(void) { return hle_unknown_syscall(118); }  /* Syscall76 */
int eu_syscall_004FE320(void) RETAIL(eu_syscall_004FE320);
int eu_syscall_004FE320(void) { return hle_unknown_syscall(119); }  /* Syscall77 */
int eu_syscall_004FE340(void) RETAIL(eu_syscall_004FE340);
int eu_syscall_004FE340(void) { return hle_unknown_syscall(120); }  /* Syscall78 */
int eu_syscall_004FE360(void) RETAIL(eu_syscall_004FE360);
int eu_syscall_004FE360(void) { return hle_unknown_syscall(121); }  /* Syscall79 */
int eu_syscall_004FE370(void) RETAIL(eu_syscall_004FE370);
int eu_syscall_004FE370(void) { return hle_unknown_syscall(122); }  /* Syscall7A */
int eu_syscall_004FE380(void) RETAIL(eu_syscall_004FE380);
int eu_syscall_004FE380(void) { return hle_unknown_syscall(123); }  /* Syscall7B */
int eu_syscall_004FE390(void) RETAIL(eu_syscall_004FE390);
int eu_syscall_004FE390(void) { return hle_unknown_syscall(124); }  /* Syscall7C */
int eu_syscall_004FE3A0(void) RETAIL(eu_syscall_004FE3A0);
int eu_syscall_004FE3A0(void) { return hle_unknown_syscall(125); }  /* Syscall7D */
int eu_syscall_004FE3B0(void) RETAIL(eu_syscall_004FE3B0);
int eu_syscall_004FE3B0(void) { return hle_unknown_syscall(126); }  /* Syscall7E */
int eu_syscall_004FE3C0(void) RETAIL(eu_syscall_004FE3C0);
int eu_syscall_004FE3C0(void) { return hle_unknown_syscall(127); }  /* Syscall7F */
int eu_syscall_004FE3D0(void) RETAIL(eu_syscall_004FE3D0);
int eu_syscall_004FE3D0(void) { return hle_unknown_syscall(130); }  /* Syscall82 */
int eu_syscall_00507178(void) RETAIL(eu_syscall_00507178);
int eu_syscall_00507178(void) { return hle_unknown_syscall(131); }  /* Syscall83 */
int eu_syscall_004FDCE0(void) RETAIL(eu_syscall_004FDCE0);
int eu_syscall_004FDCE0(void) { return hle_unknown_syscall(252); }  /* SyscallFC */
int eu_syscall_004FDCF0(void) RETAIL(eu_syscall_004FDCF0);
int eu_syscall_004FDCF0(void) { return hle_unknown_syscall(253); }  /* SyscallFD */
int eu_syscall_004FDD50(void) RETAIL(eu_syscall_004FDD50);
int eu_syscall_004FDD50(void) { return hle_unknown_syscall(65281); }  /* SyscallFF01 */
int eu_syscall_004FDD40(void) RETAIL(eu_syscall_004FDD40);
int eu_syscall_004FDD40(void) { return hle_unknown_syscall(65282); }  /* SyscallFF02 */
int eu_syscall_004FE350(void) RETAIL(eu_syscall_004FE350);
int eu_syscall_004FE350(void) { return hle_unknown_syscall(65416); }  /* SyscallFF88 */
int eu_syscall_004FE330(void) RETAIL(eu_syscall_004FE330);
int eu_syscall_004FE330(void) { return hle_unknown_syscall(65417); }  /* SyscallFF89 */
int eu_syscall_004FE310(void) RETAIL(eu_syscall_004FE310);
int eu_syscall_004FE310(void) { return hle_unknown_syscall(65418); }  /* SyscallFF8A */
int eu_syscall_004FE2B0(void) RETAIL(eu_syscall_004FE2B0);
int eu_syscall_004FE2B0(void) { return hle_unknown_syscall(65423); }  /* SyscallFF8F */
int eu_syscall_004FE290(void) RETAIL(eu_syscall_004FE290);
int eu_syscall_004FE290(void) { return hle_unknown_syscall(65424); }  /* SyscallFF90 */
int eu_syscall_004FE220(void) RETAIL(eu_syscall_004FE220);
int eu_syscall_004FE220(void) { return hle_unknown_syscall(65430); }  /* SyscallFF96 */
int eu_syscall_004FE210(void) RETAIL(eu_syscall_004FE210);
int eu_syscall_004FE210(void) { return hle_unknown_syscall(65432); }  /* SyscallFF98 */
int eu_syscall_004FE200(void) RETAIL(eu_syscall_004FE200);
int eu_syscall_004FE200(void) { return hle_unknown_syscall(65433); }  /* SyscallFF99 */
int eu_syscall_004FE190(void) RETAIL(eu_syscall_004FE190);
int eu_syscall_004FE190(void) { return hle_unknown_syscall(65441); }  /* SyscallFFA1 */
int eu_syscall_004FE170(void) RETAIL(eu_syscall_004FE170);
int eu_syscall_004FE170(void) { return hle_unknown_syscall(65442); }  /* SyscallFFA2 */
int eu_syscall_004FE150(void) RETAIL(eu_syscall_004FE150);
int eu_syscall_004FE150(void) { return hle_unknown_syscall(65443); }  /* SyscallFFA3 */
int eu_syscall_004FE130(void) RETAIL(eu_syscall_004FE130);
int eu_syscall_004FE130(void) { return hle_unknown_syscall(65444); }  /* SyscallFFA4 */
int eu_syscall_004FE100(void) RETAIL(eu_syscall_004FE100);
int eu_syscall_004FE100(void) { return hle_unknown_syscall(65446); }  /* SyscallFFA6 */
int eu_syscall_00509D50(void) RETAIL(eu_syscall_00509D50);
int eu_syscall_00509D50(void) { return hle_unknown_syscall(65448); }  /* SyscallFFA8 */
int eu_syscall_004FE0E0(void) RETAIL(eu_syscall_004FE0E0);
int eu_syscall_004FE0E0(void) { return hle_unknown_syscall(65448); }  /* SyscallFFA8 */
int eu_syscall_00509D30(void) RETAIL(eu_syscall_00509D30);
int eu_syscall_00509D30(void) { return hle_unknown_syscall(65449); }  /* SyscallFFA9 */
int eu_syscall_00509D10(void) RETAIL(eu_syscall_00509D10);
int eu_syscall_00509D10(void) { return hle_unknown_syscall(65450); }  /* SyscallFFAA */
int eu_syscall_004FE0B0(void) RETAIL(eu_syscall_004FE0B0);
int eu_syscall_004FE0B0(void) { return hle_unknown_syscall(65451); }  /* SyscallFFAB */
int eu_syscall_00509CC0(void) RETAIL(eu_syscall_00509CC0);
int eu_syscall_00509CC0(void) { return hle_unknown_syscall(65451); }  /* SyscallFFAB */
int eu_syscall_004FE090(void) RETAIL(eu_syscall_004FE090);
int eu_syscall_004FE090(void) { return hle_unknown_syscall(65453); }  /* SyscallFFAD */
int eu_syscall_004FDFE0(void) RETAIL(eu_syscall_004FDFE0);
int eu_syscall_004FDFE0(void) { return hle_unknown_syscall(65464); }  /* SyscallFFB8 */
int eu_syscall_004FDFC0(void) RETAIL(eu_syscall_004FDFC0);
int eu_syscall_004FDFC0(void) { return hle_unknown_syscall(65466); }  /* SyscallFFBA */
int eu_syscall_004FDF90(void) RETAIL(eu_syscall_004FDF90);
int eu_syscall_004FDF90(void) { return hle_unknown_syscall(65469); }  /* SyscallFFBD */
int eu_syscall_004FDF00(void) RETAIL(eu_syscall_004FDF00);
int eu_syscall_004FDF00(void) { return hle_unknown_syscall(65478); }  /* SyscallFFC6 */
int eu_syscall_004FDEE0(void) RETAIL(eu_syscall_004FDEE0);
int eu_syscall_004FDEE0(void) { return hle_unknown_syscall(65480); }  /* SyscallFFC8 */
int eu_syscall_004FDEC0(void) RETAIL(eu_syscall_004FDEC0);
int eu_syscall_004FDEC0(void) { return hle_unknown_syscall(65482); }  /* SyscallFFCA */
int eu_syscall_004FDEA0(void) RETAIL(eu_syscall_004FDEA0);
int eu_syscall_004FDEA0(void) { return hle_unknown_syscall(65484); }  /* SyscallFFCC */
int eu_syscall_004FDE70(void) RETAIL(eu_syscall_004FDE70);
int eu_syscall_004FDE70(void) { return hle_unknown_syscall(65487); }  /* SyscallFFCF */
int eu_syscall_004FDE40(void) RETAIL(eu_syscall_004FDE40);
int eu_syscall_004FDE40(void) { return hle_unknown_syscall(65490); }  /* SyscallFFD2 */
int eu_syscall_004FDE20(void) RETAIL(eu_syscall_004FDE20);
int eu_syscall_004FDE20(void) { return hle_unknown_syscall(65492); }  /* SyscallFFD4 */
int eu_syscall_004FDE00(void) RETAIL(eu_syscall_004FDE00);
int eu_syscall_004FDE00(void) { return hle_unknown_syscall(65494); }  /* SyscallFFD6 */
int eu_syscall_004FDDC0(void) RETAIL(eu_syscall_004FDDC0);
int eu_syscall_004FDDC0(void) { return hle_unknown_syscall(65498); }  /* SyscallFFDA */
int eu_syscall_004FDD30(void) RETAIL(eu_syscall_004FDD30);
int eu_syscall_004FDD30(void) { return hle_unknown_syscall(65507); }  /* SyscallFFE3 */
int eu_syscall_004FDD20(void) RETAIL(eu_syscall_004FDD20);
int eu_syscall_004FDD20(void) { return hle_unknown_syscall(65508); }  /* SyscallFFE4 */
int eu_syscall_004FDD10(void) RETAIL(eu_syscall_004FDD10);
int eu_syscall_004FDD10(void) { return hle_unknown_syscall(65509); }  /* SyscallFFE5 */
int eu_syscall_004FDD00(void) RETAIL(eu_syscall_004FDD00);
int eu_syscall_004FDD00(void) { return hle_unknown_syscall(65510); }  /* SyscallFFE6 */
