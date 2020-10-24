
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-int.h"


// BIOS video services

static byte_t col_prev = 0;

static byte_t num_hex (byte_t n)
	{
	byte_t h = n + '0';
	if (h > '9') h += 'A' - '9';
	return h;
	}

static int int_10h ()
	{
	byte_t ah = reg8_get (REG_AH);

	byte_t al;

	word_t es;
	word_t bp;
	word_t cx;

	addr_t a;
	byte_t c;

	switch (ah)
		{
		// Set cursor position
		// Detect new line as return to first column

		case 0x02:
			c = reg8_get (REG_DL); // column
			if (c == 0 && col_prev != 0)
				{
				//serial_send (13);  // CR
				serial_send (10);  // LF
				}

			col_prev = c;
			break;

		// Get cursor position

		case 0x03:
			reg16_set (REG_CX, 0);  // null cursor
			reg16_set (REG_DX, 0);  // upper-left corner (0,0)
			break;

		// Select active page

		case 0x05:
			break;

		// Scroll up

		case 0x06:
			break;

		// Write character at current cursor position

		case 0x09:
		case 0x0A:
			serial_send (reg8_get (REG_AL));  // Redirect to serial port
			break;

		// Write as teletype to current page
		// Page ignored in video mode 7

		case 0x0E:
			serial_send (reg8_get (REG_AL));  // Redirect to serial port
			break;

		// Get video mode

		case 0x0F:
			reg8_set (REG_AL, 7);   // text monochrome 80x25
			reg8_set (REG_AH, 80);  // 80 columns
			reg8_set (REG_BH, 0);   // page 0 active
			break;

		// Write string

		case 0x13:
			es = seg_get (SEG_ES);
			bp = reg16_get (REG_BP);
			cx = reg16_get (REG_CX);
			a = addr_seg_off (es, bp);

			while (cx--)
				{
				serial_send (mem_read_byte (a++));  // Redirect to serial port
				}

			break;

		// Write byte as hexadecimal

		case 0x1D:
			al = reg8_get (REG_AL);
			c = num_hex (al >> 4);
			serial_send (c);  // Redirect to serial port
			c = num_hex (al & 0x0F);
			serial_send (c);  // Redirect to serial port
			break;

		default:
			printf ("fatal: INT 10h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
	}


// BIOS memory services

static int int_12h ()
	{
	// 640 KiB of low memory
	// no extended memory

	reg16_set (REG_AX, 512);
	return 0;
	}


// BIOS misc services

static int int_15h ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Return CF=1 for all non implemented functions
		// as recommended by Alan Cox on the ELKS mailing list

		default:
			flag_set (FLAG_CF, 1);

		}

	return 0;
	}


// BIOS keyboard services

static byte_t key_prev = 0;

static int int_16h ()
	{
	int err = 0;
	byte_t c = 0;

	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Normal keyboard read

		case 0x00:
			if (key_prev)
				{
				reg8_set (REG_AL, key_prev);
				key_prev = 0;
				}
			else
				{
				c = serial_recv ();
				if (c == 0xFF)  // error
					{
					err = -1;
					break;
					}

				reg8_set (REG_AL, (byte_t) c);  // ASCII code
				}

			reg8_set (REG_AH, 0);  // No scan code
			break;

		// Peek character

		case 0x01:
			if (serial_poll ())
				{
				flag_set (FLAG_ZF, 0);
				key_prev = serial_recv ();
				if (key_prev == 0xFF)  // error
					{
					err = -1;
					break;
					}

				reg8_set (REG_AL, key_prev);
				reg8_set (REG_AH, 0);
				}
			else
				{
				flag_set (FLAG_ZF, 1);  // no character in buffer
				}

			break;

		// Set typematic rate - ignore

		case 0x03:
			break;

		// Extended keyboard read

		case 0x10:
			c = serial_recv ();
			if (c == 0xFF)  // error
				{
				err = -1;
				break;
				}

			reg8_set (REG_AL, (byte_t) c);  // ASCII code
			reg8_set (REG_AH, 0);           // No scan code
			break;

		default:
			printf ("fatal: INT 16h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return err;
	}


// BIOS printer services

static int int_17h ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		default:
			printf ("fatal: INT 17h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
	}


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


// Interrupt handler table

int_num_hand_t _int_tab [] = {
	{ 0x03, int_03h },
	{ 0x10, int_10h },
	{ 0x12, int_12h },
	{ 0x15, int_15h },
	{ 0x16, int_16h },
	{ 0x17, int_17h },
	{ 0x1A, int_1Ah },
	{ 0,    NULL    }
	};


// Interrupt initialization

void int_init (void)
	{
	// ELKS saves and calls initial INT8 (timer)
	// So implement a stub for INT8 at startup

	mem_write_byte (0xFFFF8, 0xCF, 1);  // IRET @ FFFF:8h
	mem_write_word (0x00020, 0x0008, 1);
	mem_write_word (0x00022, 0xFFFF, 1);
	}

