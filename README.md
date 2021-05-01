# EMU86 - IA16 emulator

## PURPOSE

EMU86 is yet another 8086/8088/80186/80188 emulator.

That old 16 bits processor architecture ('IA16') is still used today in some devices
(SBC, MCU, SoC, FPGA, etc.) that reuse the huge hardware and code base from
the legacy and popular PC/XT/AT platform to build embedded systems.

This emulator is intended to help the related embedded software development,
at the stage of testing / debugging the code on the development system before
burning it into the EEPROM / Flash on the device.

The goal is **NOT** to reinvent the wheel for PC / end-user emulation, as others
projects like QEMU, DOSEMU, Bochs, etc., already do that job very well.

Therefore some features like the 80286 protected mode and the 32/64 bits
modes are out of this project scope.

The emulator has a modular design with:
- instruction decoding
- instruction printing
- processor context
- instruction execution
- memory & I/O access handler for peripheral stubbing
- interrupt handler for firmware (BIOS) stubbing
- some debugger features (see below)
- several backends (see below)

Debugger features:
- silent / trace / interactive modes
- registers / stack dump
- initial code / data breakpoints
- unhandled INT3 breakpoint
- step in / over

The emulator provides 3 backends for the console or the serial port devices:

* direct standard input / output in the emulator console,
the most easy to use, but mixed with the debugger input / output,

* a pseudo-terminal (/dev/pts), with a bidirectional cat utility (`pcat`)
that allows to redirect from / to a file and thus easing test automation,

* an SDL2 window (for console only).


## CURRENT STATUS

The emulator is now able to run [ELKS](https://github.com/jbruchon/elks),
also known as 'Linux 8086', in a minimal PC/XT/AT configuration.

It is also able to emulate the Advantech SNMP-1000, an SBC based on the R8810 MCU,
that is a 80188 clone but with with its specific interrupt controller, timer and serial port.

Any addition to support more embbeded systems based on IA16 is welcome.


## CONFIGURE & BUILD

First configure EMU86 by editing `config.mk`:
* TARGET: emulated target / system (ELKS for minimal PC)
* PLATFORM: emulation on host or in web browser (experimental)
* CONSOLE: console backend for PC target
* SERIAL: serial backend
* STYLE: disassembly style in debugger

Two default configurations are provided for convenience:
* `config-elks.mk`
* `config-advtech.mk`

Then run `make clean` and `make` as usual.


## TO DO LIST

* Some IA16 instructions are rarely used and not implemented yet
* More emulation of the 80188 and R8810 MCUs
* Add EEPROM / Flash emulation for complete emulation before burning on real device
* See also the list of GitHub issues


## SEE ALSO

Other projects of interest:
- '8086tiny': https://github.com/adriancable/8086tiny
