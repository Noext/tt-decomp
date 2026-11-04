/* The data labels the recompiled code names, made by tools/make_data_fixups.py.
   Each is its guest address: with flat guest memory (ps2asm.h, PS2_HOST_FLAT_MEMORY)
   that address IS the host address of the bytes, so C can take it directly. */
#include <stdint.h>

#define PS2_DATA(name, addr) extern uint8_t name[] __asm__(#name);\
