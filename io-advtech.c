//-------------------------------------------------------------------------------
// EMU86 - Advantech board I/O mapping
//-------------------------------------------------------------------------------

#include "int-r8810.h"
#include "serial-r8810.h"
#include "timer-8018x.h"

//-------------------------------------------------------------------------------

int io_read_byte (word_t p, byte_t * b)
	{
	*b = 0xFF;

	switch (p)
		{
		case 0x0065:
			*b = 0x00;   // needed to exit loop @ F000h:11B7h
			break;

		}

	return 0;
	}

int io_write_byte (word_t p, byte_t b)
	{
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
	else if ((0xFF20 <= p) && (p < 0xFF46)) {
		//err = int_io_read (p - 0xFF20, w);
		*w = 0xFFFF;
		err = 0;
		}
	else if ((TIMER_REG_BASE <= p) && (p < TIMER_REG_BASE + TIMER_REG_SIZE)) {
		err = timer_io_read (p - TIMER_REG_BASE, w);
		}
	else if ((0xFF80 <= p) && (p < 0xFF8A)) {
		err = serial_io_read (p - 0xFF80, w);
		}
	else {
		// no port
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
	else if ((0xFF20 <= p) && (p < 0xFF46)) {
		err = int_io_write (p - 0xFF20, w);
		}
	else if ((0xFF50 <= p) && (p < 0xFF68)) {
		err = timer_io_write (p - 0xFF50, w);
		}
	else if ((0xFF80 <= p) && (p < 0xFF8A)) {
		err = serial_io_write (p - 0xFF80, w);
		}
	else {
		// no port
		err = 0;
		}

	return err;
	}

//-------------------------------------------------------------------------------

