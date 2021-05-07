/*
 * EMU86 SDL2 Console
 *	Supports ELKS Headless Console (streaming data)
 *	Supports ELKS BIOS Console (BIOS cursor, scroll etc)
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
#define COLS		80
#define LINES		25
#define CHAR_WIDTH	8
#define CHAR_HEIGHT	16
#define BPP			32			/* bits per pixel*/
extern MWIMAGEBITS rom8x16_bits[];

/* calculated parameters*/
#define WIDTH		(COLS * CHAR_WIDTH)
#define HEIGHT		(LINES * CHAR_HEIGHT)
#define PITCH		(WIDTH * (BPP >> 3))

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;
static SDL_Texture *sdlTexture;
static float sdlZoom = 1.0;
static unsigned char *screen;
//static int changed;
static int curx, cury;


/* draw a character bitmap*/
static void sdl_drawbitmap(unsigned char c, int x, int y)
{
	MWIMAGEBITS *imagebits = rom8x16_bits + CHAR_HEIGHT * c;
    int minx = x;
    int maxx = x + CHAR_WIDTH - 1;
    int bitcount = 0;
	unsigned short bitvalue = 0;
	int height = CHAR_HEIGHT;

    while (height > 0) {
		unsigned char *pixels;
        if (bitcount <= 0) {
            bitcount = 16;
            bitvalue = *imagebits++;
			pixels = screen + y * PITCH + x * (BPP >> 3);
        }
        if (MWIMAGE_TESTBIT(bitvalue)) {
			*pixels++ = 0xff;
			*pixels++ = 0xff;
			*pixels++ = 0xff;
			*pixels++ = 0xff;
		} else {
			*pixels++ = 0;
			*pixels++ = 0;
			*pixels++ = 0;
			*pixels++ = 0xff;
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

	for (y = sy; y < ey; y++)
	{
		int j = VID_BASE + (y * COLS + sx) * 2;
		for (x = sx; x < ex; x++)
		{
			sdl_drawbitmap(mem_stat[j], x * CHAR_WIDTH, y * CHAR_HEIGHT);
			j += 2;
		}
	}
}


static void cursoron(void)
{
#if 0
	/* no simulated cursor in first column because of CR issue*/
	if (curx != 0)
	{
		sdl_drawbitmap('_', curx * CHAR_WIDTH, cury * CHAR_HEIGHT);
		changed = 1;
	}
#endif
	mem_stat[BDA_BASE+0x50] = curx;
	mem_stat[BDA_BASE+0x51] = cury;
	int pos = cury * COLS + curx;
	crtc_curhi = pos >> 8;
	crtc_curlo = pos & 0xff;
}

static void cursoroff(void)
{
#if 0
	if (curx != 0)
	{
		sdl_drawbitmap(' ', curx * CHAR_WIDTH, cury * CHAR_HEIGHT);
		changed = 1;
	}
#endif
}


static void scrollup(void)
{
#if 0
	memcpy(screen, screen + CHAR_HEIGHT * PITCH, (LINES-1) * CHAR_HEIGHT * PITCH);
	memset(screen + (LINES-1) * CHAR_HEIGHT * PITCH, 0, CHAR_HEIGHT * PITCH);
	changed = 1;
#endif
	// scroll adapter RAM

	int pitch = COLS * 2;
	byte_t *vid = mem_stat + VID_BASE;
	memcpy(vid, vid + pitch, (LINES-1) * pitch);
	memset(vid + (LINES-1) * pitch, 0, pitch);
	update_dirty_region (0, 0);
	update_dirty_region (COLS-1, LINES-1);
}


/* output character at cursor location*/
void sdl_textout(unsigned char c)
{
	//changed = 1;
	cursoroff();

	switch (c) {
	case '\0':	return;
	case '\b':	if (--curx <= 0) curx = 0; goto update;
	case '\r':	curx = 0; goto update;
	case '\n':  goto scroll;
	}

	//sdl_drawbitmap(c, curx * CHAR_WIDTH, cury * CHAR_HEIGHT);
	mem_stat[VID_BASE + (cury * COLS + curx) * 2] = c;
	update_dirty_region (curx, cury);

	if (++curx >= COLS) {
		curx = 0;
scroll:
		if (++cury >= LINES) {
			scrollup();
			cury = LINES - 1;
		}
	}

update:
	cursoron();
}


int con_put_char (byte_t c)
	{
	sdl_textout(c);
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


int con_scrollup ()
	{
	cursoroff();
	scrollup();
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

//printf("DRAW %d,%d %d,%d\n", x, y, width, height);
	unsigned char *pixels = screen + y * PITCH + x * (BPP >> 3);
	SDL_UpdateTexture(sdlTexture, &r, pixels, PITCH);

	/* copy texture to display*/
	SDL_SetRenderDrawColor(sdlRenderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
	//changed = 0;
}

// Called periodically from the main loop

int con_proc ()
	{
	int err = 0;
	int lastx = -1, lasty;

	if (vid_maxx >= 0 || vid_maxy >= 0)
		{
		// draw text bitmaps from adaptor RAM
		draw_video_ram (vid_minx, vid_miny, vid_maxx+1, vid_maxy+1);

		// draw cursor
		int pos = (crtc_curhi << 8) | crtc_curlo;
		int y = pos / COLS;
		int x = pos % COLS;
		sdl_drawbitmap ('_', x * CHAR_WIDTH, y * CHAR_HEIGHT);
		update_dirty_region (x, y);
		lastx = x; lasty = y;

		// update SDL
		sdl_draw (vid_minx * CHAR_WIDTH, vid_miny * CHAR_HEIGHT,
			(vid_maxx-vid_minx+1) * CHAR_WIDTH, (vid_maxy-vid_miny+1) * CHAR_HEIGHT);

		// reset but redraw previous cursor location contents
		reset_dirty_region ();
		if (lastx != x || lasty != y)
			update_dirty_region (x, y);
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

  	//SDL_ShowCursor(SDL_DISABLE);	/* hide SDL cursor*/

	//SDL_PumpEvents();	/* SDL bug: must call before output or black window overwrite*/

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
