# The port's own build: the runtime, the HLE and the host, linked into one Linux binary.
# Nothing here is game data; the image is read from the user's own CORE.TT at run time.
CC      ?= clang
CFLAGS  := -O2 -g -D_GNU_SOURCE -DPS2_HOST_FLAT_MEMORY \
           -I gs -I runtime -I hle -I include -I host
LDFLAGS := -lm

SRC := runtime/vu0_interp.c runtime/ps2asm.c runtime/memory.c \
       gs/gs.c gs/memory.c hle/vif.c hle/kernel.c hle/syscalls.c \
       host/host_linux.c host/port_glue.c host/boot_main.c
OBJ := $(SRC:.c=.o)

all: bootport

bootport: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) bootport

.PHONY: all clean
