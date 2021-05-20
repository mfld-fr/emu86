//------------------------------------------------------------------------------
// EMU86 - 8018X interrupt controller
//------------------------------------------------------------------------------

#include "emu-int.h"

#include "int-8018x.h"

#include <stdio.h>


#define INT_REG_EOI  0
#define INT_REG_MASK 3

int _int_line_max = INT_LINE_MAX;
int _int_prio_max = INT_PRIO_MAX;

// Timer & serial are edge triggered

int _int_mode [INT_LINE_MAX] =
	{ 1, 0, 1, 0, 0, 0, 0, 0};

int _int_prio [INT_LINE_MAX] =
	{ 0, 1, 2, 3, 4, 5, 6, 7};

int _int_vect [INT_LINE_MAX] =
	{ 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

int _int_mask [INT_LINE_MAX] =
	{ 1, 1, 1, 1, 1, 1, 1, 1};

int _int_req [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0};

int _int_serv [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0};


// PIC I/O write

int int_io_write (word_t p, word_t w)
	{
	int err = 0;

	int r = p >> 1;

	if (r == INT_REG_EOI)
		{
		if ((w & 0x001F) == 0x0008) int_end_line (INT_LINE_TIMER0);
		if ((w & 0x001F) == 0x0014) int_end_line (INT_LINE_SERIAL);
		}

	else if (r == INT_REG_MASK)
		{
		_int_mask [INT_LINE_TIMER0] = w & 0x0001;
		_int_mask [INT_LINE_SERIAL] = w & 0x0004;
		}

	else
		{
		printf ("\nerror: write %hXh to int register %i", w, r);
		err = -1;
		}

	return err;
	}


// PIC initialization

void int_init (void)
	{
	}
