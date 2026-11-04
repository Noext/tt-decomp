# The HLE's own tests: no game data, no emulator. `make test` builds and runs them all.
CC      ?= clang
CFLAGS  := -O2 -g -D_POSIX_C_SOURCE=200809L -DPS2_HOST_FLAT_MEMORY -I ../hle
RECOMP  := ../../build/port/recomp
OBJS    := kernel.o

all: test_thread test_sync test_recompiled test_map

kernel.o: ../hle/kernel.c ../hle/hle.h
	$(CC) $(CFLAGS) -c $< -o $@

test_thread: test_thread.c kernel.o
	$(CC) $(CFLAGS) $< kernel.o -o $@

test_sync: test_sync.c kernel.o
	$(CC) $(CFLAGS) $< kernel.o -o $@

# No kernel here: the classifier is pure arithmetic, and the test supplies its own fault hook.
test_map: test_map.c ../runtime/ps2asm.h
	$(CC) $(CFLAGS) -I ../runtime -I ../hle -I ../host -I ../gs -I ../include $< -o $@

test: all
	./test_thread
	./test_sync
	./test_recompiled
	./test_map

clean:
	rm -f *.o test_thread test_sync test_recompiled test_map
