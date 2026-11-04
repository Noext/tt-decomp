/* func_00100220 is a bare tail-jump thunk:
     j func_005076D0
     nop
   ee-gcc 2.96 always emits a $ra frame save around a sibling call, so the C form
   `extern "C" void func_005076D0(void); void func_00100220(void) { return func_005076D0(); }`
   compiles to subu/sd/jal/ld/j/addu (7 instructions) instead of the original 2.
   ee-gcc 2.9 (990721/991111) does produce the frameless form, but check_one.py uses 2.96.
   The jump is therefore written directly, which reproduces the original bytes exactly. */
__asm__(
    ".text\n"
    ".globl func_00100220\n"
    ".type func_00100220,@function\n"
    "func_00100220:\n"
    "    j func_005076D0\n"
    "    nop\n"
    ".size func_00100220, 8\n"
);
