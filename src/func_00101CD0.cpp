typedef int s32;
typedef short s16;

struct VEntry {
    s16 delta;
    s16 index;
    void (*fn)(char *, char *);
};

struct Obj {
    char pad0[0x9C];
    char **unk9C;
    s32 pad1[2];
    s32 unkA8;
    s32 unkAC;
};

extern "C" void func_00106068(char *arg0);
extern "C" void func_00476DB0(char *arg0);
extern "C" void func_00476E20(char *arg0);

extern "C" void func_00101CD0(Obj *arg0) {
    char *p = (char *)arg0 + 0x6C;

    func_00476DB0(p);
    arg0->unkA8 = 0;
    char **obj = arg0->unk9C;
    arg0->unkAC = 0;
    VEntry *e = (VEntry *)(*obj + 0x10);
    e->fn((char *)obj + e->delta, (char *)arg0);
    func_00476E20(p);
    func_00106068((char *)arg0);
}
