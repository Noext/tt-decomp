extern "C" void func_00101268(void);
extern "C" void func_003003B0(void *arg0, int arg1);

extern "C" char *D_005818D4;

extern "C" void func_00101480(void) {
    func_00101268();
    func_003003B0(D_005818D4 + 0x710, 0);
    func_003003B0(D_005818D4 + 0xF20, 0);
}
