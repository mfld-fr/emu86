//-------------------------------------------------------------------------------
// EMU86 - 8018X timers
//-------------------------------------------------------------------------------


#define TIMER0_COUNT   0   // timer 0 count register
#define TIMER0_MAX     1   // timer 0 max A register
#define TIMER0_MODE    3   // timer 0 mode & control register

#define TIMER2_COUNT   8   // timer 2 count register
#define TIMER2_MAX     9   // timer 2 max register
#define TIMER2_MODE    10  // timer 2 mode & control register

static word_t timer_regs [TIMER_REG_COUNT];

#define TIMER_MODE_EN 0x8000  // timer enabled


// Timer device procedure
// Called from main emulator loop

void timer_proc (void)
	{
	int timer2 = 0;

	if (timer_regs [TIMER2_MODE] & TIMER_MODE_EN)
		{
		timer_regs [TIMER2_COUNT]++;
		if (timer_regs [TIMER2_COUNT] == timer_regs [TIMER2_MAX]) {
			timer_regs [TIMER2_COUNT] = 0;
			timer2 = 1;
			}
		}

	if (timer_regs [TIMER0_MODE] & TIMER_MODE_EN)
		{
		if (timer2) timer_regs [TIMER0_COUNT]++;
		if (timer_regs [TIMER0_COUNT] == timer_regs [TIMER0_MAX]) {
			timer_regs [TIMER0_COUNT] = 0;
			int_line_set (INT_LINE_TIMER0, 1);
			}
		else
			{
			int_line_set (INT_LINE_TIMER0, 0);
			}
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
#ifdef MCU_8018X_EB
	timer_regs [TIMER0_MODE] = 0;
	timer_regs [TIMER0_COUNT] = 0;
	timer_regs [TIMER0_MAX] = 0;

	timer_regs [TIMER2_MODE] = 0;
	timer_regs [TIMER2_COUNT] = 0;
	timer_regs [TIMER2_MAX] = 0;
#endif

#ifdef MCU_R8810
	timer_regs [TIMER0_MODE] = TIMER_MODE_EN;
	timer_regs [TIMER0_COUNT] = 0;
	timer_regs [TIMER0_MAX] = 1000;

	timer_regs [TIMER2_MODE] = TIMER_MODE_EN;
	timer_regs [TIMER2_COUNT] = 0;
	timer_regs [TIMER2_MAX] = 5000;
#endif
	}
