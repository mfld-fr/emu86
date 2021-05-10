
#include "emu-mem-io.h"
#include "mem-io-elks.h"

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

// Memory and video access

void mem_write_byte (addr_t a, byte_t b, byte_t init)
	{
	// FIXME change VID_PAGE_SIZE to VID_SIZE when pages implemented
	if (a >= VID_BASE && a < (VID_BASE + VID_PAGE_SIZE))
		{
		byte_t * p = (byte_t *) mem_get_addr (a);
		*p = b;

		// calculate bounding rectangle
		a = (a - VID_BASE) / 2;
		update_dirty_region(a % VID_COLS, a / VID_COLS);
		}

	// TODO: intercept access to BDA

	else
		{
		// Main memory

		mem_write_byte_0 (a, b, init);
		}
	}

void mem_write_word (addr_t a, word_t w, byte_t init)
	{
	// FIXME change VID_PAGE_SIZE to VID_SIZE when pages implemented
	if (a >= VID_BASE && a < (VID_BASE + VID_PAGE_SIZE))
		{
		word_t * p = (word_t *) mem_get_addr (a);
		*p = w;

		// calculate bounding rectangle
		a = (a - VID_BASE) / 2;
		update_dirty_region(a % VID_COLS, a / VID_COLS);
		}

	// TODO: intercept access to BDA

	else
		{
		// Main memory

		mem_write_word_0 (a, w, init);
		}
	}
