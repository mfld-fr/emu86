//------------------------------------------------------------------------------
// EMU86 - Advantech ROM stub (BIOS)
//------------------------------------------------------------------------------

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-int.h"

#include "int-advtech.h"

#include <stdlib.h>
#include <stdio.h>


// BIOS time services
// Hook to avoid using the RTC chip (not emulated yet)
// and because DAA / DAS are not implemented yet

static int int_1Ah ()
	{
	int err = 0;

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
			printf ("\nerror: INT 1Ah AH=%hhXh not implemented\n", ah);
			err = -1;
			break;

		}

	return err;
	}


// BIOS Ethernet services
// Tweaked by Advantech

static int int_60h ()
	{
	int err = 0;

	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Ethernet reset

		case 0x00:
			break;

		default:
			printf ("\nerror: INT 60h AH=%hhXh not implemented\n", ah);
			err = -1;
		}

	return err;
	}


// BIOS configuration services
// Advantech specific

static int int_D0h ()
	{
	int err = 0;

	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// ADVTECH: NIC I/O base

		case 0x00:
			reg16_set (REG_AX, 0x300);
			break;

		// ROM program select (DIP SW)
		// 00b: PROG0
		// 01b: PROG1 or MON86
		// 10b: PROG2 = PROG0
		// 11b: ROM TEST

		case 0x01:
			reg16_set (REG_AX, 3);
			break;

		// No RTC emulated in EMU86

		case 0x02:
			puts ("\ninfo: INT D0h AH=02h: reporting no RTC");
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
			printf ("\nerror: INT D0h AH=%hhXh not implemented\n", ah);
			err = -1;
		}

	return err;
	}


// Interrupt handler table

int_num_hand_t _int_tab [] = {
//	{ 0x03, int_03h },
	{ 0x1A, int_1Ah },
	{ 0x60, int_60h },
	{ 0xD0, int_D0h },
	{ 0,    NULL    }
	};


// ROM stub (BIOS) initialization

void rom_init (void)
	{
	}

int rom_image_load (char * path)
	{
	return 1;  // error
	}

void rom_term (void)
	{
	}
