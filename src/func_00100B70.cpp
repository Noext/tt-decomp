typedef int s32;

struct func_00100B70_arg0 {
    char pad0[0x68];
    s32 unk68;
};

extern "C" void func_00476DB0(struct func_00100B70_arg0 *);
extern "C" void func_00476E20(struct func_00100B70_arg0 *);

extern "C" void func_00100B70(struct func_00100B70_arg0 *arg0) {
    func_00476DB0(arg0);
    if (arg0->unk68 == 0) {
        arg0->unk68 = 1;
    }
    func_00476E20(arg0);
}
