
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "emu-mem-io.h"

byte_t mem_stat [MEM_MAX];

byte_t _break_data_flag = 0;
addr_t _break_data_addr = 0x100000;

int vid_minx = 32767, vid_miny = 32767;
int vid_maxx = -1, vid_maxy = -1;
#define MIN(a,b)      ((a) < (b) ? (a) : (b))
#define MAX(a,b)      ((a) > (b) ? (a) : (b))

void update_dirty_region (int x, int y)
	{
	vid_minx = MIN(x, vid_minx);
	vid_miny = MIN(y, vid_miny);
	vid_maxx = MAX(vid_maxx, x);
	vid_maxy = MAX(vid_maxy, y);
	}

void reset_dirty_region ()
	{
	vid_minx = vid_miny = 32767;
	vid_maxx = vid_maxy = -1;
	}

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

	if (a >= VID_BASE && a < (VID_BASE + VID_PAGE_SIZE))
		{
		byte_t * p = (byte_t *) mem_get_addr (a);
		*p = b;

		// calculate bounding rectangle
		a = (a - VID_BASE) / 2;
		update_dirty_region(a % VID_COLS, a / VID_COLS);
		}
	else if (a >= ROM_BASE && !init)		// Protect ROM
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

	if (a >= VID_BASE && a < (VID_BASE + VID_PAGE_SIZE))
		{
		word_t * p = (word_t *) mem_get_addr (a);
		*p = w;

		// calculate bounding rectangle
		a = (a - VID_BASE) / 2;
		update_dirty_region(a % VID_COLS, a / VID_COLS);
		}
	else if (a >= (ROM_BASE - 1) && !init)	// Protect ROM
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

	memset (mem_stat+BDA_BASE, 0x00, 256);	// FIXME move to rom-elks.c?
	}

//-------------------------------------------------------------------------------

