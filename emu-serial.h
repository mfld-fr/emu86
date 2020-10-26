
#pragma once

#include "op-common.h"

// Serial device

int serial_proc ();

void serial_dev_init ();

// Serial backend

int serial_send (byte_t c);
int serial_recv (byte_t * c);

int serial_poll ();

void serial_raw ();
void serial_normal ();

void serial_init ();
void serial_term ();
