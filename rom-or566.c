//------------------------------------------------------------------------------
// EMU86 - Orkit 566 custom ROM
//------------------------------------------------------------------------------

#include "emu-int.h"
#include "emu-mem-io.h"

#include <stdlib.h>


// Interrupt handler table

int_num_hand_t _int_tab [] = {
//	{ 0x03, int_03h },
	{ 0,    NULL    }
	};


// ROM stub initialization

void rom_init (void)
	{
	// ROM boot start @ F000:FF00h
	// ELKS kernel @ E000:0h

	mem_stat [0xFFF00] = 0x9A;  // CALLF E000:0003h
	mem_stat [0xFFF01] = 0x03;
	mem_stat [0xFFF02] = 0x00;
	mem_stat [0xFFF03] = 0x00;
	mem_stat [0xFFF04] = 0xE0;

	mem_stat [0xFFF05] = 0xCD;  // INT 19h
	mem_stat [0xFFF06] = 0x19;

	// CPU boot starts @ FFFF:0h
	// Jump to ROM boot

	mem_stat [0xFFFF0] = 0xEA;  // JMPF F000:FF00h
	mem_stat [0xFFFF1] = 0x00;
	mem_stat [0xFFFF2] = 0xFF;
	mem_stat [0xFFFF3] = 0x00;
	mem_stat [0xFFFF4] = 0xF0;
	}

int rom_image_load (char * path)
	{
	return 1;  // error
	}

void rom_term (void)
	{
	}
