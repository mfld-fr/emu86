//------------------------------------------------------------------------------
// EMU86 - 8018X interrupt controller
//------------------------------------------------------------------------------

#pragma once

#include "emu-types.h"

// Interrupt lines
// 8018X EB variant

#define INT_LINE_TIMER0 0
#define INT_LINE_CASC   1  // reserved
#define INT_LINE_SERIAL 2
#define INT_LINE_INT4   3
#define INT_LINE_INT0   4
#define INT_LINE_INT1   5
#define INT_LINE_INT2   6
#define INT_LINE_INT3   7

#define INT_LINE_MAX    8

// Interrupt controller

#define INT_PRIO_MAX    8

#define INT_REG_BASE 0xFF02
#define INT_REG_COUNT 15
#define INT_REG_SIZE (INT_REG_COUNT * 2)

int int_io_write (word_t p, word_t w);
