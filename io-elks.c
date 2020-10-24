//-------------------------------------------------------------------------------
// EMU86 I/O emulation
// ELKS target
//-------------------------------------------------------------------------------

#include "emu-mem-io.h"

//-------------------------------------------------------------------------------

int io_read_byte (word_t p, byte_t * b)
	{
	*b = 0xFF;
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
	else {
		// no port
		err = 0;
		}

	return err;
	}

//-------------------------------------------------------------------------------

