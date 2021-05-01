//------------------------------------------------------------------------------
// EMU86 - Generic console
//------------------------------------------------------------------------------

#pragma once

#include "emu-types.h"

int con_put_char (byte_t c);
int con_pos_set (byte_t row, byte_t col);

int con_get_key (word_t * k);
int con_poll_key ();

void con_normal ();
void con_raw ();

int con_init ();
void con_term ();
