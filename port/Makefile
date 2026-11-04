# The port's build: the runtime, the HLE and the host, linked with every recompiled function.
#
#   make          the game binary (bootport), from what is already generated
#   make clean    remove the port's own objects and the binary
#
# Nothing here is game data: the image is read from the user's own CORE.TT at run time.
CC      ?= clang
# PIE: the guest RAM needs 0x00400000-0x02000000 for its low view, which a non-PIE binary would take.
CFLAGS  := -O2 -g -fPIE -D_GNU_SOURCE -DPS2_HOST_FLAT_MEMORY \
           -I gs -I runtime -I hle -I include -I host
RCFLAGS := -O1 -w -fPIE -fPIC -D_GNU_SOURCE -DPS2_HOST_FLAT_MEMORY \
           -I hle -I runtime -I include -I host
LDFLAGS := -lm

HOST_SRC := runtime/vu0_interp.c runtime/ps2asm.c runtime/memory.c runtime/recomp.c \
            gs/gs.c gs/memory.c hle/vif.c hle/kernel.c hle/syscalls.c hle/entry_table.c \
            hle/data_labels.c hle/unrecompiled.c \
            host/host_linux.c host/port_glue.c host/boot_main.c
HOST_OBJ := $(HOST_SRC:.c=.o)

# The game as recomp.py translated it (one .c per function) plus the scalar dispatcher.
RC_DIR  := ../build/port/recomp
RC_OBJ  := ../build/port/rcobj
RC_SRC  := $(wildcard $(RC_DIR)/func_*.c) $(RC_DIR)/scalar_dispatch.c
RC_OBJS := $(patsubst $(RC_DIR)/%.c,$(RC_OBJ)/%.o,$(RC_SRC))

all: bootport

bootport: $(HOST_OBJ) $(RC_OBJS)
	$(CC) $(CFLAGS) $(HOST_OBJ) $(RC_OBJS) -o $@ $(LDFLAGS)

$(RC_OBJ)/%.o: $(RC_DIR)/%.c
	@mkdir -p $(RC_OBJ)
	$(CC) $(RCFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(HOST_OBJ) bootport

.PHONY: all clean
