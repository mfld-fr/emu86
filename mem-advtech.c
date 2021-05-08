
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "emu-mem-io.h"

// Memory access

void mem_write_byte (addr_t a, byte_t b, byte_t init)
	{
	assert (a < MEM_MAX);

	if (a >= ROM_BASE && !init)		// Protect ROM
		{
		printf ("warning: writing byte into ROM @ %lxh\n", a);
		}
	else
		{
		byte_t * p = mem_get_addr (a);
		*p = b;
		}
	}

void mem_write_word (addr_t a, word_t w, byte_t init)
	{
	assert (a < MEM_MAX - 1);

	if (a >= (ROM_BASE - 1) && !init)	// Protect ROM
		{
		printf ("warning: writing word into ROM @ %lxh\n", a);
		}
	else
		{
		word_t * p = (word_t *) mem_get_addr (a);
		*p = w;
		}
	}
