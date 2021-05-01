//-------------------------------------------------------------------------------
// EMU86 - Generic character backend
//-------------------------------------------------------------------------------

#pragma once

#include "emu-types.h"

int char_send (byte_t c);
int char_recv (byte_t * c);

int char_poll ();

void char_raw ();
void char_normal ();

int char_init ();
void char_term ();
