//------------------------------------------------------------------------------
// EMU86 - 8018X interrupt controller
//------------------------------------------------------------------------------

#pragma once

#include "emu-types.h"

// Interrupt lines
// Merge 8018X XL/EA/EB/EC variants

#define INT_LINE_TIMER0     0
#define INT_LINE_CASC       1
#define INT_LINE_DMA0       2
#define INT_LINE_DMA1       3
#define INT_LINE_INT0       4
#define INT_LINE_INT1       5
#define INT_LINE_INT2       6
#define INT_LINE_INT3       7
#define INT_LINE_INT4       9
#define INT_LINE_TIMER1    10
#define INT_LINE_TIMER2    11
#define INT_LINE_SERIAL_RX 12
#define INT_LINE_SERIAL_TX 13

#define INT_LINE_MAX       14

// Interrupt controller

#define INT_PRIO_MAX    8

#define INT_REG_BASE 0xFF02
#define INT_REG_COUNT 15
#define INT_REG_SIZE (INT_REG_COUNT * 2)

int int_io_read (word_t p, word_t * w);
int int_io_write (word_t p, word_t w);
