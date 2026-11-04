typedef int s32;

struct VObj {
    char pad[0x68];
    s32 f68;
};

extern "C" void func_0033FF30(VObj *);

extern "C" void func_001009D8(VObj *arg0) {
    arg0->f68 = 0;
    func_0033FF30(arg0);
}
