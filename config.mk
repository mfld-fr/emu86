# EMU86 configuration

# Target selection
# pcxtat:  legacy PC/XT/AT
# elks:    minimal PC to run ELKS
# advtech: Advantech SNMP-1000 SBC
# or556:   Orkitt 556

TARGET=pcxtat
#TARGET=elks
#TARGET=advtech
#TARGET=or556

# Host selection
# native:     run on native host
# emscripten: run in web browser

HOST=native
#HOST=emscripten

# Console backend
# none:  no console backend
# stdio: character console - EMU86 stdin & stdout
# pty:   character console - PTY (created by EMU86 as master)
# sdl:   graphical console - SDL window

# NOTE: console is forced to SDL for emscripten

#CONSOLE=none
CONSOLE=stdio
#CONSOLE=pty
#CONSOLE=sdl

# Serial backend
# none:  no serial backend
# stdio: EMU86 stdin & stdout
# pty:   PTY (created by EMU86 as master)

# WARNING: cannot use stdio or pty backend if any already used by console

SERIAL=none
#SERIAL=stdio
#SERIAL=pty

# Disassembly style
# att:   AT&T syntax (GNU default)
# intel: Intel syntax

STYLE=att
#STYLE=intel
