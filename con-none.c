//-------------------------------------------------------------------------------
// EMU86 - Console to null mapping
//-------------------------------------------------------------------------------

#include "emu-con.h"   // generic console


int con_put_char (byte_t c, byte_t a)
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

int con_scroll (int dn, byte_t n, byte_t at, byte_t r, byte_t c, byte_t r2, byte_t c2)
	{
	return 0;
	}

int con_get_key (word_t * k)
	{
	*k = 0;
	return 1;  // no key
	}

int con_poll_key ()
	{
	return 0;  // no key
	}

int con_proc ()
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
