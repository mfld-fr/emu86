//------------------------------------------------------------------------------
// EMU86 - Generic serial port
//------------------------------------------------------------------------------

#pragma once

#include "emu-types.h"

// Virtual device

int serial_proc ();

void serial_dev_init ();
void serial_dev_term ();

// Backend

int serial_send (byte_t c);
int serial_recv (byte_t * c);

int serial_poll ();

// Serial modes

void serial_normal ();
void serial_raw ();

// Subsystem

int serial_init ();
void serial_term ();
