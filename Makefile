# EMU86 main makefile

CFLAGS = -g
PREFIX = /usr/local

# EMU86 main program

EMU86_PROG = emu86

EMU86_HDRS = \
	op-common.h \
	op-id-name.h \
	op-class.h \
	emu-mem-io.h \
	emu-proc.h \
	emu-serial.h \
	emu-int.h \
	op-exec.h \
	# end of list

EMU86_OBJS = \
	op-common.o \
	op-id-name.o \
	op-class.o \
	emu-mem-io.o \
	emu-proc.o \
	emu-int.o \
	op-exec.o \
	emu-main.o \
	#end of list

# Disassembly style
# AT&T syntax (GNU default)
# Intel syntax

#STYLE=att
STYLE=intel

EMU86_OBJS += op-print-$(STYLE).o

# Serial emulation
# Connected to EMU86 stdin & stdout
# Connected to PTY (created by EMU86)

#SERIAL=console
SERIAL=pty

EMU86_OBJS += emu-$(SERIAL).o

# Target selection
# elks = minimal PC to run ELKS
# advtech = Advantech SNMP-1000 SBC

TARGET=elks
#TARGET=advtech

EMU86_OBJS += int-$(TARGET).o

# PCAT utility for EMU86 serial port

PCAT_PROG = pcat

PCAT_SRCS = pcat-main.c
PCAT_OBJS = pcat-main.o

# Rules

.PHONY: all install clean

all: $(EMU86_PROG) $(PCAT_PROG)

install: $(EMU86_PROG) $(PCAT_PROG)
	install -m 755 -s $(EMU86_PROG) $(PREFIX)/bin/$(EMU86_PROG)
	install -m 755 -s $(PCAT_PROG) $(PREFIX)/bin/$(PCAT_PROG)

clean:
	rm -f $(EMU86_OBJS) $(EMU86_PROG) $(PCAT_OBJS) $(PCAT_PROG)

$(EMU86_OBJS): $(EMU86_HDRS)

$(EMU86_PROG): $(EMU86_OBJS)
	$(CC) -o $(EMU86_PROG) $(EMU86_OBJS)

$(PCAT_PROG): $(PCAT_OBJS)
	$(CC) -o $(PCAT_PROG) $(PCAT_OBJS)
