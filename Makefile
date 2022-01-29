# EMU86 main makefile

include config.mk

CFLAGS = -g -Wall -Werror
PREFIX = /usr/local

# EMU86 main program

EMU86_PROG = emu86

ifeq ($(HOST), emscripten)
CC = emcc -s ASYNCIFY -O3 --emrun -s USE_SDL=2 -DSDL=1
#CC = emcc -s ASYNCIFY -O2 --emrun -s USE_SDL=2 -DSDL=1 --closure 1 -flto
CFLAGS =
LDLIBS = --preload-file=../elks-gh/elks/arch/i86/boot/Image@Image \
          --preload-file=../elks-gh/image/romfs.bin@romfs.bin
EMU86_PROG = emu86.html
endif

EMU86_HDRS = \
	list.h \
	emu-types.h \
	op-id.h \
	op-id-name.h \
	op-common.h \
	op-class.h \
	emu-mem-io.h \
	emu-proc.h \
	op-exec.h \
	emu-int.h \
	emu-timer.h \
	emu-serial.h \
	emu-con.h \
	emu-char.h \
	# end of list

EMU86_OBJS = \
	list.o \
	op-common.o \
	op-id-name.o \
	op-class.o \
	emu-mem-io.o \
	emu-proc.o \
	op-exec.o \
	emu-int.o \
	emu-main.o \
	mapfile.o
	# end of list

EMU86_OBJS += op-print-$(STYLE).o


# Force console to SDL for emscripten

ifeq ($(HOST), emscripten)
	CONSOLE=sdl
endif


# Console backend

ifeq ($(CONSOLE), none)
	EMU86_OBJS += con-none.o
endif

ifeq ($(CONSOLE), stdio)
	EMU86_OBJS += emu-con.o con-char.o char-stdio.o
endif

ifeq ($(CONSOLE), pty)
	EMU86_OBJS += emu-con.o con-char.o char-pty.o
endif

ifeq ($(CONSOLE), sdl)
	EMU86_OBJS += emu-con.o con-sdl.o rom8x16.o
	CFLAGS += -DSDL=1
	LDLIBS += -lSDL2
endif


# Serial backend

ifeq ($(SERIAL), none)
	EMU86_OBJS += serial-none.o
endif

ifeq ($(SERIAL), stdio)
	EMU86_OBJS += serial-char.o char-stdio.o
endif

ifeq ($(SERIAL), pty)
	EMU86_OBJS += serial-char.o char-pty.o
endif


# Target selection

ifeq ($(TARGET), pcxtat)
	CPU = 808x
	PLATFORM = 8xxx
	BOARD = pcxtat
	EMU86_OBJS += rom-bios.o rom-pcxtat.o
endif

ifeq ($(TARGET), elks)
	CPU = 808x
	PLATFORM = 8xxx
	BOARD = pcxtat
	EMU86_OBJS += rom-bios.o rom-elks.o
endif

ifeq ($(TARGET), advtech)
	CFLAGS += -DMCU_R8810
	CPU = 8018x
	PLATFORM = r8810
	BOARD = advtech
	EMU86_OBJS += rom-advtech.o
endif

ifeq ($(TARGET), or566)
	CFLAGS += -DMCU_8018X_EB
	CPU = 8018x
	PLATFORM = 8018x
	BOARD = or566
	EMU86_OBJS += rom-or566.o
endif

EMU86_OBJS += \
	mem-$(BOARD).o \
	io-$(BOARD).o \
	int-$(PLATFORM).o \
	timer-$(PLATFORM).o \
	serial-$(PLATFORM).o \
	# end of list


# PCAT utility for EMU86 serial port

PCAT_PROG = pcat

PCAT_OBJS = pcat-main.o


# Rules

.PHONY: all install clean

all: $(EMU86_PROG) $(PCAT_PROG)

install: $(EMU86_PROG) $(PCAT_PROG)
	install -m 755 -s $(EMU86_PROG) $(PREFIX)/bin/$(EMU86_PROG)
	install -m 755 -s $(PCAT_PROG) $(PREFIX)/bin/$(PCAT_PROG)

clean:
	rm -f *.o $(EMU86_PROG) $(PCAT_PROG) emu86.pts
	rm -f emu86.html emu86.js emu86.wasm emu86.data pcat.html pcat.wasm

$(EMU86_OBJS): $(EMU86_HDRS)

$(EMU86_PROG): $(EMU86_OBJS)
	$(CC) -o $(EMU86_PROG) $(EMU86_OBJS) $(LDLIBS)

$(PCAT_PROG): $(PCAT_OBJS)
	$(CC) -o $(PCAT_PROG) $(PCAT_OBJS)
