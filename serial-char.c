//-------------------------------------------------------------------------------
// EMU86 - Serial port to character backend mapping
//-------------------------------------------------------------------------------

#include "emu-serial.h"
#include "emu-char.h"

int serial_init ()
	{
	int err = char_init ();
	if (err) return err;
	serial_dev_init ();
	return 0;
	}

void serial_term ()
	{
	serial_dev_term ();
	char_term ();
	}

int serial_poll ()
	{
	return char_poll ();
	}

int serial_recv (byte_t * c)
	{
	return char_recv (c);
	}

int serial_send (byte_t c)
	{
	return char_send (c);
	}
