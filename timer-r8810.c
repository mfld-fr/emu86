//-------------------------------------------------------------------------------
// EMU86 - R8810 timers
//-------------------------------------------------------------------------------

#include "emu-int.h"
#include "emu-timer.h"

#include "int-r8810.h"

//-------------------------------------------------------------------------------

#define TIMER0_REG_COUNT   0
#define TIMER0_REG_MAX     1

#define TIMER2_REG_COUNT   8
#define TIMER2_REG_MAX     9

static word_t timer_regs [12];

//-------------------------------------------------------------------------------

// Timer device procedure
// Called from main emulator loop

void timer_proc (void)
	{
	timer_regs [TIMER2_REG_COUNT]++;
	if (timer_regs [TIMER2_REG_COUNT] == timer_regs [TIMER2_REG_MAX]) {
		timer_regs [TIMER2_REG_COUNT] = 0;
		timer_regs [TIMER0_REG_COUNT]++;
		}

	if (timer_regs [TIMER0_REG_COUNT] == timer_regs [TIMER0_REG_MAX]) {
		timer_regs [TIMER0_REG_COUNT] = 0;
		int_line_set (INT_LINE_TIMER0, 1);
		}
	else
		{
		int_line_set (INT_LINE_TIMER0, 0);
		}
	}

// Timer I/O read

int timer_io_read (word_t p, word_t * w)
	{
	int r = p >> 1;
	*w = timer_regs [r];

	return 0;
	}

// Timer I/O write

int timer_io_write (word_t p, word_t  w)
	{
	int r = p >> 1;
	timer_regs [r] = w;

	return 0;
	}

// Timer initialization

void timer_init ()
	{
	timer_regs [TIMER0_REG_COUNT] = 0;
	timer_regs [TIMER0_REG_MAX] = 1000;

	timer_regs [TIMER2_REG_COUNT] = 0;
	timer_regs [TIMER2_REG_MAX] = 5000;
	}

