
#include "emu-mem-io.h"

#include <stdio.h>


// Memory access

byte_t mem_read_byte (addr_t a)
	{
	// No memory mapped device
	return mem_read_byte_0 (a);
	}


word_t mem_read_word (addr_t a)
	{
	// No memory mapped device
	return mem_read_word_0 (a);
	}


void mem_write_byte (addr_t a, byte_t b, byte_t init)
	{
	// No memory mapped device
	mem_write_byte_0 (a, b, init);
	}


void mem_write_word (addr_t a, word_t w, byte_t init)
	{
	// No memory mapped device
	mem_write_word_0 (a, w, init);
	}
