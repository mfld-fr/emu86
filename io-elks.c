//-------------------------------------------------------------------------------
// EMU86 I/O emulation
// ELKS target
//-------------------------------------------------------------------------------

#include <stdio.h>
#include "emu-mem-io.h"
#include "mem-io-elks.h"
#include "int-elks.h"
#include "timer-elks.h"

extern int info_level;

byte_t crtc_curhi, crtc_curlo;		// 6845 CRTC cursor
static byte_t crtc_lastcommand;

//-------------------------------------------------------------------------------

int io_read_byte (word_t p, byte_t * b)
	{
	switch (p)
		{
		case 0x1F7:		// HD1 status
		case 0x177:		// HD2 status
			*b = 0x7f;	// ready, drive not found
			break;
		default:
			*b = 0xFF;
			break;
		}
	if (info_level & 4) printf("[ INB %3xh AL %02xh]\n", p, *b);
	return 0;
	}

int io_write_byte (word_t p, byte_t b)
	{
	switch (p)
		{
		case 0x20:  // 8259 PIC
			int_io_write (p - 0x20, b);
			break;

		case TIMER_CTRL_PORT:		// 8253/8254 timer
			timer_io_write (p, b);
			break;

		case 0x80:					// I/O delay
			break;

		case CRTC_CTRL_PORT:		// 6845 CRTC
			crtc_lastcommand = b;
			break;

		case CRTC_DATA_PORT:
			if (crtc_lastcommand == 0x0E)
				crtc_curhi = b;
			else if (crtc_lastcommand == 0x0F)
				crtc_curlo = b;
			// set display page 0x0C/0x0D ignored
			break;

		default:
			if (info_level & 4) printf("[OUTB %3xh AL %0xh]\n", p, b);
		}
	return 0;
	}

//-------------------------------------------------------------------------------

int io_read_word (word_t p, word_t * w)
	{
	int err;

	if (p & 0x0001) {
		// bad alignment
		err = -1;
		}
	else {
		// no port
		if (info_level & 4) printf("[ INW %3xh AX %04xh]\n", p, *w);
		*w = 0xFFFF;
		err = 0;
		}

	return err;
	}

int io_write_word (word_t p, word_t w)
	{
	int err;

	if (p & 0x0001) {
		// bad alignment
		err = -1;
		}
	else {
		// no port
		if (info_level & 4) printf("[OUTW %3xh AX %0xh]\n", p, w);
		err = 0;
		}

	return err;
	}

//-------------------------------------------------------------------------------

