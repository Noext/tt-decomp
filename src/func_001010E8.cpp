typedef int s32;

extern "C" s32 func_004E1D90(void);
extern "C" void func_004794E8(void);
extern "C" void func_00472BC8(void);
extern "C" void func_0046CE98(void);

extern "C" void func_001010E8(void) {
    while (func_004E1D90() != 1) {
        func_004794E8();
    }
    func_00472BC8();
    func_0046CE98();
}
