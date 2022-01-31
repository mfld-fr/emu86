// EMU86 - Common helpers

#pragma once

#include "emu-types.h"

// Output helpers

void print_string (char * s);  // faster than formatted print
void print_column (char * s, byte_t w);

void print_rel (byte_t prefix, short rel);

int map_file_load(char *file);
char *text_symbol (int addr);
