//-------------------------------------------------------------------------------
// EMU86 - Console to character backend mapping
//-------------------------------------------------------------------------------

#include "emu-con.h"   // generic console
#include "emu-char.h"  // generic character backend


int con_put_char (word_t c)
	{
	return char_send (c);
	}


static byte_t col_prev = 0;

int con_pos_set (byte_t row, byte_t col)
	{
	// Detect new line as return to first column

	if (col == 0 && col_prev != 0)
		{
		char_send (10);  // LF
		}

	col_prev = col;
	return 0;
	}

int con_pos_get (byte_t *row, byte_t *col)
	{
	*row = *col = 0;
	return 0;
	}

int con_scrollup (byte_t n, byte_t at, byte_t r, byte_t c, byte_t r2, byte_t c2)
	{
	return 0;
	}


int con_proc ()
	{
	int err;

	while (1)
		{
		err = char_poll ();
		if (err <= 0) break;

		byte_t c = 0;
		err = char_recv (&c);

		if (!err) con_put_key ((word_t) c);
		break;
		}

	return err;
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
	con_init_key ();
	return char_init ();
	}


void con_term ()
	{
	char_term ();
	}

//-------------------------------------------------------------------------------
