#!/bin/bash

# Run ELKS previously built in EMU86 configuration (see elks/emu86.sh)

exec ./emu86 -w 0xe0000 -f ../elks-upstream/elks/arch/i86/boot/Image -w 0x80000 -f ../elks-upstream/image/romfs.bin $@

# Run ELKS in browser (set PLATFORM=emscripten in config.mk and rebuild EMU86):
# source ~/emsdk/emsdk_env.sh
# make

exec emrun --serve_after_close emu86.html -- -v3 -w 0x10000 -x 0x1000:0x34 -f emu-main.c
