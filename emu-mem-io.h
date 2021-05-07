
#pragma once

#include "emu-types.h"


#define MEM_MAX 0x100000  // 1 MB
#define IO_MAX  0x10000   // 64 KB

#define ROM_BASE 0x80000

// EGA/MDA Adaptor

#define VID_BASE		0xB8000		// EGA
//#define VID_BASE		0xB0000		// MDA (mono)
#define VID_COLS		80
#define VID_LINES		25
#define VID_SIZE		0x4000		// 16k
#define VID_PAGE_SIZE	(VID_COLS * 2 * VID_LINES)

// 6845 CRT Controller

#define CRTC_IOBASE		0x03D4		// color
//#define CRTC_IOBASE	0x03B4		// mono (MDA)

// BIOS Data Area

#define BDA_BASE		0x00400

extern byte_t mem_stat [MEM_MAX];
extern byte_t crtc_curhi, crtc_curlo;
extern int vid_minx, vid_miny;
extern int vid_maxx, vid_maxy;
void update_dirty_region (int x, int y);
void reset_dirty_region ();


// Memory breakpoint

extern byte_t _break_data_flag;
extern addr_t _break_data_addr;


// Memory operations

byte_t * mem_get_addr (addr_t a);

byte_t mem_read_byte (addr_t a);
word_t mem_read_word (addr_t a);

void mem_write_byte (addr_t a, byte_t b, byte_t init);
void mem_write_word (addr_t a, word_t w, byte_t init);

// I/O operations

int io_read_byte (word_t p, byte_t * b);
int io_write_byte (word_t p, byte_t b);

int io_read_word (word_t p, word_t * w);
int io_write_word (word_t p, word_t w);

void mem_io_reset ();
