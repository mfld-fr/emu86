//------------------------------------------------------------------------------
// EMU86 - Generic console
//------------------------------------------------------------------------------

#pragma once

#include "list.h"
#include "emu-types.h"

// Screen emulation

int con_put_char (byte_t c, byte_t a);
int con_pos_set (byte_t row, byte_t col);
int con_pos_get (byte_t *row, byte_t *col);
int con_scrollup (byte_t n, byte_t at, byte_t r, byte_t c, byte_t r2, byte_t c2);

// Keyboard queue

void con_init_key ();
void con_put_key (word_t k);
int con_get_key (word_t * k);
int con_poll_key ();

// Console modes

void con_normal ();
void con_raw ();

// Console main

int con_proc ();

int con_init ();
void con_term ();

//------------------------------------------------------------------------------
