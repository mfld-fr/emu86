//-------------------------------------------------------------------------------
// EMU86 - Orkit 566 board I/O mapping (only 8018X for now)
//-------------------------------------------------------------------------------

#include "emu-mem-io.h"

#include "int-8018x.h"
#include "timer-8018x.h"
#include "serial-8018x.h"


int io_read_byte (word_t p, byte_t * b)
	{
	return io_read_byte_0 (p, b);
	}


int io_write_byte (word_t p, byte_t b)
	{
	int err = 0;

	if ((p >= SERIAL_REG_BASE) && (p < (SERIAL_REG_BASE + SERIAL_REG_SIZE)))
		{
		err = serial_io_write (p - SERIAL_REG_BASE, b);
		}

	else
		{
		// Default
		err = io_write_byte_0 (p, b);
		}

	return err;
	}


int io_read_word (word_t p, word_t * w)
	{
	int err = 0;

	if ((p >= SERIAL_REG_BASE) && (p < (SERIAL_REG_BASE + SERIAL_REG_SIZE)))
		{
		err = serial_io_read (p - SERIAL_REG_BASE, w);
		}

	else
		{
		// Default
		err = io_read_word_0 (p, w);
		}

	return err;
	}


int io_write_word (word_t p, word_t w)
	{
	int err = 0;

	if ((p >= INT_REG_BASE) && (p < (INT_REG_BASE + INT_REG_SIZE)))
		{
		err = int_io_write (p - INT_REG_BASE, w);
		}

	else if ((p >= TIMER_REG_BASE) && (p < (TIMER_REG_BASE + TIMER_REG_SIZE)))
		{
		err = timer_io_write (p - TIMER_REG_BASE, w);
		}

	else
		{
		// Default
		err = io_write_word_0 (p, w);
		}

	return err;
	}

//-------------------------------------------------------------------------------
