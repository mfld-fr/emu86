
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "emu-mem-io.h"

static byte_t mem_stat [MEM_MAX];

byte_t _break_data_flag = 0;
addr_t _break_data_addr = 0x100000;


// Memory access

byte_t * mem_get_addr (addr_t a)
	{
	assert (a < MEM_MAX);

	// Data breakpoint test
	// Will break the main execution loop later

	if (a == _break_data_addr)
		_break_data_flag = 1;

	return (mem_stat + a);
	}


byte_t mem_read_byte (addr_t a)
	{
	assert (a < MEM_MAX);
	byte_t * p = mem_get_addr (a);
	return *p;
	}

word_t mem_read_word (addr_t a)
	{
	assert (a < MEM_MAX - 1);
	word_t * p = (word_t *) mem_get_addr (a);
	return *p;
	}


void mem_write_byte (addr_t a, byte_t b, byte_t init)
	{
	assert (a < MEM_MAX);

	// Protect ROM

	if (a >= ROM_BASE && !init)
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

	// Protect ROM

	if (a >= (ROM_BASE - 1) && !init)
		{
		printf ("warning: writing word into ROM @ %lxh\n", a);
		}
	else
		{
		word_t * p = (word_t *) mem_get_addr (a);
		*p = w;
		}
	}

//-------------------------------------------------------------------------------

// Memory reset

void mem_io_reset ()
	{
	// No or uninitialized memory: all bits to 1
	// Used to check interrupt vector initialized in op_int()

	memset (mem_stat, 0xFF, sizeof mem_stat);
	}

//-------------------------------------------------------------------------------

