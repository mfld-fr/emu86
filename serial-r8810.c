//-------------------------------------------------------------------------------
// EMU86 - R8810 serial port
//-------------------------------------------------------------------------------

#include "emu-serial.h"
#include "emu-int.h"

#include "int-r8810.h"

#include <stdio.h>


#define SERIAL_MODE   0
#define SERIAL_STAT   1
#define SERIAL_TDATA  2
#define SERIAL_RDATA  3
#define SERIAL_RATE   4

static word_t serial_regs [5];

#define SERIAL_MODE_TXIE  0x0800
#define SERIAL_MODE_RXIE  0x0400

#define SERIAL_STAT_TEMT  0x0040
#define SERIAL_STAT_THRE  0x0020
#define SERIAL_STAT_RDR   0x0010


// Set the serial interrupt line

void serial_int (void)
	{
	int stat = 0;

	if ((serial_regs [SERIAL_STAT] & SERIAL_STAT_RDR)
		&&  (serial_regs [SERIAL_MODE] & SERIAL_MODE_RXIE))
		stat = 1;

	if ((serial_regs [SERIAL_STAT] & SERIAL_STAT_THRE)
		&& (serial_regs [SERIAL_MODE] & SERIAL_MODE_TXIE))
		stat = 1;

	int_line_set (INT_LINE_SERIAL, stat);
	}


// Serial device procedure
// Called from main emulator loop

static int loop_count = 0;
#define LOOP_MAX 5000

int serial_proc (void)
	{
	int err = 0;

	while (1) {
		// Input stream

		while (1)
			{
			// Moderate the poll rate
			// for more instruction processing

			loop_count++;
			if (loop_count < LOOP_MAX) break;
			loop_count = 0;

			int res = serial_poll ();
			if (!res) break;

			byte_t c;
			err = serial_recv (&c);
			if (err) break;

			// Overflow detection

			if (serial_regs [SERIAL_STAT] & SERIAL_STAT_RDR)
				{
				puts ("\nwarning: serial port input overflow");
				// TODO: set overflow flag in status register
				}

			serial_regs [SERIAL_RDATA] = (word_t) c;
			serial_regs [SERIAL_STAT] |= SERIAL_STAT_RDR;

			serial_int ();
			break;
			}

		if (err) break;

		// Output stream

		while (1) {
			if (serial_regs [SERIAL_STAT] & SERIAL_STAT_THRE) break;

			err = serial_send ((byte_t) serial_regs [SERIAL_TDATA]);
			if (err) break;

			serial_regs [SERIAL_STAT] |= SERIAL_STAT_TEMT | SERIAL_STAT_THRE;
			serial_int ();
			break;
			}

		break;
		}

	return err;
	}


// Serial I/O read

int serial_io_read (word_t p, word_t * w)
	{
	int r = p >> 1;
	*w = serial_regs [r];

	if (r == SERIAL_RDATA) {
		serial_regs [SERIAL_STAT] &= ~SERIAL_STAT_RDR;
		serial_int ();
		}

	return 0;
	}


// Serial I/O write

int serial_io_write (word_t p, word_t  w)
	{
	int r = p >> 1;
	serial_regs [r] = w;

	if (r == SERIAL_TDATA) {
		serial_regs [SERIAL_STAT] &= ~(SERIAL_STAT_TEMT | SERIAL_STAT_THRE);
		serial_int ();
		}

	return 0;
	}


// Serial device initialization

void serial_dev_init ()
	{
	// Initialize registers

	serial_regs [SERIAL_MODE] = SERIAL_MODE_RXIE;
	serial_regs [SERIAL_STAT] = SERIAL_STAT_TEMT | SERIAL_STAT_THRE;
	}


void serial_dev_term ()
	{
	}
