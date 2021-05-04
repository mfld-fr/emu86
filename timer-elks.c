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

void timer_init ()
	{
	}
