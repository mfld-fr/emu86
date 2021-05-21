
#pragma once

#include "emu-types.h"

// FIXME: not right as guessed from ELKS code
#define SERIAL_REG_BASE 0xFF66
#define SERIAL_REG_COUNT 3
#define SERIAL_REG_SIZE (SERIAL_REG_COUNT * 2)

int serial_io_read (word_t p, word_t * w);
int serial_io_write (word_t p, word_t w);

