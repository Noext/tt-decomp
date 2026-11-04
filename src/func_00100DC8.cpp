typedef int s32;
typedef unsigned int u32;

extern "C" void func_00100D00(s32 arg0, u32 arg1);

extern "C" void func_00100DC8(void) {
    func_00100D00(0, 0xFFFF);
}
