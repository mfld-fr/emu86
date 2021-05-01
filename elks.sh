#!/bin/bash

# Run ELKS previously built in EMU86 configuration (see elks/emu86.sh)

./emu86 -w 0xe0000 -f ../elks-upstream/elks/arch/i86/boot/Image -w 0x80000 -f ../elks-upstream/image/romfs.bin $@
