/*
 * EMU86 SDL2 Console
 *	Supports ELKS Headless Console (streaming data)
 *	Supports ELKS BIOS Console (BIOS cursor, scroll etc)
 *	Supports ELKS Direct Console (emulated video RAM)
 *	Also used by Emscripten platform for emulator console output in browser
 *
 * Dec 2020 Greg Haerr <greg@censoft.com>
 */

#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "emu-con.h"
#include "emu-mem-io.h"
#include "con-sdl.h"
#include "mem-io-elks.h"

/* configurable parameters*/
#define CHAR_WIDTH	8
#define CHAR_HEIGHT	16
#define BPP			32			/* bits per pixel*/
extern MWIMAGEBITS rom8x16_bits[];

/* calculated parameters*/
#define WIDTH		(VID_COLS * CHAR_WIDTH)
#define HEIGHT		(VID_LINES * CHAR_HEIGHT)
#define PITCH		(WIDTH * (BPP >> 3))

/* display attributes*/
#define ATTR_OR     0x10000		// OR w/pixels (cursor)
#define ATTR_BLINK   0x8000
#define ATTR_BGCOLOR 0x7000
#define ATTR_BOLD    0x0800
#define ATTR_FGCOLOR 0x0700

#define FG_WHITE 0x07
#define BG_BLACK 0x00

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;
static SDL_Texture *sdlTexture;
static float sdlZoom = 1.0;
static unsigned char *screen;
static int curx, cury;


#define RGBDEF(r,g,b)	{ r,g,b,255 }
struct rgba { unsigned char r, g, b, a; };

#if VID_MODE == 7

// MDA palette for attribute mapping
static struct rgba EGA_COLORMAP[16] = {
	RGBDEF( 0  , 0  , 0   ),	/* black*/
	RGBDEF( 0  , 192, 0   ),	/* blue*/
	RGBDEF( 0  , 192, 0   ),	/* green*/
	RGBDEF( 0  , 192, 0   ),	/* cyan*/
	RGBDEF( 0  , 192, 0   ),	/* red*/
	RGBDEF( 0  , 192, 0   ),	/* magenta*/
	RGBDEF( 0  , 192, 0   ),	/* adjusted brown*/
	RGBDEF( 0  , 192, 0   ),	/* ltgray*/
	RGBDEF( 0  , 0  , 0   ),	/* gray*/
	RGBDEF( 0  , 255, 0   ),	/* ltblue*/
	RGBDEF( 0  , 255, 0   ),	/* ltgreen*/
	RGBDEF( 0  , 255, 0   ),	/* ltcyan*/
	RGBDEF( 0  , 255, 0   ),	/* ltred*/
	RGBDEF( 0  , 255, 0   ),	/* ltmagenta*/
	RGBDEF( 0  , 255, 0   ),	/* yellow*/
	RGBDEF( 0  , 255, 0   ),	/* white*/
};

#else

// 16 color EGA palette for attribute mapping
static struct rgba EGA_COLORMAP[16] = {
	RGBDEF( 0  , 0  , 0   ),	/* black*/
	RGBDEF( 0  , 0  , 192 ),	/* blue*/
	RGBDEF( 0  , 192, 0   ),	/* green*/
	RGBDEF( 0  , 192, 192 ),	/* cyan*/
	RGBDEF( 192, 0  , 0   ),	/* red*/
	RGBDEF( 192, 0  , 192 ),	/* magenta*/
	RGBDEF( 192, 128 , 0  ),	/* adjusted brown*/
	RGBDEF( 192, 192, 192 ),	/* ltgray*/
	RGBDEF( 128, 128, 128 ),	/* gray*/
	RGBDEF( 0  , 0  , 255 ),	/* ltblue*/
	RGBDEF( 0  , 255, 0   ),	/* ltgreen*/
	RGBDEF( 0  , 255, 255 ),	/* ltcyan*/
	RGBDEF( 255, 0  , 0   ),	/* ltred*/
	RGBDEF( 255, 0  , 255 ),	/* ltmagenta*/
	RGBDEF( 255, 255, 0   ),	/* yellow*/
	RGBDEF( 255, 255, 255 ),	/* white*/
};
#endif


/* draw a character bitmap*/
static void sdl_drawbitmap(int c, int x, int y)
{
	MWIMAGEBITS *imagebits = rom8x16_bits + CHAR_HEIGHT * (c & 0xFF);
    int minx = x;
    int maxx = x + CHAR_WIDTH - 1;
    int bitcount = 0;
	unsigned short bitvalue = 0;
	int height = CHAR_HEIGHT;

#if VID_MODE == 7
	// Special MDA handling
	// 0x00, 0x08, 0x80, 0x88 (blank) -> 0x00, otherwise 0x07
	// 0x70, 0x78, 0xF0, 0xF8 (reverse video) -> 0x70, otherwise 0x07
	// MDA requires attribute 0x0700 for reverse video
	if ((c & 0x7700) != 0x7000 && (c & 0x7700) != 0x0000) {
		c |= ATTR_FGCOLOR;
		c &= ~ATTR_BGCOLOR;
	}
#endif
	int fg_color = (c & ATTR_FGCOLOR) >> 8;
	if (c & ATTR_BOLD) fg_color += 8;
	int bg_color = (c & ATTR_BGCOLOR) >> 12;
	unsigned char fg_r = EGA_COLORMAP[fg_color].r;
	unsigned char fg_g = EGA_COLORMAP[fg_color].g;
	unsigned char fg_b = EGA_COLORMAP[fg_color].b;
	unsigned char bg_r = EGA_COLORMAP[bg_color].r;
	unsigned char bg_g = EGA_COLORMAP[bg_color].g;
	unsigned char bg_b = EGA_COLORMAP[bg_color].b;

    while (height > 0) {
		unsigned char *pixels;
        if (bitcount <= 0) {
            bitcount = 16;
            bitvalue = *imagebits++;
			pixels = screen + y * PITCH + x * (BPP >> 3);
        }
        if (MWIMAGE_TESTBIT(bitvalue)) {
			*pixels++ = fg_b;
			*pixels++ = fg_g;
			*pixels++ = fg_r;
			*pixels++ = 0xFF;
		} else if (!(c & ATTR_OR)) {
			*pixels++ = bg_b;
			*pixels++ = bg_g;
			*pixels++ = bg_r;
			*pixels++ = 0xFF;
		}
        bitvalue = MWIMAGE_SHIFTBIT(bitvalue);
        bitcount--;
        if (x++ == maxx) {
            x = minx;
            ++y;
            --height;
            bitcount = 0;
        }
    }
}


// draw characters from adapter RAM
static void draw_video_ram(int sx, int sy, int ex, int ey)
{
	int x, y;
	word_t *vidram = (word_t *)&mem_stat[VID_BASE];

	for (y = sy; y < ey; y++)
	{
		int j = y * VID_COLS + sx;
		for (x = sx; x < ex; x++)
		{
			sdl_drawbitmap(vidram[j], x * CHAR_WIDTH, y * CHAR_HEIGHT);
			j++;
		}
	}
}


static void cursoron(void)
{
	mem_stat[BDA_BASE+0x50] = curx;
	mem_stat[BDA_BASE+0x51] = cury;
	int pos = cury * VID_COLS + curx;
	crtc_curhi = pos >> 8;
	crtc_curlo = pos & 0xff;
}

static void cursoroff(void)
{
}

// clear line y from x1 up to and including x2 to attribute attr
static void clear_line(int x1, byte_t x2, byte_t y, byte_t attr)
{
	int x;

	for (x = x1; x <= x2; x++) {
		*(word_t *)&mem_stat[VID_BASE + (y * VID_COLS + x) * 2] = ' ' | (attr << 8);
		update_dirty_region(x, y);
	}
}

// scroll adapter RAM up from line y1 up to and including line y2
static void scrollup(int y1, int y2, byte_t attr)
{
	int pitch = VID_COLS * 2;
	byte_t *vid = mem_stat + VID_BASE + y1 * pitch;

	memcpy(vid, vid + pitch, (VID_LINES - y1) * pitch);
	clear_line (0, VID_COLS-1, y2, attr);
	update_dirty_region (0, 0);
	update_dirty_region (VID_COLS-1, VID_LINES-1);
}


// scroll adapter RAM down from line y1 up to and including line y2
static void scrolldn(int y1, int y2, byte_t attr)
{
	int pitch = VID_COLS * 2;
	byte_t *vid = mem_stat + VID_BASE + (VID_LINES-1) * pitch;
	int y = y2;

	while (--y >= y1) {
		memcpy (vid, vid - pitch, pitch);
		vid -= pitch;
	}
	clear_line (0, VID_COLS-1, y1, attr);
	update_dirty_region (0, 0);
	update_dirty_region (VID_COLS-1, VID_LINES-1);
}


/* output character at cursor location*/
void sdl_textout(byte_t c, byte_t a)
{
	cursoroff();

	switch (c) {
	case '\0':	return;
	case '\b':	if (--curx <= 0) curx = 0; goto update;
	case '\r':	curx = 0; goto update;
	case '\n':  goto scroll;
	}

	mem_stat [VID_BASE + (cury * VID_COLS + curx) * 2 + 0] = c;
	mem_stat [VID_BASE + (cury * VID_COLS + curx) * 2 + 1] = a;

	update_dirty_region (curx, cury);

	if (++curx >= VID_COLS) {
		curx = 0;
scroll:
		if (++cury >= VID_LINES) {
			scrollup(0, VID_LINES - 1, FG_WHITE | BG_BLACK);
			cury = VID_LINES - 1;
		}
	}

update:
	cursoron();
}


int con_put_char (byte_t c, byte_t a)
	{
	sdl_textout(c, a);
	return 0;
	}


int con_pos_set (byte_t row, byte_t col)
	{
	if (curx != col || cury != row)
		{
		cursoroff();
		cury = row;
		curx = col;
		cursoron();
		}
	return 0;
	}


int con_pos_get (byte_t *row, byte_t *col)
	{
	*row = cury;
	*col = curx;
	return 0;
	}


int con_scroll (int dn, byte_t n, byte_t at, byte_t r, byte_t c, byte_t r2, byte_t c2)
	{
	cursoroff();
	if (n == 0 || n >= VID_LINES)
		clear_line(c, c2, r, at);
	else if (r != r2)
		{
		// FIXME count n, c, c2 ignored
		if (dn)
			scrolldn(r, r2, at);
		else scrollup(r, r2, at);
		}
	cursoron();
	return 0;
	}


// Shift key using the QWERTY layout

static SDL_Keycode key_shift (SDL_Keycode kc)
	{
	if (kc >= 'a' && kc < 'z')
		return (kc ^ 0x20);  // upper case

	switch (kc) {
		case '`':  return '~';
		case '1':  return '!';
		case '2':  return '@';
		case '3':  return '#';
		case '4':  return '$';
		case '5':  return '%';
		case '6':  return '^';
		case '7':  return '&';
		case '8':  return '*';
		case '9':  return '(';
		case '0':  return ')';
		case '-':  return '_';
		case '=':  return '+';
		case '[':  return '{';
		case ']':  return '}';
		case '\\': return '|';
		case ';':  return ':';
		case '\'': return '"';
		case ',':  return '<';
		case '.':  return '>';
		case '/':  return '?';

		default: break;
		}

	return kc;
	}


static void sdl_key (Uint8 state, SDL_Keysym sym)
	{
	while (1)
		{
		SDL_Scancode sc = sym.scancode;
		SDL_Keycode kc = sym.sym;
		Uint16 mod = sym.mod;

		// Invariant QWERTY code mapping
		// whatever the keyboard layout

		switch (sc)
			{
			case SDL_SCANCODE_MINUS:  kc = '-'; break;
			case SDL_SCANCODE_PERIOD: kc = '.'; break;
			case SDL_SCANCODE_SLASH:  kc = '/'; break;
			default: break;
			}

		if (kc == SDLK_LSHIFT || kc == SDLK_RSHIFT
			|| kc == SDLK_LCTRL || kc == SDLK_RCTRL)
			break;

		if (kc < 256 && (mod & (KMOD_SHIFT | KMOD_CAPS)))
			kc = key_shift (kc);

		if (kc < 256 && (mod & KMOD_CTRL))
			kc &= 0x1f;  // convert to control char

		if (kc == 0x007F) kc = '\b';  // convert DEL to BS

		if (kc >= 256) {
			printf ("warning: keycode=%X\n", kc);
			break;
			}

		con_put_key ((word_t) kc);
		break;
		}
	}


void con_raw ()
	{
	}


void con_normal ()
	{
	}


/* update SDL from framebuffer*/
static void sdl_draw(int x, int y, int width, int height)
{
	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = width? width: WIDTH;
	r.h = height? height: HEIGHT;

	unsigned char *pixels = screen + y * PITCH + x * (BPP >> 3);
	SDL_UpdateTexture(sdlTexture, &r, pixels, PITCH);

	/* copy texture to display*/
	SDL_SetRenderDrawColor(sdlRenderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
}

// Called periodically from the main loop

int con_proc ()
	{
	int err = 0;
	static int lastx = 0, lasty = 0, needscursor = 0;

	if (vid_maxx >= 0 || vid_maxy >= 0)
		{
		// draw text bitmaps from adaptor RAM
		draw_video_ram (vid_minx, vid_miny, vid_maxx+1, vid_maxy+1);

		// update SDL
		sdl_draw (vid_minx * CHAR_WIDTH, vid_miny * CHAR_HEIGHT,
			(vid_maxx-vid_minx+1) * CHAR_WIDTH, (vid_maxy-vid_miny+1) * CHAR_HEIGHT);

		reset_dirty_region ();
		needscursor = 1;
		}
		else
		{
		// draw cursor
		int pos = (crtc_curhi << 8) | crtc_curlo;
		int y = pos / VID_COLS;
		int x = pos % VID_COLS;
		if (lastx != x || lasty != y || needscursor)
			{
			// remove last cursor
			draw_video_ram (lastx, lasty, lastx+1, lasty+1);
			sdl_draw (lastx * CHAR_WIDTH, lasty * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);

			// draw current cursor
			sdl_drawbitmap ('_'|ATTR_OR|ATTR_FGCOLOR, x * CHAR_WIDTH, y * CHAR_HEIGHT);
			sdl_draw (x * CHAR_WIDTH, y * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
			lastx = x; lasty = y;
			needscursor = 0;
			}
		}

	SDL_Event event;

	while (SDL_PollEvent (&event))
		{
		switch (event.type)
			{
			case SDL_QUIT:
				err = 1;  // exit the main loop
				break;

			case SDL_KEYDOWN:
				sdl_key (event.key.state, event.key.keysym);
				break;
			}
		}

	return err;
	}


/* init SDL subsystem, return < 0 on error*/
int con_init(void)
{
	int	pixelformat;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		printf("Can't initialize SDL\n");
		return -1;
	}

	sdlWindow = SDL_CreateWindow("EMU86", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				WIDTH*sdlZoom, HEIGHT*sdlZoom, SDL_WINDOW_RESIZABLE);
	if (!sdlWindow) {
		printf("SDL: Can't create window\n");
		return -1;
	}

	sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
	if (!sdlRenderer) {
		printf("SDL: Can't create renderer\n");
		return -1;
	}
	/* 
	 * Set the SDL texture pixel format to match the framebuffer format
	 * to eliminate pixel conversions.
	 */
	pixelformat = SDL_PIXELFORMAT_ARGB8888;	/* MWPF_TRUECOLORARGB*/
	sdlTexture = SDL_CreateTexture(sdlRenderer, pixelformat, SDL_TEXTUREACCESS_STREAMING,
							WIDTH, HEIGHT);
	if (!sdlTexture) {
		printf("SDL: Can't create texture\n");
		return -1;
	}

	/* setup zoom*/
	SDL_RenderSetLogicalSize(sdlRenderer, WIDTH, HEIGHT);
	SDL_RenderSetScale(sdlRenderer, sdlZoom, sdlZoom);

	screen = malloc(HEIGHT * PITCH);
	if (!screen) {
		printf("SDL: Can't malloc framebuffer\n");
		return -1;
	}
	memset(screen, 0x00, HEIGHT*PITCH);

	con_init_key ();
	return 0;
}


void con_term (void)
{
	free(screen);
	SDL_Quit();
}
