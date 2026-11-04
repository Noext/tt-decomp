/* The EU build's syscall wrappers (tools/syscalls.py). Each C definition is bound to the
   wrapper's address, so the game calls the host implementation instead of raising syscall 0.
   A wrapper without an implementation logs and returns 0: the PS2's answer for a call this
   port does not serve yet. */
#include "hle.h"
#include <stdio.h>

int hle_verbose;

/* The kernel's dispatch (kernel.c): every wrapper reaches it with its number. */
void hle_syscall(uint32_t number);
uint32_t hle_syscall_result(void);

int hle_unknown_syscall(int number)
{
    if (hle_verbose)
        fprintf(stderr, "syscall %d: not implemented\n", number);
    return 0;
}

int eu_syscall_004FDB40(void) RETAIL(eu_syscall_004FDB40);
int eu_syscall_004FDB40(void) { hle_syscall(0); return (int)hle_syscall_result(); }  /* Syscall00 */
int eu_syscall_004FDB50(void) RETAIL(eu_syscall_004FDB50);
int eu_syscall_004FDB50(void) { hle_syscall(1); return (int)hle_syscall_result(); }  /* CreateThread */
int eu_syscall_004FDB60(void) RETAIL(eu_syscall_004FDB60);
int eu_syscall_004FDB60(void) { hle_syscall(2); return (int)hle_syscall_result(); }  /* DeleteThread */
int eu_syscall_004FDB70(void) RETAIL(eu_syscall_004FDB70);
int eu_syscall_004FDB70(void) { hle_syscall(3); return (int)hle_syscall_result(); }  /* StartThread */
int eu_syscall_004FDB80(void) RETAIL(eu_syscall_004FDB80);
int eu_syscall_004FDB80(void) { hle_syscall(4); return (int)hle_syscall_result(); }  /* ExitThread */
int eu_syscall_004FDB90(void) RETAIL(eu_syscall_004FDB90);
int eu_syscall_004FDB90(void) { hle_syscall(5); return (int)hle_syscall_result(); }  /* ExitDeleteThread */
int eu_syscall_004FDBA0(void) RETAIL(eu_syscall_004FDBA0);
int eu_syscall_004FDBA0(void) { hle_syscall(6); return (int)hle_syscall_result(); }  /* TerminateThread */
int eu_syscall_004FDBB0(void) RETAIL(eu_syscall_004FDBB0);
int eu_syscall_004FDBB0(void) { hle_syscall(7); return (int)hle_syscall_result(); }  /* iDisableDispatchThread */
int eu_syscall_004FDBC0(void) RETAIL(eu_syscall_004FDBC0);
int eu_syscall_004FDBC0(void) { hle_syscall(8); return (int)hle_syscall_result(); }  /* iEnableDispatchThread */
int eu_syscall_004FDBD0(void) RETAIL(eu_syscall_004FDBD0);
int eu_syscall_004FDBD0(void) { hle_syscall(9); return (int)hle_syscall_result(); }  /* Syscall09 */
int eu_syscall_004FDBE0(void) RETAIL(eu_syscall_004FDBE0);
int eu_syscall_004FDBE0(void) { hle_syscall(10); return (int)hle_syscall_result(); }  /* ChangeThreadPriority */
int eu_syscall_004FDBF0(void) RETAIL(eu_syscall_004FDBF0);
int eu_syscall_004FDBF0(void) { hle_syscall(11); return (int)hle_syscall_result(); }  /* iChangeThreadPriority */
int eu_syscall_004FDC00(void) RETAIL(eu_syscall_004FDC00);
int eu_syscall_004FDC00(void) { hle_syscall(12); return (int)hle_syscall_result(); }  /* RotateThreadReadyQueue */
int eu_syscall_004FDC10(void) RETAIL(eu_syscall_004FDC10);
int eu_syscall_004FDC10(void) { hle_syscall(13); return (int)hle_syscall_result(); }  /* iRotateThreadReadyQueue */
int eu_syscall_004FDC20(void) RETAIL(eu_syscall_004FDC20);
int eu_syscall_004FDC20(void) { hle_syscall(14); return (int)hle_syscall_result(); }  /* ReleaseThreadWait */
int eu_syscall_004FDC30(void) RETAIL(eu_syscall_004FDC30);
int eu_syscall_004FDC30(void) { hle_syscall(15); return (int)hle_syscall_result(); }  /* ReleaseWaitThread */
int eu_syscall_004FDC50(void) RETAIL(eu_syscall_004FDC50);
int eu_syscall_004FDC50(void) { hle_syscall(16); return (int)hle_syscall_result(); }  /* iSuspendThread */
int eu_syscall_004FDC40(void) RETAIL(eu_syscall_004FDC40);
int eu_syscall_004FDC40(void) { hle_syscall(16); return (int)hle_syscall_result(); }  /* iSuspendThread */
int eu_syscall_004FDC60(void) RETAIL(eu_syscall_004FDC60);
int eu_syscall_004FDC60(void) { hle_syscall(17); return (int)hle_syscall_result(); }  /* ResumeThread */
int eu_syscall_004FDC70(void) RETAIL(eu_syscall_004FDC70);
int eu_syscall_004FDC70(void) { hle_syscall(18); return (int)hle_syscall_result(); }  /* SuspendThread */
int eu_syscall_004FDC80(void) RETAIL(eu_syscall_004FDC80);
int eu_syscall_004FDC80(void) { hle_syscall(18); return (int)hle_syscall_result(); }  /* SuspendThread */
int eu_syscall_004FDC90(void) RETAIL(eu_syscall_004FDC90);
int eu_syscall_004FDC90(void) { hle_syscall(19); return (int)hle_syscall_result(); }  /* GetThreadId */
int eu_syscall_004FDCA0(void) RETAIL(eu_syscall_004FDCA0);
int eu_syscall_004FDCA0(void) { hle_syscall(20); return (int)hle_syscall_result(); }  /* ReferThreadStatus */
int eu_syscall_004FDCB0(void) RETAIL(eu_syscall_004FDCB0);
int eu_syscall_004FDCB0(void) { hle_syscall(21); return (int)hle_syscall_result(); }  /* iReferThreadStatus */
int eu_syscall_004FDCC0(void) RETAIL(eu_syscall_004FDCC0);
int eu_syscall_004FDCC0(void) { hle_syscall(22); return (int)hle_syscall_result(); }  /* SleepThread */
int eu_syscall_004FDCD0(void) RETAIL(eu_syscall_004FDCD0);
int eu_syscall_004FDCD0(void) { hle_syscall(23); return (int)hle_syscall_result(); }  /* WakeupThread */
int eu_syscall_004FDD60(void) RETAIL(eu_syscall_004FDD60);
int eu_syscall_004FDD60(void) { hle_syscall(32); return (int)hle_syscall_result(); }  /* iGetThreadPriority */
int eu_syscall_004FDD70(void) RETAIL(eu_syscall_004FDD70);
int eu_syscall_004FDD70(void) { hle_syscall(33); return (int)hle_syscall_result(); }  /* SetupThread */
int eu_syscall_004FDD80(void) RETAIL(eu_syscall_004FDD80);
int eu_syscall_004FDD80(void) { hle_syscall(34); return (int)hle_syscall_result(); }  /* SetupHeap */
int eu_syscall_00100228(void) RETAIL(eu_syscall_00100228);
int eu_syscall_00100228(void) { hle_syscall(35); return (int)hle_syscall_result(); }  /* EndThread */
int eu_syscall_004FDD90(void) RETAIL(eu_syscall_004FDD90);
int eu_syscall_004FDD90(void) { hle_syscall(35); return (int)hle_syscall_result(); }  /* EndThread */
int eu_syscall_004FDDA0(void) RETAIL(eu_syscall_004FDDA0);
int eu_syscall_004FDDA0(void) { hle_syscall(36); return (int)hle_syscall_result(); }  /* CreateSema */
int eu_syscall_004FDDB0(void) RETAIL(eu_syscall_004FDDB0);
int eu_syscall_004FDDB0(void) { hle_syscall(37); return (int)hle_syscall_result(); }  /* DeleteSema */
int eu_syscall_004FDDD0(void) RETAIL(eu_syscall_004FDDD0);
int eu_syscall_004FDDD0(void) { hle_syscall(39); return (int)hle_syscall_result(); }  /* iSignalSema */
int eu_syscall_004FDDE0(void) RETAIL(eu_syscall_004FDDE0);
int eu_syscall_004FDDE0(void) { hle_syscall(40); return (int)hle_syscall_result(); }  /* WaitSema */
int eu_syscall_004FDDF0(void) RETAIL(eu_syscall_004FDDF0);
int eu_syscall_004FDDF0(void) { hle_syscall(41); return (int)hle_syscall_result(); }  /* PollSema */
int eu_syscall_004FDE10(void) RETAIL(eu_syscall_004FDE10);
int eu_syscall_004FDE10(void) { hle_syscall(43); return (int)hle_syscall_result(); }  /* ReferSemaStatus */
int eu_syscall_004FDE30(void) RETAIL(eu_syscall_004FDE30);
int eu_syscall_004FDE30(void) { hle_syscall(45); return (int)hle_syscall_result(); }  /* iDeleteSema */
int eu_syscall_004FDE50(void) RETAIL(eu_syscall_004FDE50);
int eu_syscall_004FDE50(void) { hle_syscall(47); return (int)hle_syscall_result(); }  /* GetOsdConfigParam */
int eu_syscall_004FDE60(void) RETAIL(eu_syscall_004FDE60);
int eu_syscall_004FDE60(void) { hle_syscall(48); return (int)hle_syscall_result(); }  /* GetMemorySize */
int eu_syscall_004FDE80(void) RETAIL(eu_syscall_004FDE80);
int eu_syscall_004FDE80(void) { hle_syscall(50); return (int)hle_syscall_result(); }  /* GsPutIMR */
int eu_syscall_004FDE90(void) RETAIL(eu_syscall_004FDE90);
int eu_syscall_004FDE90(void) { hle_syscall(51); return (int)hle_syscall_result(); }  /* SetVSyncFlag */
int eu_syscall_004FDEB0(void) RETAIL(eu_syscall_004FDEB0);
int eu_syscall_004FDEB0(void) { hle_syscall(53); return (int)hle_syscall_result(); }  /* Syscall35 */
int eu_syscall_004FDED0(void) RETAIL(eu_syscall_004FDED0);
int eu_syscall_004FDED0(void) { hle_syscall(55); return (int)hle_syscall_result(); }  /* SetOsdConfigParam2 */
int eu_syscall_004FDEF0(void) RETAIL(eu_syscall_004FDEF0);
int eu_syscall_004FDEF0(void) { hle_syscall(57); return (int)hle_syscall_result(); }  /* ConnectMcaDB */
int eu_syscall_004FDF10(void) RETAIL(eu_syscall_004FDF10);
int eu_syscall_004FDF10(void) { hle_syscall(59); return (int)hle_syscall_result(); }  /* Syscall3B */
int eu_syscall_004FDF20(void) RETAIL(eu_syscall_004FDF20);
int eu_syscall_004FDF20(void) { hle_syscall(60); return (int)hle_syscall_result(); }  /* Syscall3C */
int eu_syscall_004FDF30(void) RETAIL(eu_syscall_004FDF30);
int eu_syscall_004FDF30(void) { hle_syscall(61); return (int)hle_syscall_result(); }  /* Syscall3D */
int eu_syscall_004FDF40(void) RETAIL(eu_syscall_004FDF40);
int eu_syscall_004FDF40(void) { hle_syscall(62); return (int)hle_syscall_result(); }  /* iFlushCache */
int eu_syscall_004FDF50(void) RETAIL(eu_syscall_004FDF50);
int eu_syscall_004FDF50(void) { hle_syscall(63); return (int)hle_syscall_result(); }  /* iFlushInstructionCache */
int eu_syscall_004FDF60(void) RETAIL(eu_syscall_004FDF60);
int eu_syscall_004FDF60(void) { hle_syscall(64); return (int)hle_syscall_result(); }  /* iFlushDataCache */
int eu_syscall_004FDF70(void) RETAIL(eu_syscall_004FDF70);
int eu_syscall_004FDF70(void) { hle_syscall(65); return (int)hle_syscall_result(); }  /* FlushCache */
int eu_syscall_004FDF80(void) RETAIL(eu_syscall_004FDF80);
int eu_syscall_004FDF80(void) { hle_syscall(66); return (int)hle_syscall_result(); }  /* FlushInstructionCache */
int eu_syscall_004FDFA0(void) RETAIL(eu_syscall_004FDFA0);
int eu_syscall_004FDFA0(void) { hle_syscall(68); return (int)hle_syscall_result(); }  /* EnableCache */
int eu_syscall_004FDFB0(void) RETAIL(eu_syscall_004FDFB0);
int eu_syscall_004FDFB0(void) { hle_syscall(69); return (int)hle_syscall_result(); }  /* DisableCache */
int eu_syscall_004FDFD0(void) RETAIL(eu_syscall_004FDFD0);
int eu_syscall_004FDFD0(void) { hle_syscall(71); return (int)hle_syscall_result(); }  /* iPutTLBEntry */
int eu_syscall_004FDFF0(void) RETAIL(eu_syscall_004FDFF0);
int eu_syscall_004FDFF0(void) { hle_syscall(73); return (int)hle_syscall_result(); }  /* iGetTLBEntry */
int eu_syscall_004FE000(void) RETAIL(eu_syscall_004FE000);
int eu_syscall_004FE000(void) { hle_syscall(74); return (int)hle_syscall_result(); }  /* ProbeTLBEntry */
int eu_syscall_004FE010(void) RETAIL(eu_syscall_004FE010);
int eu_syscall_004FE010(void) { hle_syscall(75); return (int)hle_syscall_result(); }  /* iProbeTLBEntry */
int eu_syscall_004FE020(void) RETAIL(eu_syscall_004FE020);
int eu_syscall_004FE020(void) { hle_syscall(76); return (int)hle_syscall_result(); }  /* ExpandScratchPad */
int eu_syscall_004FE030(void) RETAIL(eu_syscall_004FE030);
int eu_syscall_004FE030(void) { hle_syscall(77); return (int)hle_syscall_result(); }  /* SetMemoryMode */
int eu_syscall_004FE040(void) RETAIL(eu_syscall_004FE040);
int eu_syscall_004FE040(void) { hle_syscall(78); return (int)hle_syscall_result(); }  /* GetMemoryMode */
int eu_syscall_004FE050(void) RETAIL(eu_syscall_004FE050);
int eu_syscall_004FE050(void) { hle_syscall(79); return (int)hle_syscall_result(); }  /* SetCacheMode */
int eu_syscall_004FE060(void) RETAIL(eu_syscall_004FE060);
int eu_syscall_004FE060(void) { hle_syscall(80); return (int)hle_syscall_result(); }  /* GetCacheMode */
int eu_syscall_004FE070(void) RETAIL(eu_syscall_004FE070);
int eu_syscall_004FE070(void) { hle_syscall(81); return (int)hle_syscall_result(); }  /* InitTLB */
int eu_syscall_004FE080(void) RETAIL(eu_syscall_004FE080);
int eu_syscall_004FE080(void) { hle_syscall(82); return (int)hle_syscall_result(); }  /* iSyncCache */
int eu_syscall_004FE0A0(void) RETAIL(eu_syscall_004FE0A0);
int eu_syscall_004FE0A0(void) { hle_syscall(84); return (int)hle_syscall_result(); }  /* iFlushCacheAll */
int eu_syscall_00509CB0(void) RETAIL(eu_syscall_00509CB0);
int eu_syscall_00509CB0(void) { hle_syscall(85); return (int)hle_syscall_result(); }  /* SpeakerInit */
int eu_syscall_004FE0C0(void) RETAIL(eu_syscall_004FE0C0);
int eu_syscall_004FE0C0(void) { hle_syscall(86); return (int)hle_syscall_result(); }  /* GetKeyboardStatus */
int eu_syscall_00509CD0(void) RETAIL(eu_syscall_00509CD0);
int eu_syscall_00509CD0(void) { hle_syscall(86); return (int)hle_syscall_result(); }  /* GetKeyboardStatus */
int eu_syscall_00509D20(void) RETAIL(eu_syscall_00509D20);
int eu_syscall_00509D20(void) { hle_syscall(87); return (int)hle_syscall_result(); }  /* SetLED */
int eu_syscall_004FE0D0(void) RETAIL(eu_syscall_004FE0D0);
int eu_syscall_004FE0D0(void) { hle_syscall(87); return (int)hle_syscall_result(); }  /* SetLED */
int eu_syscall_00509D40(void) RETAIL(eu_syscall_00509D40);
int eu_syscall_00509D40(void) { hle_syscall(88); return (int)hle_syscall_result(); }  /* GetLED */
int eu_syscall_00509D60(void) RETAIL(eu_syscall_00509D60);
int eu_syscall_00509D60(void) { hle_syscall(89); return (int)hle_syscall_result(); }  /* beep */
int eu_syscall_004FE0F0(void) RETAIL(eu_syscall_004FE0F0);
int eu_syscall_004FE0F0(void) { hle_syscall(89); return (int)hle_syscall_result(); }  /* beep */
int eu_syscall_00509B80(void) RETAIL(eu_syscall_00509B80);
int eu_syscall_00509B80(void) { hle_syscall(90); return (int)hle_syscall_result(); }  /* SetAlarm */
int eu_syscall_005074A8(void) RETAIL(eu_syscall_005074A8);
int eu_syscall_005074A8(void) { hle_syscall(90); return (int)hle_syscall_result(); }  /* SetAlarm */
int eu_syscall_00507338(void) RETAIL(eu_syscall_00507338);
int eu_syscall_00507338(void) { hle_syscall(90); return (int)hle_syscall_result(); }  /* SetAlarm */
int eu_syscall_00507758(void) RETAIL(eu_syscall_00507758);
int eu_syscall_00507758(void) { hle_syscall(90); return (int)hle_syscall_result(); }  /* SetAlarm */
int eu_syscall_005070F0(void) RETAIL(eu_syscall_005070F0);
int eu_syscall_005070F0(void) { hle_syscall(90); return (int)hle_syscall_result(); }  /* SetAlarm */
int eu_syscall_004FE110(void) RETAIL(eu_syscall_004FE110);
int eu_syscall_004FE110(void) { hle_syscall(91); return (int)hle_syscall_result(); }  /* iSetAlarm */
int eu_syscall_005077A0(void) RETAIL(eu_syscall_005077A0);
int eu_syscall_005077A0(void) { hle_syscall(91); return (int)hle_syscall_result(); }  /* iSetAlarm */
int eu_syscall_00509BC8(void) RETAIL(eu_syscall_00509BC8);
int eu_syscall_00509BC8(void) { hle_syscall(91); return (int)hle_syscall_result(); }  /* iSetAlarm */
int eu_syscall_00507380(void) RETAIL(eu_syscall_00507380);
int eu_syscall_00507380(void) { hle_syscall(91); return (int)hle_syscall_result(); }  /* iSetAlarm */
int eu_syscall_004FE120(void) RETAIL(eu_syscall_004FE120);
int eu_syscall_004FE120(void) { hle_syscall(92); return (int)hle_syscall_result(); }  /* CancelAlarm */
int eu_syscall_004FE140(void) RETAIL(eu_syscall_004FE140);
int eu_syscall_004FE140(void) { hle_syscall(93); return (int)hle_syscall_result(); }  /* iCancelAlarm */
int eu_syscall_004FE160(void) RETAIL(eu_syscall_004FE160);
int eu_syscall_004FE160(void) { hle_syscall(94); return (int)hle_syscall_result(); }  /* ReleaseAlarm */
int eu_syscall_004FE180(void) RETAIL(eu_syscall_004FE180);
int eu_syscall_004FE180(void) { hle_syscall(95); return (int)hle_syscall_result(); }  /* iReleaseAlarm */
int eu_syscall_004FE1A0(void) RETAIL(eu_syscall_004FE1A0);
int eu_syscall_004FE1A0(void) { hle_syscall(96); return (int)hle_syscall_result(); }  /* GetAlarmRemain */
int eu_syscall_004FE1B0(void) RETAIL(eu_syscall_004FE1B0);
int eu_syscall_004FE1B0(void) { hle_syscall(97); return (int)hle_syscall_result(); }  /* Syscall61 */
int eu_syscall_004FE1C0(void) RETAIL(eu_syscall_004FE1C0);
int eu_syscall_004FE1C0(void) { hle_syscall(98); return (int)hle_syscall_result(); }  /* Syscall62 */
int eu_syscall_004FE1D0(void) RETAIL(eu_syscall_004FE1D0);
int eu_syscall_004FE1D0(void) { hle_syscall(99); return (int)hle_syscall_result(); }  /* Syscall63 */
int eu_syscall_004FE1E0(void) RETAIL(eu_syscall_004FE1E0);
int eu_syscall_004FE1E0(void) { hle_syscall(100); return (int)hle_syscall_result(); }  /* Syscall64 */
int eu_syscall_004FE1F0(void) RETAIL(eu_syscall_004FE1F0);
int eu_syscall_004FE1F0(void) { hle_syscall(102); return (int)hle_syscall_result(); }  /* Syscall66 */
int eu_syscall_004FE230(void) RETAIL(eu_syscall_004FE230);
int eu_syscall_004FE230(void) { hle_syscall(107); return (int)hle_syscall_result(); }  /* Syscall6B */
int eu_syscall_004FE240(void) RETAIL(eu_syscall_004FE240);
int eu_syscall_004FE240(void) { hle_syscall(108); return (int)hle_syscall_result(); }  /* Syscall6C */
int eu_syscall_004FE250(void) RETAIL(eu_syscall_004FE250);
int eu_syscall_004FE250(void) { hle_syscall(109); return (int)hle_syscall_result(); }  /* Syscall6D */
int eu_syscall_004FE260(void) RETAIL(eu_syscall_004FE260);
int eu_syscall_004FE260(void) { hle_syscall(110); return (int)hle_syscall_result(); }  /* Syscall6E */
int eu_syscall_004FE270(void) RETAIL(eu_syscall_004FE270);
int eu_syscall_004FE270(void) { hle_syscall(111); return (int)hle_syscall_result(); }  /* Syscall6F */
int eu_syscall_004FE280(void) RETAIL(eu_syscall_004FE280);
int eu_syscall_004FE280(void) { hle_syscall(112); return (int)hle_syscall_result(); }  /* Syscall70 */
int eu_syscall_004FE2A0(void) RETAIL(eu_syscall_004FE2A0);
int eu_syscall_004FE2A0(void) { hle_syscall(113); return (int)hle_syscall_result(); }  /* Syscall71 */
int eu_syscall_004FE2C0(void) RETAIL(eu_syscall_004FE2C0);
int eu_syscall_004FE2C0(void) { hle_syscall(114); return (int)hle_syscall_result(); }  /* Syscall72 */
int eu_syscall_004FE2D0(void) RETAIL(eu_syscall_004FE2D0);
int eu_syscall_004FE2D0(void) { hle_syscall(115); return (int)hle_syscall_result(); }  /* Syscall73 */
int eu_syscall_005072C0(void) RETAIL(eu_syscall_005072C0);
int eu_syscall_005072C0(void) { hle_syscall(116); return (int)hle_syscall_result(); }  /* Syscall74 */
int eu_syscall_005074E8(void) RETAIL(eu_syscall_005074E8);
int eu_syscall_005074E8(void) { hle_syscall(116); return (int)hle_syscall_result(); }  /* Syscall74 */
int eu_syscall_00507328(void) RETAIL(eu_syscall_00507328);
int eu_syscall_00507328(void) { hle_syscall(116); return (int)hle_syscall_result(); }  /* Syscall74 */
int eu_syscall_004FE2E0(void) RETAIL(eu_syscall_004FE2E0);
int eu_syscall_004FE2E0(void) { hle_syscall(116); return (int)hle_syscall_result(); }  /* Syscall74 */
int eu_syscall_00509BD8(void) RETAIL(eu_syscall_00509BD8);
int eu_syscall_00509BD8(void) { hle_syscall(116); return (int)hle_syscall_result(); }  /* Syscall74 */
int eu_syscall_00507748(void) RETAIL(eu_syscall_00507748);
int eu_syscall_00507748(void) { hle_syscall(116); return (int)hle_syscall_result(); }  /* Syscall74 */
int eu_syscall_004FE2F0(void) RETAIL(eu_syscall_004FE2F0);
int eu_syscall_004FE2F0(void) { hle_syscall(117); return (int)hle_syscall_result(); }  /* Syscall75 */
int eu_syscall_004FE300(void) RETAIL(eu_syscall_004FE300);
int eu_syscall_004FE300(void) { hle_syscall(118); return (int)hle_syscall_result(); }  /* Syscall76 */
int eu_syscall_004FE320(void) RETAIL(eu_syscall_004FE320);
int eu_syscall_004FE320(void) { hle_syscall(119); return (int)hle_syscall_result(); }  /* Syscall77 */
int eu_syscall_004FE340(void) RETAIL(eu_syscall_004FE340);
int eu_syscall_004FE340(void) { hle_syscall(120); return (int)hle_syscall_result(); }  /* Syscall78 */
int eu_syscall_004FE360(void) RETAIL(eu_syscall_004FE360);
int eu_syscall_004FE360(void) { hle_syscall(121); return (int)hle_syscall_result(); }  /* Syscall79 */
int eu_syscall_004FE370(void) RETAIL(eu_syscall_004FE370);
int eu_syscall_004FE370(void) { hle_syscall(122); return (int)hle_syscall_result(); }  /* Syscall7A */
int eu_syscall_004FE380(void) RETAIL(eu_syscall_004FE380);
int eu_syscall_004FE380(void) { hle_syscall(123); return (int)hle_syscall_result(); }  /* Syscall7B */
int eu_syscall_004FE390(void) RETAIL(eu_syscall_004FE390);
int eu_syscall_004FE390(void) { hle_syscall(124); return (int)hle_syscall_result(); }  /* Syscall7C */
int eu_syscall_004FE3A0(void) RETAIL(eu_syscall_004FE3A0);
int eu_syscall_004FE3A0(void) { hle_syscall(125); return (int)hle_syscall_result(); }  /* Syscall7D */
int eu_syscall_004FE3B0(void) RETAIL(eu_syscall_004FE3B0);
int eu_syscall_004FE3B0(void) { hle_syscall(126); return (int)hle_syscall_result(); }  /* Syscall7E */
int eu_syscall_004FE3C0(void) RETAIL(eu_syscall_004FE3C0);
int eu_syscall_004FE3C0(void) { hle_syscall(127); return (int)hle_syscall_result(); }  /* Syscall7F */
int eu_syscall_004FE3D0(void) RETAIL(eu_syscall_004FE3D0);
int eu_syscall_004FE3D0(void) { hle_syscall(130); return (int)hle_syscall_result(); }  /* Syscall82 */
int eu_syscall_00507178(void) RETAIL(eu_syscall_00507178);
int eu_syscall_00507178(void) { hle_syscall(131); return (int)hle_syscall_result(); }  /* Syscall83 */
int eu_syscall_004FDCE0(void) RETAIL(eu_syscall_004FDCE0);
int eu_syscall_004FDCE0(void) { hle_syscall(252); return (int)hle_syscall_result(); }  /* SyscallFC */
int eu_syscall_004FDCF0(void) RETAIL(eu_syscall_004FDCF0);
int eu_syscall_004FDCF0(void) { hle_syscall(253); return (int)hle_syscall_result(); }  /* SyscallFD */
int eu_syscall_004FDD50(void) RETAIL(eu_syscall_004FDD50);
int eu_syscall_004FDD50(void) { hle_syscall(65281); return (int)hle_syscall_result(); }  /* SyscallFF01 */
int eu_syscall_004FDD40(void) RETAIL(eu_syscall_004FDD40);
int eu_syscall_004FDD40(void) { hle_syscall(65282); return (int)hle_syscall_result(); }  /* SyscallFF02 */
int eu_syscall_004FE350(void) RETAIL(eu_syscall_004FE350);
int eu_syscall_004FE350(void) { hle_syscall(65416); return (int)hle_syscall_result(); }  /* SyscallFF88 */
int eu_syscall_004FE330(void) RETAIL(eu_syscall_004FE330);
int eu_syscall_004FE330(void) { hle_syscall(65417); return (int)hle_syscall_result(); }  /* SyscallFF89 */
int eu_syscall_004FE310(void) RETAIL(eu_syscall_004FE310);
int eu_syscall_004FE310(void) { hle_syscall(65418); return (int)hle_syscall_result(); }  /* SyscallFF8A */
int eu_syscall_004FE2B0(void) RETAIL(eu_syscall_004FE2B0);
int eu_syscall_004FE2B0(void) { hle_syscall(65423); return (int)hle_syscall_result(); }  /* SyscallFF8F */
int eu_syscall_004FE290(void) RETAIL(eu_syscall_004FE290);
int eu_syscall_004FE290(void) { hle_syscall(65424); return (int)hle_syscall_result(); }  /* SyscallFF90 */
int eu_syscall_004FE220(void) RETAIL(eu_syscall_004FE220);
int eu_syscall_004FE220(void) { hle_syscall(65430); return (int)hle_syscall_result(); }  /* SyscallFF96 */
int eu_syscall_004FE210(void) RETAIL(eu_syscall_004FE210);
int eu_syscall_004FE210(void) { hle_syscall(65432); return (int)hle_syscall_result(); }  /* SyscallFF98 */
int eu_syscall_004FE200(void) RETAIL(eu_syscall_004FE200);
int eu_syscall_004FE200(void) { hle_syscall(65433); return (int)hle_syscall_result(); }  /* SyscallFF99 */
int eu_syscall_004FE190(void) RETAIL(eu_syscall_004FE190);
int eu_syscall_004FE190(void) { hle_syscall(65441); return (int)hle_syscall_result(); }  /* SyscallFFA1 */
int eu_syscall_004FE170(void) RETAIL(eu_syscall_004FE170);
int eu_syscall_004FE170(void) { hle_syscall(65442); return (int)hle_syscall_result(); }  /* SyscallFFA2 */
int eu_syscall_004FE150(void) RETAIL(eu_syscall_004FE150);
int eu_syscall_004FE150(void) { hle_syscall(65443); return (int)hle_syscall_result(); }  /* SyscallFFA3 */
int eu_syscall_004FE130(void) RETAIL(eu_syscall_004FE130);
int eu_syscall_004FE130(void) { hle_syscall(65444); return (int)hle_syscall_result(); }  /* SyscallFFA4 */
int eu_syscall_004FE100(void) RETAIL(eu_syscall_004FE100);
int eu_syscall_004FE100(void) { hle_syscall(65446); return (int)hle_syscall_result(); }  /* SyscallFFA6 */
int eu_syscall_00509D50(void) RETAIL(eu_syscall_00509D50);
int eu_syscall_00509D50(void) { hle_syscall(65448); return (int)hle_syscall_result(); }  /* SyscallFFA8 */
int eu_syscall_004FE0E0(void) RETAIL(eu_syscall_004FE0E0);
int eu_syscall_004FE0E0(void) { hle_syscall(65448); return (int)hle_syscall_result(); }  /* SyscallFFA8 */
int eu_syscall_00509D30(void) RETAIL(eu_syscall_00509D30);
int eu_syscall_00509D30(void) { hle_syscall(65449); return (int)hle_syscall_result(); }  /* SyscallFFA9 */
int eu_syscall_00509D10(void) RETAIL(eu_syscall_00509D10);
int eu_syscall_00509D10(void) { hle_syscall(65450); return (int)hle_syscall_result(); }  /* SyscallFFAA */
int eu_syscall_004FE0B0(void) RETAIL(eu_syscall_004FE0B0);
int eu_syscall_004FE0B0(void) { hle_syscall(65451); return (int)hle_syscall_result(); }  /* SyscallFFAB */
int eu_syscall_00509CC0(void) RETAIL(eu_syscall_00509CC0);
int eu_syscall_00509CC0(void) { hle_syscall(65451); return (int)hle_syscall_result(); }  /* SyscallFFAB */
int eu_syscall_004FE090(void) RETAIL(eu_syscall_004FE090);
int eu_syscall_004FE090(void) { hle_syscall(65453); return (int)hle_syscall_result(); }  /* SyscallFFAD */
int eu_syscall_004FDFE0(void) RETAIL(eu_syscall_004FDFE0);
int eu_syscall_004FDFE0(void) { hle_syscall(65464); return (int)hle_syscall_result(); }  /* SyscallFFB8 */
int eu_syscall_004FDFC0(void) RETAIL(eu_syscall_004FDFC0);
int eu_syscall_004FDFC0(void) { hle_syscall(65466); return (int)hle_syscall_result(); }  /* SyscallFFBA */
int eu_syscall_004FDF90(void) RETAIL(eu_syscall_004FDF90);
int eu_syscall_004FDF90(void) { hle_syscall(65469); return (int)hle_syscall_result(); }  /* SyscallFFBD */
int eu_syscall_004FDF00(void) RETAIL(eu_syscall_004FDF00);
int eu_syscall_004FDF00(void) { hle_syscall(65478); return (int)hle_syscall_result(); }  /* SyscallFFC6 */
int eu_syscall_004FDEE0(void) RETAIL(eu_syscall_004FDEE0);
int eu_syscall_004FDEE0(void) { hle_syscall(65480); return (int)hle_syscall_result(); }  /* SyscallFFC8 */
int eu_syscall_004FDEC0(void) RETAIL(eu_syscall_004FDEC0);
int eu_syscall_004FDEC0(void) { hle_syscall(65482); return (int)hle_syscall_result(); }  /* SyscallFFCA */
int eu_syscall_004FDEA0(void) RETAIL(eu_syscall_004FDEA0);
int eu_syscall_004FDEA0(void) { hle_syscall(65484); return (int)hle_syscall_result(); }  /* SyscallFFCC */
int eu_syscall_004FDE70(void) RETAIL(eu_syscall_004FDE70);
int eu_syscall_004FDE70(void) { hle_syscall(65487); return (int)hle_syscall_result(); }  /* SyscallFFCF */
int eu_syscall_004FDE40(void) RETAIL(eu_syscall_004FDE40);
int eu_syscall_004FDE40(void) { hle_syscall(65490); return (int)hle_syscall_result(); }  /* SyscallFFD2 */
int eu_syscall_004FDE20(void) RETAIL(eu_syscall_004FDE20);
int eu_syscall_004FDE20(void) { hle_syscall(65492); return (int)hle_syscall_result(); }  /* SyscallFFD4 */
int eu_syscall_004FDE00(void) RETAIL(eu_syscall_004FDE00);
int eu_syscall_004FDE00(void) { hle_syscall(65494); return (int)hle_syscall_result(); }  /* SyscallFFD6 */
int eu_syscall_004FDDC0(void) RETAIL(eu_syscall_004FDDC0);
int eu_syscall_004FDDC0(void) { hle_syscall(65498); return (int)hle_syscall_result(); }  /* SyscallFFDA */
int eu_syscall_004FDD30(void) RETAIL(eu_syscall_004FDD30);
int eu_syscall_004FDD30(void) { hle_syscall(65507); return (int)hle_syscall_result(); }  /* SyscallFFE3 */
int eu_syscall_004FDD20(void) RETAIL(eu_syscall_004FDD20);
int eu_syscall_004FDD20(void) { hle_syscall(65508); return (int)hle_syscall_result(); }  /* SyscallFFE4 */
int eu_syscall_004FDD10(void) RETAIL(eu_syscall_004FDD10);
int eu_syscall_004FDD10(void) { hle_syscall(65509); return (int)hle_syscall_result(); }  /* SyscallFFE5 */
int eu_syscall_004FDD00(void) RETAIL(eu_syscall_004FDD00);
int eu_syscall_004FDD00(void) { hle_syscall(65510); return (int)hle_syscall_result(); }  /* SyscallFFE6 */

/* The recompiled code calls the wrapper by its guest address; this names the host function. */
__asm__(".globl func_004FDB40\n.set func_004FDB40, eu_syscall_004FDB40");
__asm__(".globl func_004FDB50\n.set func_004FDB50, eu_syscall_004FDB50");
__asm__(".globl func_004FDB60\n.set func_004FDB60, eu_syscall_004FDB60");
__asm__(".globl func_004FDB70\n.set func_004FDB70, eu_syscall_004FDB70");
__asm__(".globl func_004FDB80\n.set func_004FDB80, eu_syscall_004FDB80");
__asm__(".globl func_004FDB90\n.set func_004FDB90, eu_syscall_004FDB90");
__asm__(".globl func_004FDBA0\n.set func_004FDBA0, eu_syscall_004FDBA0");
__asm__(".globl func_004FDBB0\n.set func_004FDBB0, eu_syscall_004FDBB0");
__asm__(".globl func_004FDBC0\n.set func_004FDBC0, eu_syscall_004FDBC0");
__asm__(".globl func_004FDBD0\n.set func_004FDBD0, eu_syscall_004FDBD0");
__asm__(".globl func_004FDBE0\n.set func_004FDBE0, eu_syscall_004FDBE0");
__asm__(".globl func_004FDBF0\n.set func_004FDBF0, eu_syscall_004FDBF0");
__asm__(".globl func_004FDC00\n.set func_004FDC00, eu_syscall_004FDC00");
__asm__(".globl func_004FDC10\n.set func_004FDC10, eu_syscall_004FDC10");
__asm__(".globl func_004FDC20\n.set func_004FDC20, eu_syscall_004FDC20");
__asm__(".globl func_004FDC30\n.set func_004FDC30, eu_syscall_004FDC30");
__asm__(".globl func_004FDC50\n.set func_004FDC50, eu_syscall_004FDC50");
__asm__(".globl func_004FDC40\n.set func_004FDC40, eu_syscall_004FDC40");
__asm__(".globl func_004FDC60\n.set func_004FDC60, eu_syscall_004FDC60");
__asm__(".globl func_004FDC70\n.set func_004FDC70, eu_syscall_004FDC70");
__asm__(".globl func_004FDC80\n.set func_004FDC80, eu_syscall_004FDC80");
__asm__(".globl func_004FDC90\n.set func_004FDC90, eu_syscall_004FDC90");
__asm__(".globl func_004FDCA0\n.set func_004FDCA0, eu_syscall_004FDCA0");
__asm__(".globl func_004FDCB0\n.set func_004FDCB0, eu_syscall_004FDCB0");
__asm__(".globl func_004FDCC0\n.set func_004FDCC0, eu_syscall_004FDCC0");
__asm__(".globl func_004FDCD0\n.set func_004FDCD0, eu_syscall_004FDCD0");
__asm__(".globl func_004FDD60\n.set func_004FDD60, eu_syscall_004FDD60");
__asm__(".globl func_004FDD70\n.set func_004FDD70, eu_syscall_004FDD70");
__asm__(".globl func_004FDD80\n.set func_004FDD80, eu_syscall_004FDD80");
__asm__(".globl func_00100228\n.set func_00100228, eu_syscall_00100228");
__asm__(".globl func_004FDD90\n.set func_004FDD90, eu_syscall_004FDD90");
__asm__(".globl func_004FDDA0\n.set func_004FDDA0, eu_syscall_004FDDA0");
__asm__(".globl func_004FDDB0\n.set func_004FDDB0, eu_syscall_004FDDB0");
__asm__(".globl func_004FDDD0\n.set func_004FDDD0, eu_syscall_004FDDD0");
__asm__(".globl func_004FDDE0\n.set func_004FDDE0, eu_syscall_004FDDE0");
__asm__(".globl func_004FDDF0\n.set func_004FDDF0, eu_syscall_004FDDF0");
__asm__(".globl func_004FDE10\n.set func_004FDE10, eu_syscall_004FDE10");
__asm__(".globl func_004FDE30\n.set func_004FDE30, eu_syscall_004FDE30");
__asm__(".globl func_004FDE50\n.set func_004FDE50, eu_syscall_004FDE50");
__asm__(".globl func_004FDE60\n.set func_004FDE60, eu_syscall_004FDE60");
__asm__(".globl func_004FDE80\n.set func_004FDE80, eu_syscall_004FDE80");
__asm__(".globl func_004FDE90\n.set func_004FDE90, eu_syscall_004FDE90");
__asm__(".globl func_004FDEB0\n.set func_004FDEB0, eu_syscall_004FDEB0");
__asm__(".globl func_004FDED0\n.set func_004FDED0, eu_syscall_004FDED0");
__asm__(".globl func_004FDEF0\n.set func_004FDEF0, eu_syscall_004FDEF0");
__asm__(".globl func_004FDF10\n.set func_004FDF10, eu_syscall_004FDF10");
__asm__(".globl func_004FDF20\n.set func_004FDF20, eu_syscall_004FDF20");
__asm__(".globl func_004FDF30\n.set func_004FDF30, eu_syscall_004FDF30");
__asm__(".globl func_004FDF40\n.set func_004FDF40, eu_syscall_004FDF40");
__asm__(".globl func_004FDF50\n.set func_004FDF50, eu_syscall_004FDF50");
__asm__(".globl func_004FDF60\n.set func_004FDF60, eu_syscall_004FDF60");
__asm__(".globl func_004FDF70\n.set func_004FDF70, eu_syscall_004FDF70");
__asm__(".globl func_004FDF80\n.set func_004FDF80, eu_syscall_004FDF80");
__asm__(".globl func_004FDFA0\n.set func_004FDFA0, eu_syscall_004FDFA0");
__asm__(".globl func_004FDFB0\n.set func_004FDFB0, eu_syscall_004FDFB0");
__asm__(".globl func_004FDFD0\n.set func_004FDFD0, eu_syscall_004FDFD0");
__asm__(".globl func_004FDFF0\n.set func_004FDFF0, eu_syscall_004FDFF0");
__asm__(".globl func_004FE000\n.set func_004FE000, eu_syscall_004FE000");
__asm__(".globl func_004FE010\n.set func_004FE010, eu_syscall_004FE010");
__asm__(".globl func_004FE020\n.set func_004FE020, eu_syscall_004FE020");
__asm__(".globl func_004FE030\n.set func_004FE030, eu_syscall_004FE030");
__asm__(".globl func_004FE040\n.set func_004FE040, eu_syscall_004FE040");
__asm__(".globl func_004FE050\n.set func_004FE050, eu_syscall_004FE050");
__asm__(".globl func_004FE060\n.set func_004FE060, eu_syscall_004FE060");
__asm__(".globl func_004FE070\n.set func_004FE070, eu_syscall_004FE070");
__asm__(".globl func_004FE080\n.set func_004FE080, eu_syscall_004FE080");
__asm__(".globl func_004FE0A0\n.set func_004FE0A0, eu_syscall_004FE0A0");
__asm__(".globl func_00509CB0\n.set func_00509CB0, eu_syscall_00509CB0");
__asm__(".globl func_004FE0C0\n.set func_004FE0C0, eu_syscall_004FE0C0");
__asm__(".globl func_00509CD0\n.set func_00509CD0, eu_syscall_00509CD0");
__asm__(".globl func_00509D20\n.set func_00509D20, eu_syscall_00509D20");
__asm__(".globl func_004FE0D0\n.set func_004FE0D0, eu_syscall_004FE0D0");
__asm__(".globl func_00509D40\n.set func_00509D40, eu_syscall_00509D40");
__asm__(".globl func_00509D60\n.set func_00509D60, eu_syscall_00509D60");
__asm__(".globl func_004FE0F0\n.set func_004FE0F0, eu_syscall_004FE0F0");
__asm__(".globl func_00509B80\n.set func_00509B80, eu_syscall_00509B80");
__asm__(".globl func_005074A8\n.set func_005074A8, eu_syscall_005074A8");
__asm__(".globl func_00507338\n.set func_00507338, eu_syscall_00507338");
__asm__(".globl func_00507758\n.set func_00507758, eu_syscall_00507758");
__asm__(".globl func_005070F0\n.set func_005070F0, eu_syscall_005070F0");
__asm__(".globl func_004FE110\n.set func_004FE110, eu_syscall_004FE110");
__asm__(".globl func_005077A0\n.set func_005077A0, eu_syscall_005077A0");
__asm__(".globl func_00509BC8\n.set func_00509BC8, eu_syscall_00509BC8");
__asm__(".globl func_00507380\n.set func_00507380, eu_syscall_00507380");
__asm__(".globl func_004FE120\n.set func_004FE120, eu_syscall_004FE120");
__asm__(".globl func_004FE140\n.set func_004FE140, eu_syscall_004FE140");
__asm__(".globl func_004FE160\n.set func_004FE160, eu_syscall_004FE160");
__asm__(".globl func_004FE180\n.set func_004FE180, eu_syscall_004FE180");
__asm__(".globl func_004FE1A0\n.set func_004FE1A0, eu_syscall_004FE1A0");
__asm__(".globl func_004FE1B0\n.set func_004FE1B0, eu_syscall_004FE1B0");
__asm__(".globl func_004FE1C0\n.set func_004FE1C0, eu_syscall_004FE1C0");
__asm__(".globl func_004FE1D0\n.set func_004FE1D0, eu_syscall_004FE1D0");
__asm__(".globl func_004FE1E0\n.set func_004FE1E0, eu_syscall_004FE1E0");
__asm__(".globl func_004FE1F0\n.set func_004FE1F0, eu_syscall_004FE1F0");
__asm__(".globl func_004FE230\n.set func_004FE230, eu_syscall_004FE230");
__asm__(".globl func_004FE240\n.set func_004FE240, eu_syscall_004FE240");
__asm__(".globl func_004FE250\n.set func_004FE250, eu_syscall_004FE250");
__asm__(".globl func_004FE260\n.set func_004FE260, eu_syscall_004FE260");
__asm__(".globl func_004FE270\n.set func_004FE270, eu_syscall_004FE270");
__asm__(".globl func_004FE280\n.set func_004FE280, eu_syscall_004FE280");
__asm__(".globl func_004FE2A0\n.set func_004FE2A0, eu_syscall_004FE2A0");
__asm__(".globl func_004FE2C0\n.set func_004FE2C0, eu_syscall_004FE2C0");
__asm__(".globl func_004FE2D0\n.set func_004FE2D0, eu_syscall_004FE2D0");
__asm__(".globl func_005072C0\n.set func_005072C0, eu_syscall_005072C0");
__asm__(".globl func_005074E8\n.set func_005074E8, eu_syscall_005074E8");
__asm__(".globl func_00507328\n.set func_00507328, eu_syscall_00507328");
__asm__(".globl func_004FE2E0\n.set func_004FE2E0, eu_syscall_004FE2E0");
__asm__(".globl func_00509BD8\n.set func_00509BD8, eu_syscall_00509BD8");
__asm__(".globl func_00507748\n.set func_00507748, eu_syscall_00507748");
__asm__(".globl func_004FE2F0\n.set func_004FE2F0, eu_syscall_004FE2F0");
__asm__(".globl func_004FE300\n.set func_004FE300, eu_syscall_004FE300");
__asm__(".globl func_004FE320\n.set func_004FE320, eu_syscall_004FE320");
__asm__(".globl func_004FE340\n.set func_004FE340, eu_syscall_004FE340");
__asm__(".globl func_004FE360\n.set func_004FE360, eu_syscall_004FE360");
__asm__(".globl func_004FE370\n.set func_004FE370, eu_syscall_004FE370");
__asm__(".globl func_004FE380\n.set func_004FE380, eu_syscall_004FE380");
__asm__(".globl func_004FE390\n.set func_004FE390, eu_syscall_004FE390");
__asm__(".globl func_004FE3A0\n.set func_004FE3A0, eu_syscall_004FE3A0");
__asm__(".globl func_004FE3B0\n.set func_004FE3B0, eu_syscall_004FE3B0");
__asm__(".globl func_004FE3C0\n.set func_004FE3C0, eu_syscall_004FE3C0");
__asm__(".globl func_004FE3D0\n.set func_004FE3D0, eu_syscall_004FE3D0");
__asm__(".globl func_00507178\n.set func_00507178, eu_syscall_00507178");
__asm__(".globl func_004FDCE0\n.set func_004FDCE0, eu_syscall_004FDCE0");
__asm__(".globl func_004FDCF0\n.set func_004FDCF0, eu_syscall_004FDCF0");
__asm__(".globl func_004FDD50\n.set func_004FDD50, eu_syscall_004FDD50");
__asm__(".globl func_004FDD40\n.set func_004FDD40, eu_syscall_004FDD40");
__asm__(".globl func_004FE350\n.set func_004FE350, eu_syscall_004FE350");
__asm__(".globl func_004FE330\n.set func_004FE330, eu_syscall_004FE330");
__asm__(".globl func_004FE310\n.set func_004FE310, eu_syscall_004FE310");
__asm__(".globl func_004FE2B0\n.set func_004FE2B0, eu_syscall_004FE2B0");
__asm__(".globl func_004FE290\n.set func_004FE290, eu_syscall_004FE290");
__asm__(".globl func_004FE220\n.set func_004FE220, eu_syscall_004FE220");
__asm__(".globl func_004FE210\n.set func_004FE210, eu_syscall_004FE210");
__asm__(".globl func_004FE200\n.set func_004FE200, eu_syscall_004FE200");
__asm__(".globl func_004FE190\n.set func_004FE190, eu_syscall_004FE190");
__asm__(".globl func_004FE170\n.set func_004FE170, eu_syscall_004FE170");
__asm__(".globl func_004FE150\n.set func_004FE150, eu_syscall_004FE150");
__asm__(".globl func_004FE130\n.set func_004FE130, eu_syscall_004FE130");
__asm__(".globl func_004FE100\n.set func_004FE100, eu_syscall_004FE100");
__asm__(".globl func_00509D50\n.set func_00509D50, eu_syscall_00509D50");
__asm__(".globl func_004FE0E0\n.set func_004FE0E0, eu_syscall_004FE0E0");
__asm__(".globl func_00509D30\n.set func_00509D30, eu_syscall_00509D30");
__asm__(".globl func_00509D10\n.set func_00509D10, eu_syscall_00509D10");
__asm__(".globl func_004FE0B0\n.set func_004FE0B0, eu_syscall_004FE0B0");
__asm__(".globl func_00509CC0\n.set func_00509CC0, eu_syscall_00509CC0");
__asm__(".globl func_004FE090\n.set func_004FE090, eu_syscall_004FE090");
__asm__(".globl func_004FDFE0\n.set func_004FDFE0, eu_syscall_004FDFE0");
__asm__(".globl func_004FDFC0\n.set func_004FDFC0, eu_syscall_004FDFC0");
__asm__(".globl func_004FDF90\n.set func_004FDF90, eu_syscall_004FDF90");
__asm__(".globl func_004FDF00\n.set func_004FDF00, eu_syscall_004FDF00");
__asm__(".globl func_004FDEE0\n.set func_004FDEE0, eu_syscall_004FDEE0");
__asm__(".globl func_004FDEC0\n.set func_004FDEC0, eu_syscall_004FDEC0");
__asm__(".globl func_004FDEA0\n.set func_004FDEA0, eu_syscall_004FDEA0");
__asm__(".globl func_004FDE70\n.set func_004FDE70, eu_syscall_004FDE70");
__asm__(".globl func_004FDE40\n.set func_004FDE40, eu_syscall_004FDE40");
__asm__(".globl func_004FDE20\n.set func_004FDE20, eu_syscall_004FDE20");
__asm__(".globl func_004FDE00\n.set func_004FDE00, eu_syscall_004FDE00");
__asm__(".globl func_004FDDC0\n.set func_004FDDC0, eu_syscall_004FDDC0");
__asm__(".globl func_004FDD30\n.set func_004FDD30, eu_syscall_004FDD30");
__asm__(".globl func_004FDD20\n.set func_004FDD20, eu_syscall_004FDD20");
__asm__(".globl func_004FDD10\n.set func_004FDD10, eu_syscall_004FDD10");
__asm__(".globl func_004FDD00\n.set func_004FDD00, eu_syscall_004FDD00");
