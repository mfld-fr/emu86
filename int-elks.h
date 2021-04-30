#pragma once

// Interrupt lines

#define INT_LINE_TIMER 0
#define INT_LINE_KEY   1
#define INT_LINE_CASC  2
#define INT_LINE_COM2  3
#define INT_LINE_COM1  4
#define INT_LINE_HDD   5
#define INT_LINE_FDD   6
#define INT_LINE_LPT1  7

#define INT_LINE_MAX    8

// Interrupt controller

#define INT_PRIO_MAX    8

int int_io_write (word_t p, byte_t b);
