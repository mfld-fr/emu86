
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-int.h"

#include "int-advtech.h"

//------------------------------------------------------------------------------
// Interrupt controller
//------------------------------------------------------------------------------

#define INT_REG_VECT 0
#define INT_REG_EOI  1

int _int_line_max = INT_LINE_MAX;
int _int_prio_max = INT_PRIO_MAX;

int _int_line [INT_LINE_MAX];

int _int_prio [INT_LINE_MAX] =
	{ 0, 7, 1, 2, 3, 4, 5, 6, 7, 7, 0, 0, 7};

int _int_vect [INT_LINE_MAX] =
	{ 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14 };

int _int_mask [INT_LINE_MAX] =
	{ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};  // FIXME: unmask timer & serial by program

int _int_req [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int _int_serv [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


// Interrupt I/O write

int int_io_write (word_t p, word_t w)
	{
	int r = p >> 1;

	if (r == INT_REG_EOI) {
		if ((w & 0x001F) == 0x0008) int_end (INT_LINE_TIMER0);
		if ((w & 0x001F) == 0x0014) int_end (INT_LINE_SERIAL);
		}

	return 0;
	}

//------------------------------------------------------------------------------
// Interrupt handlers
//------------------------------------------------------------------------------

// BIOS video services
// Tweaked by Advantech
// Redirected to serial port

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
		// Print character

		case 0x0A:
			serial_send (reg8_get (REG_AL));
			break;

		// Print string

		case 0x13:
			es = seg_get (SEG_ES);
			bp = reg16_get (REG_BP);
			cx = reg16_get (REG_CX);
			a = addr_seg_off (es, bp);

			while (cx--)
				{
				serial_send (mem_read_byte (a++));
				}

			break;

		// Write byte as hexadecimal

		case 0x1D:
			al = reg8_get (REG_AL);
			c = num_hex (al >> 4);
			serial_send (c);
			c = num_hex (al & 0x0F);
			serial_send (c);
			break;

		default:
			printf ("fatal: INT 10h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
	}

// BIOS keyboard services
// Tweaked by Advantech
// Redirected to serial port

static int int_16h ()
	{
	int err = 0;
	byte_t c = 0;

	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Keyboard initialization
		// Do not intercept BIOS

		case 0x00:
			err = 1;
			break;

		// Read next char (blocking)

		case 0x10:
			err = serial_recv (&c);
			if (err) break;

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
// Tweaked by Advantech
// Redirected to serial port

static int int_17h ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Serial reset

		case 0x06:
			break;

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
			reg16_set (REG_AX, 3);  // program 01 -> MON86  11 -> TEST
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
	{ 0x03, int_03h },
	{ 0x10, int_10h },
	{ 0x16, int_16h },
	{ 0x17, int_17h },
	{ 0x1A, int_1Ah },
	{ 0x60, int_60h },
	{ 0xD0, int_D0h },
	{ 0xD2, int_D2h },
	{ 0,    NULL    }
	};


// Interrupt initialization

void int_init (void)
	{
	}
