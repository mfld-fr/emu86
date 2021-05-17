//------------------------------------------------------------------------------
// EMU86 - Advantech ROM stub (BIOS)
//------------------------------------------------------------------------------

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-int.h"

#include "int-r8810.h"

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
			reg16_set (REG_AX, 1);
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


// BIOS boot sequence starts @ F000:0h

static addr_t bios_boot = 0xF0000;

// BIOS extension check

static void bios_check (addr_t b, int size)
	{
	byte_t sum = 0;

	for (addr_t a = b; a < (b + size); a++)
		sum += mem_read_byte (a);

	if (sum == 0)
		{
		// Append a far call to any detected extension to the BIOS boot sequence

		mem_write_byte (bios_boot, 0x9A, 1);  // CALLF
		mem_write_word (bios_boot + 1, 0x0003, 1);
		mem_write_word (bios_boot + 3, b >> 4, 1);
		bios_boot += 5;
		}
	}

// ROM stub (BIOS) initialization

void rom_init (void)
	{
	// Any extension @ E000:0h must be 64K

	if (mem_read_word (0xE0000) == 0xAA55)
		{
		bios_check (0xE0000, 0x10000);
		}

	if (bios_boot > 0xF0000)
		{
		// End BIOS boot initialization sequence with INT 19h for OS boot

		mem_write_byte (bios_boot++, 0xCD, 1);  // INT 19h
		mem_write_byte (bios_boot++, 0x19, 1);

		// CPU boot starts @ FFFF:0h
		// Jump to BIOS boot

		mem_write_byte (0xFFFF0, 0xEA,   1);  // JMPF
		mem_write_word (0xFFFF1, 0x0000, 1);
		mem_write_word (0xFFFF3, 0xF000, 1);
		}
	}

int rom_image_load (char * path)
	{
	return 1;  // error
	}

void rom_term (void)
	{
	}
