
#pragma once

#include "emu-types.h"

// Video
// TODO: move to con-video.h

#define VID_COLS		80
#define VID_LINES		25
#define VID_SIZE		0x4000		// 16k
#define VID_PAGE_SIZE	(VID_COLS * 2 * VID_LINES)

// 6845 CRT Controller

#define CRTC_CTRL_PORT	0x03D4		// color
//#define CRTC_CTRL_PORT 0x03B4		// mono (MDA)
#define CRTC_DATA_PORT	(CRTC_CTRL_PORT+1)

// 8253/8254 Timer
#define TIMER_DATA_PORT	0x42
#define TIMER_CTRL_PORT	(TIMER_DATA_PORT+1)

extern byte_t crtc_curhi, crtc_curlo;
extern int vid_minx, vid_miny;
extern int vid_maxx, vid_maxy;

int vid_base(void);
void update_dirty_region (int x, int y);
void reset_dirty_region ();
