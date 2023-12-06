//-------------------------------------------------------------------------------
// EMU86 - 8018X serial port
//-------------------------------------------------------------------------------

#include "emu-serial.h"
#include "emu-int.h"

#include "int-8018x.h"
#include "serial-8018x.h"

#include <stdio.h>


#define SERIAL_STAT   0  // status register
#define SERIAL_RDAT   1  // receive data register
#define SERIAL_TDAT   2  // transmit data register

static word_t serial_regs [SERIAL_REG_COUNT];

#define SERIAL_STAT_RINT  0x0040  // receiver interrupt
#define SERIAL_STAT_TRDY  0x0008  // transmitter ready


// Set the serial interrupt line

void serial_int (void)
	{
	int stat = 0;

	/*
	if ((serial_regs [SERIAL_STAT] & SERIAL_STAT_RINT)
		&&  (serial_regs [SERIAL_MODE] & SERIAL_CONTROL_RXIE))
		stat = 1;

	if ((serial_regs [SERIAL_STAT] & SERIAL_STATUS_THRE)
		&& (serial_regs [SERIAL_MODE] & SERIAL_CONTROL_TXIE))
		stat = 1;
	*/

	int_line_set (INT_LINE_SERIAL_RX, stat);
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

			if (serial_regs [SERIAL_STAT] & SERIAL_STAT_RINT)
				{
				puts ("\nwarning: serial port input overflow");
				// TODO: set overflow flag in status register
				}

			serial_regs [SERIAL_RDAT] = (word_t) c;
			serial_regs [SERIAL_STAT] |= SERIAL_STAT_RINT;

			serial_int ();
			break;
			}

		if (err) break;

		// Output stream

		while (1) {
			if (serial_regs [SERIAL_STAT] & SERIAL_STAT_TRDY) break;

			err = serial_send ((byte_t) serial_regs [SERIAL_TDAT]);
			if (err) break;

			serial_regs [SERIAL_STAT] |= SERIAL_STAT_TRDY;
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

	if (r == SERIAL_RDAT) {
		serial_regs [SERIAL_STAT] &= ~SERIAL_STAT_RINT;
		serial_int ();
		}

	return 0;
	}


// Serial I/O write

int serial_io_write (word_t p, word_t  w)
	{
	int r = p >> 1;
	serial_regs [r] = w;

	if (r == SERIAL_TDAT) {
		serial_regs [SERIAL_STAT] &= ~(SERIAL_STAT_TRDY);
		serial_int ();
		}

	return 0;
	}


// Serial device initialization

void serial_dev_init ()
	{
	// Initialize registers

	//serial_regs [SERIAL_MODE] = SERIAL_CONTROL_RXIE;
	serial_regs [SERIAL_STAT] = SERIAL_STAT_TRDY;
	}


void serial_dev_term ()
	{
	}
