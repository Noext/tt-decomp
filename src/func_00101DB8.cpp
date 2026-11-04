typedef int s32;

struct Obj {
    char pad0[0xAC];
    s32 count;
};

extern "C" void func_00476DB0(void *);
extern "C" void func_00476E20(void *);
extern "C" void func_00106078(Obj *);

extern "C" void func_00101DB8(Obj *self) {
    void *lock = (char *)self + 0x6C;
    func_00476DB0(lock);
    s32 n = self->count;
    if (n < 3) {
        n++;
        self->count = n;
        if (n == 3)
            func_00106078(self);
    }
    func_00476E20(lock);
}
