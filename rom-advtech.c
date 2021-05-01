//------------------------------------------------------------------------------
// EMU86 - Advantech ROM stub (BIOS)
//------------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-int.h"

#include "int-advtech.h"


// BIOS time services

static int int_1Ah ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Get system time

		case 0x00:
			reg16_set (REG_CX, 0);  // stay on 0
			reg16_set (REG_DX, 0);
			reg8_set (REG_AL, 0);   // no day wrap
			break;

		default:
			printf ("fatal: INT 1Ah: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
	}


// BIOS Ethernet services
// Tweaked by Advantech

static int int_60h ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Ethernet reset

		case 0x00:
			break;

		default:
			printf ("fatal: INT 60h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
	}


// BIOS configuration services
// Advantech specific

static int int_D0h ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// ADVTECH: NIC IO base

		case 0x00:
			reg16_set (REG_AX, 0x300);
			break;

		// ADVTECH: program select (DIP SW) ?

		case 0x01:
			reg16_set (REG_AX, 1);  // program 01 -> MON86  11 -> TEST
			break;

		// No RTC emulated in EMU86

		case 0x02:
			puts ("info: INT D0h AH=02h: reporting no RTC");
			reg16_set (REG_AX, 0);
			break;

		// Get CPU frequency
		// Fixed to 20 MHz

		case 0x03:
			reg16_set (REG_AX, 11520);
			reg16_set (REG_DX, 305);
			break;

		case 0x06:
			reg16_set (REG_AX, 1);  // password reset jumper ON
			break;

		default:
			printf ("fatal: INT D0h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
	}


// BIOS misc services
// Advantech specific

static int int_D2h ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		case 0x00:  // LED
			break;

		case 0x02:  // set D2h_flag
			break;

		default:
			printf ("fatal: INT D2h: AH=%hxh not implemented\n", ah);
			assert (0);

		}

	return 0;
	}


// Interrupt handler table

int_num_hand_t _int_tab [] = {
//	{ 0x03, int_03h },
	{ 0x1A, int_1Ah },
	{ 0x60, int_60h },
	{ 0xD0, int_D0h },
	{ 0xD2, int_D2h },
	{ 0,    NULL    }
	};


// ROM stub (BIOS) initialization

void rom_init (void)
	{
	}
