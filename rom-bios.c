// Common BIOS code between PC/XT/AT and ELKS targets

#include "emu-mem-io.h"
#include "rom-bios.h"


void rom_init_0 (void)
	{
	// ELKS saves and calls initial INT 08h (timer)
	// So implement a stub for INT 08h that does EOI
	// Starting @ F000:1000h

	// MS-DOS also samples the BIOS timer at boot
	// So increment that timer (only the low part)

	mem_stat [BDA_TIMER_LO + 0] = 0;
	mem_stat [BDA_TIMER_LO + 1] = 0;
	mem_stat [BDA_TIMER_HI + 0] = 0;
	mem_stat [BDA_TIMER_HI + 1] = 0;
	mem_stat [BDA_TIMER_DAY] = 0;  // no day wrap

	mem_stat [0xF1000] = 0x1E;  // PUSH DS
	mem_stat [0xF1001] = 0x50;  // PUSH AX
	mem_stat [0xF1002] = 0x33;  // XOR AX,AX
	mem_stat [0xF1003] = 0xC0;
	mem_stat [0xF1004] = 0x8E;  // MOV DS,AX
	mem_stat [0xF1005] = 0xD8;
	mem_stat [0xF1006] = 0xFF;  // INC WORD [046Ch] = [BDA_TIMER_LO]
	mem_stat [0xF1007] = 0x06;
	mem_stat [0xF1008] = 0x6C;
	mem_stat [0xF1009] = 0x04;
	mem_stat [0xF100A] = 0xB0;  // MOV AL,20h
	mem_stat [0xF100B] = 0x20;
	mem_stat [0xF100C] = 0xE6;  // OUT 20h,AL
	mem_stat [0xF100D] = 0x20;
	mem_stat [0xF100E] = 0x58;  // POP AX
	mem_stat [0xF100F] = 0x1F;  // POP DS
	mem_stat [0xF1010] = 0xCF;  // IRET

	// Point vector 08h to that stub

	mem_stat [0x08 * 4 + 0] = 0x00;
	mem_stat [0x08 * 4 + 1] = 0x10;
	mem_stat [0x08 * 4 + 2] = 0x00;
	mem_stat [0x08 * 4 + 3] = 0xF0;
	}


void rom_term_0 (void)
	{
	}
