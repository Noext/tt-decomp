struct S {
    char pad[0x68];
    int field;
};

extern "C" void func_003400F8(S *);

void func_00100C78(S *arg0) {
    if (arg0->field < 2)
        func_003400F8(arg0);
}
