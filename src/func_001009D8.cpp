struct Obj_001009D8 {
    char pad[0x68];
    int unk68;
};

extern "C" void func_0033FF30(struct Obj_001009D8 *) throw();

extern "C" void func_001009D8(struct Obj_001009D8 *arg0) {
    arg0->unk68 = 0;
    func_0033FF30(arg0);
}
