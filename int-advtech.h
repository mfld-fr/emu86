//------------------------------------------------------------------------------
// EMU86 - Advantech interrupt controller
//------------------------------------------------------------------------------

#pragma once

#include "emu-types.h"

// Interrupt lines

#define INT_LINE_TIMER0 0
#define INT_LINE_CASC   1  // reserved
#define INT_LINE_DMA0   2
#define INT_LINE_DMA1   3
#define INT_LINE_INT0   4
#define INT_LINE_INT1   5
#define INT_LINE_INT2   6
#define INT_LINE_INT3   7
#define INT_LINE_INT4   8
#define INT_LINE_WD     9
#define INT_LINE_TIMER1 10
#define INT_LINE_TIMER2 11
#define INT_LINE_SERIAL 12

#define INT_LINE_MAX    13

// Interrupt controller

#define INT_PRIO_MAX    8

int int_io_write (word_t p, word_t w);
