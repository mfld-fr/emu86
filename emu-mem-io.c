
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "emu-mem-io.h"

byte_t mem_stat [MEM_MAX];

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


//-------------------------------------------------------------------------------

// Memory reset

void mem_io_reset ()
	{
	// No or uninitialized memory: all bits to 1
	// Used to check interrupt vector initialized in op_int()

	memset (mem_stat, 0xFF, sizeof mem_stat);
	}

//-------------------------------------------------------------------------------

