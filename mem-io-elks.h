
#pragma once

#include "emu-types.h"


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
