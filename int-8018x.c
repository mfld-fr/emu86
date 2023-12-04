//------------------------------------------------------------------------------
// EMU86 - 8018X interrupt controller
//------------------------------------------------------------------------------

#include "emu-int.h"

#include "int-8018x.h"

#include <stdio.h>


#define INT_REG_EOI  0
#define INT_REG_MASK 3
#define INT_REG_PRIMSK 4
#define INT_REG_REQST 6
#define INT_REG_TCUCON 8
#define INT_REG_SCUCON 9

#define INT_REG_I4CON 10
#define INT_REG_I0CON 11
#define INT_REG_I1CON 12
#define INT_REG_I2CON 13
#define INT_REG_I3CON 14

// REQST and MASK register bits
#define BIT_TMR  0
// Bit 1 is reserved on all models
// Bits 2 & 3 are for DMA in XL variant
// Bits 2 & 3 are for serial and INT4 for EB variant
#define BIT_SER  2
#define BIT_INT4 3
#define BIT_INT0 4
#define BIT_INT1 5
#define BIT_INT2 6
#define BIT_INT3 7

int _int_line_max = INT_LINE_MAX;
int _int_prio_max = INT_PRIO_MAX;

int _pri_mask = 7;

// Timers & serial are edge triggered
int _int_mode [INT_LINE_MAX] =
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1};

// Default priority for timers is 0
// Default priority for serial is 1
int _int_prio [INT_LINE_MAX] =
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};

int _int_vect [INT_LINE_MAX] =
	{ 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};

int _int_mask [INT_LINE_MAX] =
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// Initialized to zero by CRT
int _int_req [INT_LINE_MAX];
int _int_serv [INT_LINE_MAX];

word_t imask;

// PIC I/O read

int int_io_read (word_t p, word_t * w)
{
	int err = 0;

	int r = p >> 1;

	if (r == INT_REG_REQST)
		{
			word_t out = 0;
			for (size_t i = 0; i < 8; i++) {
				if (i == BIT_TMR) {
					if (_int_req[INT_LINE_TIMER0] || _int_req[INT_LINE_TIMER1] || _int_req[INT_LINE_TIMER2]) {
						out |= 1 << i;
					}
				} else if (i == BIT_SER) {
					if (_int_req[INT_LINE_SERIAL_RX] || _int_req[INT_LINE_SERIAL_TX]) {
						out |= 1 << i;
					}
				} else if (_int_req[i]) {
					out |= 1 << i;
				}
			}
			*w = out;
		}
	else if (r == INT_REG_MASK)
		{
			word_t out = 0;
			for (size_t i = 0; i < 8; i++) {
				if (i == BIT_TMR) {
					if (_int_mask[INT_LINE_TIMER0]) {
						out |= 1 << i;
					}
				} else if (i == BIT_SER) {
					if (_int_mask[INT_LINE_SERIAL_RX]) {
						out |= 1 << i;
					}
				} else if (_int_mask[i]) {
					out |= 1 << i;
				}
			}
		//	printf("int read IMASK: %08x\n", out);
			*w = out;
		}
	else
		{
		printf ("\nerror: read from int register %02x %i", p+INT_REG_BASE, r);
		err = -1;
		}

	return err;
}

// PIC I/O write

int int_io_write (word_t p, word_t w)
	{
	int err = 0;

	int r = p >> 1;

	if (r == INT_REG_EOI)
		{
//			printf("EOI %d\n", w);
		if (w == 0x8000)
			{
			int_end_prio ();
			}
		else
			{
			switch (w & 0x001F)
			{
				case 8:
					int_end_line (INT_LINE_TIMER0);
					int_end_line (INT_LINE_TIMER1);
					int_end_line (INT_LINE_TIMER2);
					break;
				case 12:
					int_end_line (INT_LINE_INT0);
					break;
				case 13:
					int_end_line (INT_LINE_INT1);
					break;
				case 14:
					int_end_line (INT_LINE_INT2);
					break;
				case 15:
					int_end_line (INT_LINE_INT3);
					break;
				case 17:
					int_end_line (INT_LINE_INT4);
					break;
				case 20:
					int_end_line (INT_LINE_SERIAL_RX);
					int_end_line (INT_LINE_SERIAL_TX);
					break;
				default:
					printf("nemam %d\n", w & 0x1f);
			}
			}
		}
	else if (r == INT_REG_MASK)
		{
			for (int i = 0; i < 8; i++) {
				int mask = (w & 0x01) ? 1 : 0;

				if (i == BIT_TMR) {
					_int_mask[INT_LINE_TIMER0] = mask;
					_int_mask[INT_LINE_TIMER1] = mask;
					_int_mask[INT_LINE_TIMER2] = mask;
				}
				else if (i == BIT_SER) {
					_int_mask[INT_LINE_SERIAL_RX] = mask;
					_int_mask[INT_LINE_SERIAL_TX] = mask;
				}
				else {
					_int_mask[i] = mask;
				}
				w >>= 1;
			}
		}
	else if (r == INT_REG_PRIMSK)
	{
//		printf("int primask %d\n", w & 7);
		_pri_mask = w & 7;
	}
	else if (r == INT_REG_TCUCON)
	{
		// TODO: level ?
		_int_mask[INT_LINE_TIMER0] = (p >> 3) & 1;
		_int_prio[INT_LINE_TIMER0] = p & 7;
		_int_mask[INT_LINE_TIMER1] = (p >> 3) & 1;
		_int_prio[INT_LINE_TIMER1] = p & 7;
		_int_mask[INT_LINE_TIMER2] = (p >> 3) & 1;
		_int_prio[INT_LINE_TIMER2] = p & 7;
//		printf("int tcu pri %d mask %d\n", p & 7, (p >> 3) & 1);
	}
	else if (r == INT_REG_SCUCON)
	{
		// TODO: level ?
		_int_mask[INT_LINE_SERIAL_RX] = (p >> 3) & 1;
		_int_mask[INT_LINE_SERIAL_TX] = (p >> 3) & 1;
		_int_prio[INT_LINE_SERIAL_RX] = p & 7;
		_int_prio[INT_LINE_SERIAL_TX] = p & 7;
//		printf("int scu pri %d mask %d\n", p & 7, (p >> 3) & 1);
	}
	else if (r == INT_REG_I4CON)
	{
		// TODO: level ?
		_int_mask[INT_LINE_INT4] = (p >> 3) & 1;
		_int_prio[INT_LINE_INT4] = p & 7;
//		printf("int int4 pri %d mask %d\n", p & 7, (p >> 3) & 1);
	}
	else if (r == INT_REG_I0CON)
	{
		// TODO: level ?
		_int_mask[INT_LINE_INT0] = (p >> 3) & 1;
		_int_prio[INT_LINE_INT0] = p & 7;
//		printf("int int0 pri %d mask %d\n", p & 7, (p >> 3) & 1);
	}
	else if (r == INT_REG_I1CON)
	{
		_int_mask[INT_LINE_INT1] = (p >> 3) & 1;
		_int_prio[INT_LINE_INT1] = p & 7;
//		printf("int int1 pri %d mask %d\n", p & 7, (p >> 3) & 1);
	}
	else if (r == INT_REG_I2CON)
	{
		_int_mask[INT_LINE_INT2] = (p >> 3) & 1;
		_int_prio[INT_LINE_INT2] = p & 7;
//		printf("int int2 pri %d mask %d\n", p & 7, (p >> 3) & 1);
	}
	else if (r == INT_REG_I3CON)
	{
		_int_mask[INT_LINE_INT3] = (p >> 3) & 1;
		_int_prio[INT_LINE_INT3] = p & 7;
//		printf("int int3 pri %d mask %d\n", p & 7, (p >> 3) & 1);
	}
	else
		{
		printf ("\nerror: write %hXh to int register %02x %i", w, p+INT_REG_BASE, r);
		err = -1;
		}

	return err;
	}


// PIC initialization

void int_init (void)
	{
	}
