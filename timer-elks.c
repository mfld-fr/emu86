//------------------------------------------------------------------------------
// EMU86 - PC/XT/AT timer
//------------------------------------------------------------------------------

#include "emu-int.h"
#include "emu-timer.h"

#include "int-elks.h"

#define TIMER_MAX 3000

int timer_enabled = 0;
static int timer_count = 0;

void timer_proc ()
	{
	if (timer_enabled && ++timer_count >= TIMER_MAX)
		{
		int_line_set (INT_LINE_TIMER, 1);
		timer_count = 0;
		}
	else
		{
		int_line_set (INT_LINE_TIMER, 0);
		}
	}


void timer_io_write (word_t p, byte_t b)
	{
	if (p == 0x43) {
		timer_enabled = 0;
		if (b == 0x34)
			timer_enabled = 1; // timer 0, binary count, mode 2
		}
	}


void timer_init ()
	{
	}
