//------------------------------------------------------------------------------
// EMU86 - 8259 interrupt controller
//------------------------------------------------------------------------------

#include "emu-int.h"
#include "int-elks.h"

int _int_line_max = INT_LINE_MAX;
int _int_prio_max = INT_PRIO_MAX;

// All edge triggered for PC/XT/AT

int _int_mode [INT_LINE_MAX] =
	{ 1, 1, 1, 1, 1, 1, 1, 1};

int _int_prio [INT_LINE_MAX] =
	{ 0, 1, 2, 3, 4, 5, 6, 7};

int _int_vect [INT_LINE_MAX] =
	{ 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

int _int_mask [INT_LINE_MAX] =
	{ 0, 1, 1, 1, 1, 1, 1, 1};  // timer unmasked by default

int _int_req [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0};

int _int_serv [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0};


// PIC I/O write

int int_io_write (word_t p, byte_t b)
	{
	if (p == 0) {
		if (b == 0x20) {  // EOI
			int_end_prio ();
			}
		}

	return 0;
	}


// PIC initialization

void int_init (void)
	{
	}
