#pragma once

#include "emu-types.h"

#define TIMER_REG_COUNT 12
#define TIMER_REG_BASE 0xFF30
#define TIMER_REG_SIZE (TIMER_REG_COUNT * 2)

int timer_io_read (word_t p, word_t * w);
int timer_io_write (word_t p, word_t  w);
