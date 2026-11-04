typedef short s16;

struct VEntry {
    s16 delta;
    s16 index;
    void (*fn)(void *, int);
};

struct VObj {
    char pad[0x64];
    char *vt;
};

extern "C" void func_00100B30(VObj *arg0, int arg1) {
    VEntry *e = (VEntry *)(arg0->vt + 0x80);
    e->fn((char *)arg0 + e->delta, arg1);
}
