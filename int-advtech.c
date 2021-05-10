//------------------------------------------------------------------------------
// EMU86 - R8810 interrupt controller
//------------------------------------------------------------------------------

#include "emu-int.h"
#include "int-advtech.h"

#define INT_REG_VECT 0
#define INT_REG_EOI  1

int _int_line_max = INT_LINE_MAX;
int _int_prio_max = INT_PRIO_MAX;

// Timers are edge triggered

int _int_mode [INT_LINE_MAX] =
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0};

int _int_prio [INT_LINE_MAX] =
	{ 0, 7, 1, 2, 3, 4, 5, 6, 7, 7, 0, 0, 7};

int _int_vect [INT_LINE_MAX] =
	{ 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14 };

int _int_mask [INT_LINE_MAX] =
	{ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};  // FIXME: unmask timer & serial by program

int _int_req [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int _int_serv [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


// PIC I/O write

int int_io_write (word_t p, word_t w)
	{
	int r = p >> 1;

	if (r == INT_REG_EOI) {
		if ((w & 0x001F) == 0x0008) int_end_line (INT_LINE_TIMER0);
		if ((w & 0x001F) == 0x0014) int_end_line (INT_LINE_SERIAL);
		}

	return 0;
	}


// PIC initialization

void int_init (void)
	{
	}
