//-------------------------------------------------------------------------------
// EMU86 - Console to null mapping
//-------------------------------------------------------------------------------

#include "emu-con.h"   // generic console


int con_put_char (byte_t c)
	{
	return 0;
	}

int con_pos_set (byte_t row, byte_t col)
	{
	return 0;
	}

int con_pos_get (byte_t *row, byte_t *col)
	{
	*row = *col = 0;
	return 0;
	}

int con_scrollup ()
	{
	return 0;
	}

int con_get_key (word_t * k)
	{
	*k = 0;
	return 0;
	}

int con_poll_key ()
	{
	return 0;
	}

int con_update ()
	{
	return 0;
	}

void con_raw ()
	{
	}

void con_normal ()
	{
	}

int con_init ()
	{
	return 0;
	}

void con_term ()
	{
	}
