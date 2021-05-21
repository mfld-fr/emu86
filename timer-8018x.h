//-------------------------------------------------------------------------------
// EMU86 - 8018X timers
//-------------------------------------------------------------------------------

#pragma once

#include "emu-types.h"

#ifdef MCU_8018X_EB
#define TIMER_REG_BASE 0xFF30
#endif

#ifdef MCU_R8810
#define TIMER_REG_BASE 0xFF50
#endif

#define TIMER_REG_COUNT 12
#define TIMER_REG_SIZE (TIMER_REG_COUNT * 2)

int timer_io_read (word_t p, word_t * w);
int timer_io_write (word_t p, word_t  w);
