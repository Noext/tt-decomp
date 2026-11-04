#!/bin/sh
# Build and run the port on the user's own copy of the game.
#   port/tools/run.sh IMAGE.bin [ENTRY]
# None of the game's bytes are in this repository: extract them first with
#   tools/core2elf.py CORE.TT CORE.TT.elf   then cut the load image out.
set -e
cd "$(dirname "$0")/.."
make -s
exec ./bootport "${1:?usage: run.sh IMAGE.bin [ENTRY]}" "${2:-0x00100008}"
