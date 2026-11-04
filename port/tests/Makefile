# The HLE's own tests: no game data, no emulator. `make test` builds and runs them all.
CC      ?= clang
CFLAGS  := -O2 -g -D_POSIX_C_SOURCE=199309L -I ../hle
OBJS    := kernel.o

all: test_thread test_sync

kernel.o: ../hle/kernel.c ../hle/hle.h
	$(CC) $(CFLAGS) -c $< -o $@

test_thread: test_thread.c kernel.o
	$(CC) $(CFLAGS) $< kernel.o -o $@

test_sync: test_sync.c kernel.o
	$(CC) $(CFLAGS) $< kernel.o -o $@

test: all
	./test_thread
	./test_sync

clean:
	rm -f *.o test_thread test_sync
