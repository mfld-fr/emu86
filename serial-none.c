//-------------------------------------------------------------------------------
// EMU86 - Serial port null mapping
//-------------------------------------------------------------------------------

#include "emu-serial.h"

int serial_init ()
	{
	serial_dev_init ();
	return 0;
	}

void serial_term ()
	{
	serial_dev_term ();
	}

int serial_poll ()
	{
	return 0;
	}

int serial_recv (byte_t * c)
	{
	*c = 0;
	return 0;
	}

int serial_send (byte_t c)
	{
	return 0;
	}
