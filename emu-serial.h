//------------------------------------------------------------------------------
// EMU86 - Generic serial port
//------------------------------------------------------------------------------

#pragma once

#include "emu-types.h"

int serial_proc ();

void serial_init ();

int serial_send (byte_t c);
int serial_recv (byte_t * c);

int serial_poll ();

void serial_term ();
