/* Vararg bridging for the recompiled code, per host ABI.

   On the x86-64 hosts the SysV ABI passes varargs in registers the recompiled code does not model,
   so the bridged call must be made through a real C vararg function. The recompiled code only
   needs the *entry point* here; downhill-revived's i386 build (port/hle/vararg_abi.h there) carries
   the full register-slot walk, which applies only to that target. */
#ifndef VARARG_ABI_H
#define VARARG_ABI_H

#include <stdarg.h>
#include <stdint.h>

#endif
