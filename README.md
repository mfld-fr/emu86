WHAT IS THIS ?

EMU86 is yet another 8086/8088/80186/80188 emulator.

That old 16-bit processor architecture is still used today in some devices
(SBC, MCU, SoC, FPGA, etc) that reuse the huge hardware and code base from
the legacy and popular PC/XT/AT platform to build embedded systems.

This emulator is intended to help the related embedded software development,
at the stage of testing / debugging the code on the development system before
burning it into the EEPROM / Flash on the device.

The goal is NOT to reinvent the wheel for PC / end-user emulation, as others
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
- some debugger features

Debugger features:
- silent / trace / interactive modes
- registers / stack dump
- initial code / data breakpoint
- unhandled INT3 breakpoint
- step in / over

Emulated serial port is redirected:

* either to a pseudo-terminal (/dev/pts), with a bidirectional cat utility
(pcat) to be able to redirect serial I/O from / to a file and thus easing
test automation,

* or directly to the emulator console (standard input / output).

A good reference is the '8086tiny' project : https://github.com/adriancable/8086tiny


CURRENT STATUS:

The emulator is now able to execute MON86 and SYS86 code, targetting the
Advantech SNMP-1000 SBC with its specific BIOS and devices (interrupt
controller, timer and serial port).

It also executes partially the original ROM code of the same SBC, enough to
reach the user prompt and to run some basic commands of the embedded
diagnostics program.

It is also used to debug ELKS in a minimal ROM-loaded configuration,
headless and diskless, but with a ROM-disk and a serial console.


WHAT REMAINS TO DO ?

* General:
  - Complete the 8086 instruction set
  - Complete the 80186 instruction set

* For the SBC:
  - More HW emulation of the R8810 MCU
  - Add EEPROM / Flash emulation

* For ELKS:
  - More BIOS support
