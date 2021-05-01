//-------------------------------------------------------------------------------
// EMU86 - Character console
//-------------------------------------------------------------------------------

#include "emu-con.h"   // generic console
#include "emu-char.h"  // generic character backend


int con_put_char (byte_t c)
	{
	return char_send (c);
	}


static byte_t col_prev = 0;

int con_pos_set (byte_t row, byte_t col)
	{
	// Detect new line as return to first column

	if (col == 0 && col_prev != 0)
		{
		// char_send (13);  // CR
		char_send (10);  // LF
		}

	col_prev = col;
	return 0;
	}


int con_get_key (word_t * k)
	{
	byte_t c = 0;
	int err = char_recv (&c);
	*k = (word_t) c;
	return err;
	}


int con_poll_key ()
	{
	return char_poll ();
	}


void con_raw ()
	{
	char_raw ();
	}

void con_normal ()
	{
	char_normal ();
	}


int con_init ()
	{
	return char_init ();
	}


void con_term ()
	{
	char_term ();
	}
