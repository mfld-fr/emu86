//-------------------------------------------------------------------------------
// EMU86 - 8018X timers
//-------------------------------------------------------------------------------

#include "emu-int.h"
#include "emu-timer.h"

#include "int-8018x.h"
#include "timer-8018x.h"

#include <stdio.h>

#define REG_T0CNT     0   // timer 0 count register
#define REG_T0CMPA    1   // timer 0 max A register
#define REG_T0CMPB    2   // 
#define REG_T0CON     3   // 

#define REG_T1CNT     4   // timer 0 count register
#define REG_T1CMPA    5   // timer 0 max A register
#define REG_T1CMPB    6   // 
#define REG_T1CON     7   // 

#define REG_T2CNT     8   // timer 2 count register
#define REG_T2CMPA    9   // timer 2 max register
#define REG_T2CON     11  // timer 2 mode & control register

static char *regs[] = {
	"CNT",
	"CMPA",
	"CMPB",
	"CON",
};

static word_t timer_regs [TIMER_REG_COUNT];

#define TIMER_CON_CONT 0x0001
#define TIMER_CON_ALT  0x0002
#define TIMER_CON_EXT  0x0004  // not implemented
#define TIMER_CON_P    0x0008
#define TIMER_CON_RTG  0x0010  // not implemented
#define TIMER_CON_MC   0x0020 
#define TIMER_CON_RIU  0x1000 
#define TIMER_CON_INT  0x2000
#define TIMER_CON_INH  0x4000
#define TIMER_CON_EN   0x8000  // timer enabled


// Timer device procedure
// Called from main emulator loop

void timer_proc (void)
{
	int timer2 = 0;
	int timer1 = 0;
	int timer0 = 0;

	if (timer_regs [REG_T2CON] & TIMER_CON_EN) {
		timer_regs [REG_T2CNT]++;
		if (timer_regs [REG_T2CNT] == timer_regs [REG_T2CMPA]) {
			timer_regs [REG_T2CON] |= TIMER_CON_MC;
			timer_regs [REG_T2CNT] = 0;

			if ((timer_regs[REG_T2CON] & TIMER_CON_CONT) == 0) {
				timer_regs [REG_T2CON] &= ~TIMER_CON_EN;
			}

			timer2 = 1;
		}
	}

	if (timer2) {
		int_line_set(INT_LINE_TIMER2, 1);
	} else {
		int_line_set(INT_LINE_TIMER2, 0);
	}

	if (timer_regs [REG_T1CON] & TIMER_CON_EN) {
		if (timer_regs[REG_T1CON] & TIMER_CON_P) {
			if (timer2) timer_regs [REG_T1CNT]++;
		} else {
			timer_regs [REG_T1CNT]++;
		}

		if ((timer_regs[REG_T1CON] & TIMER_CON_ALT) == 0) {
			if (timer_regs [REG_T1CNT] == timer_regs [REG_T1CMPA]) {
				timer_regs [REG_T1CON] |= TIMER_CON_MC;
				timer_regs [REG_T1CNT] = 0;
				timer1 = 1;
				if ((timer_regs[REG_T1CON] & TIMER_CON_CONT) == 0) {
					timer_regs [REG_T1CON] &= ~TIMER_CON_EN;
				}
			}
		} else {
			if ((timer_regs[REG_T1CON] & TIMER_CON_RIU) == 0) {
				if (timer_regs [REG_T1CNT] == timer_regs [REG_T1CMPA]) {
					timer_regs[REG_T1CON] |= TIMER_CON_RIU;
					timer1 = 1;
				}
			} else {
				if (timer_regs [REG_T1CNT] == timer_regs [REG_T1CMPB]) {
					timer_regs[REG_T1CON] &= ~TIMER_CON_RIU;
					timer1 = 1;
					if ((timer_regs[REG_T1CON] & TIMER_CON_CONT) == 0) {
						timer_regs [REG_T1CON] &= ~TIMER_CON_EN;
					}
				}
			}
		}
	}

	if (timer1) {
		int_line_set (INT_LINE_TIMER1, 1);
	} else{
		int_line_set (INT_LINE_TIMER1, 0);
	}

	if (timer_regs [REG_T0CON] & TIMER_CON_EN) {
		if (timer_regs[REG_T0CON] & TIMER_CON_P) {
			if (timer2) timer_regs [REG_T0CNT]++;
		} else {
			timer_regs [REG_T0CNT]++;
		}

		if ((timer_regs[REG_T0CON] & TIMER_CON_ALT) == 0) {
			if (timer_regs [REG_T0CNT] == timer_regs [REG_T0CMPA]) {
				timer_regs [REG_T0CON] |= TIMER_CON_MC;
				timer_regs [REG_T0CNT] = 0;
				timer1 = 1;
				if ((timer_regs[REG_T0CON] & TIMER_CON_CONT) == 0) {
					timer_regs [REG_T0CON] &= ~TIMER_CON_EN;
				}
			}
		} else {
			if ((timer_regs[REG_T0CON] & TIMER_CON_RIU) == 0) {
				if (timer_regs [REG_T0CNT] == timer_regs [REG_T0CMPA]) {
					timer_regs[REG_T0CON] |= TIMER_CON_RIU;
					timer1 = 1;
				}
			} else {
				if (timer_regs [REG_T0CNT] == timer_regs [REG_T0CMPB]) {
					timer_regs[REG_T0CON] &= ~TIMER_CON_RIU;
					timer1 = 1;
					if ((timer_regs[REG_T0CON] & TIMER_CON_CONT) == 0) {
						timer_regs [REG_T0CON] &= ~TIMER_CON_EN;
					}
				}
			}
		}
	}

	if (timer0) {
		int_line_set (INT_LINE_TIMER0, 1);
	} else{
		int_line_set (INT_LINE_TIMER0, 0);
	}
}

// Timer I/O read
int timer_io_read (word_t p, word_t * w)
	{
	int r = p >> 1;
	int t = (p >> (1+2));

	*w = timer_regs [r];
	printf("timer(%d): read %s %02x\n", t, regs[r], *w); 

	return 0;
	}


// Timer I/O write

int timer_io_write (word_t p, word_t  w)
	{
	int r = (p >> 1) & 0x03;
//	int t = (p >> (1+2));

//	printf("timer(%d): write %s %02x\n", t, regs[r], w); 
	timer_regs [r] = w;

	return 0;
	}


// Timer initialization

void timer_init ()
{
	timer_regs [REG_T0CON] = 0;
	timer_regs [REG_T0CNT] = 0;
	timer_regs [REG_T0CMPA] = 0;
	timer_regs [REG_T0CMPB] = 0;

	timer_regs [REG_T1CON] = 0;
	timer_regs [REG_T1CNT] = 0;
	timer_regs [REG_T1CMPA] = 0;
	timer_regs [REG_T1CMPB] = 0;

	timer_regs [REG_T2CON] = 0;
	timer_regs [REG_T2CNT] = 0;
	timer_regs [REG_T2CMPA] = 0;
}
