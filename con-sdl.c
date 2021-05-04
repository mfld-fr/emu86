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

#include "con-sdl.h"
#include "emu-con.h"

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

/* event types*/
#define EVENT_NONE	0
#define EVENT_MOUSE	1
#define EVENT_KBD	2
#define EVENT_QUIT	3

static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;
static SDL_Texture *sdlTexture;
static float sdlZoom = 1.0;
static unsigned char *screen;
static int changed;
static int curx, cury;

int sdl_pollevents(void);
int sdl_pollkbd(void);

static void sdl_drawbitmap(unsigned char c, int x, int y);
static void cursoron(void);
static void cursoroff(void);
static void scrollup(void);

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


int con_get_key (word_t * k)
	{
	MWKEY mwkey;
	MWSCANCODE scancode;
	int ret;

	*k = 0;
	ret = sdl_readkbd(&mwkey, &scancode);
	if (ret == KBD_QUIT) {
		con_term();
		exit(0);
	}

	if (ret != KBD_KEYPRESS) {
printf("KBD: != KBD_KEYPRESS\n");
		return -1;
	}
	if (mwkey & MWKEY_NONASCII_MASK) {
printf("KBD: %x\n", mwkey);
		return 0;
	}

	*k = (word_t)mwkey;
	if (*k == 0x7f) *k = '\b';	// convert DEL to BS

	return 0;
	}


int con_update ()
	{
	if (changed) sdl_draw(0, 0, 0, 0);
	return sdl_pollevents() == EVENT_QUIT;	// return 1 to terminate
	}


int con_poll_key ()
	{
	return sdl_pollkbd();
	}


void con_raw ()
	{
	}


void con_normal ()
	{
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

	SDL_PumpEvents();	/* SDL bug: must call before output or black window overwrite*/

	screen = malloc(HEIGHT * PITCH);
	if (!screen) {
		printf("SDL: Can't malloc framebuffer\n");
		return -1;
	}
	memset(screen, 0x00, HEIGHT*PITCH);
	return 0;
}

/* return nonzero if event available*/
int sdl_pollevents(void)
{
	SDL_Event event;

  	if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
#if 0
		if (event.type >= SDL_MOUSEMOTION && event.type <= SDL_MOUSEWHEEL)
			return EVENT_MOUSE;
		if (event.type >= SDL_FINGERDOWN && event.type <= SDL_FINGERMOTION)
			return EVENT_MOUSE;
		if (event.type >= SDL_KEYDOWN && event.type <= SDL_TEXTINPUT)
			return EVENT_KBD;
#endif
		if (event.type == SDL_KEYDOWN)
			return EVENT_KBD;
		if (event.type == SDL_QUIT)
			return EVENT_QUIT;

		/* dump event*/
  		SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
		//printf("SDL: dumping event %x\n", event.type);
	}
	SDL_PumpEvents();

	return EVENT_NONE;
}

void con_term (void)
{
	free(screen);
	SDL_Quit();
}

/* update SDL from framebuffer*/
void sdl_draw(int x, int y, int width, int height)
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
	changed = 0;
}

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

static void cursoron(void)
{
	/* no simulated cursor in first column because of CR issue*/
	if (curx != 0)
	{
		sdl_drawbitmap('_', curx * CHAR_WIDTH, cury * CHAR_HEIGHT);
		//sdl_draw(curx * CHAR_WIDTH, cury * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
		changed = 1;
	}
}

static void cursoroff(void)
{
	if (curx != 0)
	{
		sdl_drawbitmap(' ', curx * CHAR_WIDTH, cury * CHAR_HEIGHT);
		//sdl_draw(curx * CHAR_WIDTH, cury * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
		changed = 1;
	}
}

static void scrollup(void)
{
	memcpy(screen, screen + CHAR_HEIGHT * PITCH, (LINES-1) * CHAR_HEIGHT * PITCH);
	memset(screen + (LINES-1) * CHAR_HEIGHT * PITCH, 0, CHAR_HEIGHT * PITCH);
	//sdl_draw(0, 0, WIDTH, HEIGHT);
	changed = 1;
}

/* output character at cursor location*/
void sdl_textout(unsigned char c)
{
	changed = 1;
	cursoroff();

	switch (c) {
	case '\0':	return;
	case '\b':	if (--curx <= 0) curx = 0; goto update;
	case '\r':	curx = 0; goto update;
	case '\n':  goto scroll;
	}

	sdl_drawbitmap(c, curx * CHAR_WIDTH, cury * CHAR_HEIGHT);
	//sdl_draw(curx * CHAR_WIDTH, cury * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);

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

/* keyboard handling*/

int sdl_pollkbd(void)
{
	return (sdl_pollevents() >= EVENT_KBD);	/* kbd or quit*/
}

/* convert key code to shift-key code*/
static int
shift_convert(int key)
{
	if (key >= 'a' && key < 127)
		return (key ^ 0x20);			/* upper case*/
	switch (key) {
	case '`':
		return '~';
	case '1':
		return '!';
	case '2':
		return '@';
	case '3':
		return '#';
	case '4':
		return '$';
	case '5':
		return '%';
	case '6':
		return '^';
	case '7':
		return '&';
	case '8':
		return '*';
	case '9':
		return '(';
	case '0':
		return ')';
	case '-':
		return '_';
	case '=':
		return '+';
	case '[':
		return '{';
	case ']':
		return '}';
	case '\\':
		return '|';
	case ';':
		return ':';
	case '\'':
		return '"';
	case ',':
		return '<';
	case '.':
		return '>';
	case '/':
		return '?';
	}
	return key;
}

static int keytrans[] = {
	SDLK_F1,			MWKEY_F1,
	SDLK_F2,			MWKEY_F2,
	SDLK_F3,			MWKEY_F3,
	SDLK_F4,			MWKEY_F4,
	SDLK_F5,			MWKEY_F5,
	SDLK_F6,			MWKEY_F6,
	SDLK_F7,			MWKEY_F7,
	SDLK_F8,			MWKEY_F8,
	SDLK_F9,			MWKEY_F9,
	SDLK_F10,			MWKEY_F10,
	SDLK_F11,			MWKEY_F11,
	SDLK_F12,			MWKEY_F12,
	SDLK_F15,			MWKEY_QUIT,
	SDLK_PRINTSCREEN,	MWKEY_PRINT,
	SDLK_CAPSLOCK,		MWKEY_CAPSLOCK,
	SDLK_SCROLLLOCK,	MWKEY_SCROLLOCK,
	SDLK_PAUSE,			MWKEY_PAUSE,
	SDLK_INSERT,		MWKEY_INSERT,
	SDLK_HOME,			MWKEY_HOME,
	SDLK_PAGEUP,		MWKEY_PAGEUP,
	SDLK_DELETE,		MWKEY_DELETE,
	SDLK_END,			MWKEY_END,
	SDLK_PAGEDOWN,		MWKEY_PAGEDOWN,
	SDLK_RIGHT,			MWKEY_RIGHT,
	SDLK_LEFT,			MWKEY_LEFT,
	SDLK_DOWN,			MWKEY_DOWN,
	SDLK_UP,			MWKEY_UP,
	SDLK_KP_DIVIDE,		MWKEY_KP_DIVIDE,
	SDLK_KP_MULTIPLY,	MWKEY_KP_MULTIPLY,
	SDLK_KP_MINUS,		MWKEY_KP_MINUS,
	SDLK_KP_PLUS,		MWKEY_KP_PLUS,
	SDLK_KP_ENTER,		MWKEY_KP_ENTER,
	SDLK_KP_1,			MWKEY_KP1,
	SDLK_KP_2,			MWKEY_KP2,
	SDLK_KP_3,			MWKEY_KP3,
	SDLK_KP_4,			MWKEY_KP4,
	SDLK_KP_5,			MWKEY_KP5,
	SDLK_KP_6,			MWKEY_KP6,
	SDLK_KP_7,			MWKEY_KP7,
	SDLK_KP_8,			MWKEY_KP8,
	SDLK_KP_9,			MWKEY_KP9,
	SDLK_KP_0,			MWKEY_KP0,
	SDLK_KP_PERIOD,		MWKEY_KP_PERIOD,
	SDLK_KP_EQUALS,		MWKEY_KP_EQUALS,
	SDLK_MENU,			MWKEY_MENU,
	SDLK_SELECT,		MWKEY_SELECTDOWN,
	SDLK_SYSREQ,		MWKEY_SYSREQ,
	SDLK_CANCEL,		MWKEY_CANCEL,
	SDLK_LCTRL,			MWKEY_LCTRL,
	SDLK_LSHIFT,		MWKEY_LSHIFT,
	SDLK_LALT,			MWKEY_LALT,
	SDLK_LGUI,			MWKEY_ALTGR,
	SDLK_RCTRL,			MWKEY_RCTRL,
	SDLK_RSHIFT,		MWKEY_RSHIFT,
	SDLK_RALT,			MWKEY_RALT,
	SDLK_RGUI,			MWKEY_ALTGR,
	SDLK_APP1,			MWKEY_APP1,
	SDLK_APP2,			MWKEY_APP2,
	SDLK_CANCEL,		MWKEY_CANCEL,
	0
};

static int
fnkey_convert(int key)
{
	int *kp = keytrans;

	while (*kp)
	{
		if (key == *kp)
			return *(kp + 1);
		kp += 2;
	}
	return key;
}

/*
 * Read a keystroke event, and the current state of the modifier keys (ALT, SHIFT, etc). 
 * Returns KBD_NODATA, KBD_QUIT, KBD_KEYPRESS or KBD_KEYRELEASE
 * This is a non-blocking call.
 */
int sdl_readkbd(MWKEY *kbuf, MWSCANCODE *scancode)
{
	int mwkey;
	SDL_Scancode sc;
	SDL_Keymod mod;
	SDL_Event event;

	//if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_KEYDOWN, SDL_TEXTINPUT)) { 
	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_KEYDOWN, SDL_KEYDOWN)) { 
		switch (event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			sc = event.key.keysym.scancode;
			mwkey = event.key.keysym.sym;
			mod = SDL_GetModState();

			//printf("key %x,%x %x = %x\n", mwkey, sc, mod, SDL_GetKeyFromScancode(sc));

			// Invariant QWERTY code mapping
			// whatever the keyboard layout

			switch (sc)
				{
				case SDL_SCANCODE_MINUS:  mwkey = '-'; break;
				case SDL_SCANCODE_PERIOD: mwkey = '.'; break;
				case SDL_SCANCODE_SLASH:  mwkey = '/'; break;
				default: break;
				}

			if (mwkey < 256 && (mod & (KMOD_SHIFT|KMOD_CAPS))) {
				mwkey = shift_convert(mwkey);
			}
			if (mwkey < 256 && (mod & KMOD_CTRL)) {
				mwkey &= 0x1f;			/* convert to control char*/
			}

			if (mwkey >= 128) {			/* convert function key from SDL To MW*/
				mwkey = fnkey_convert(mwkey);
				if (mwkey == 0)
					return KBD_NODATA;
			}

			*kbuf = mwkey;		
			*scancode = sc;
			return (event.type == SDL_KEYDOWN)? KBD_KEYPRESS: KBD_KEYRELEASE;

		case SDL_TEXTINPUT:
			mwkey = event.text.text[0];
			return KBD_NODATA;			/* ignore for now*/
		}
	}

	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_QUIT, SDL_QUIT))
		return KBD_QUIT;				/* terminate application*/

	return KBD_NODATA;
}

#if 0
/* mouse handling*/

/*
 * Mouse poll entry point
 */
int sdl_pollmouse(void)
{
	return (sdl_pollevents() == EVENT_MOUSE);
}

/*
 * Read mouse event.
 * Returns MOUSE_NODATA or MOUSE_ABSPOS
 * This is a non-blocking call.
 */

int
sdl_readmouse(int *dx, int *dy, int *dz, int *bp)
{
	int xm, ym;
	int buttons = 0;
	SDL_Event event;
	static int lastx, lasty, lastdn;

	/* handle mouse events*/
	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEWHEEL)) {
		int state = SDL_GetMouseState(&xm, &ym);
//printf("mouseev %x, %d,%d\n", state, xm, ym);
		if (state & SDL_BUTTON(SDL_BUTTON_LEFT))
    		buttons |= MWBUTTON_L;
		if (state & (SDL_BUTTON(SDL_BUTTON_RIGHT) | SDL_BUTTON(SDL_BUTTON_X1)))
			buttons |= MWBUTTON_R;
		if (state & SDL_BUTTON(SDL_BUTTON_MIDDLE))
			buttons |= MWBUTTON_M;

		if (event.type == SDL_MOUSEWHEEL) {
			if (event.wheel.y < 0)
				buttons |= MWBUTTON_SCROLLDN; /* wheel down*/
			else
				buttons |= MWBUTTON_SCROLLUP; /* wheel up*/
		}
		*dx = (int)xm / sdlZoom;
		*dy = (int)ym / sdlZoom;
		*dz = 0;
		*bp = buttons;

		return MOUSE_ABSPOS;
	}

	/* handle touchpad events FIXME need right mouse button support*/
	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_FINGERDOWN, SDL_FINGERMOTION)) {
		if (event.type == SDL_FINGERDOWN) {
			*dx = lastx = (int)(event.tfinger.x * WIDTH / sdlZoom);
			*dy = lasty = (int)(event.tfinger.y * HEIGHT / sdlZoom);
    			lastdn = MWBUTTON_L;
//printf("mousedn %d,%d\n", lastx, lasty);
			*dz = 0;
			*bp = lastdn;
			return MOUSE_NODATA;
		}

		if (event.type == SDL_FINGERUP) {
			*dx = lastx = (int)(event.tfinger.x * WIDTH / sdlZoom);
			*dy = lasty = (int)(event.tfinger.y * HEIGHT / sdlZoom);
			lastdn = 0;
//printf("mouseup %d,%d\n", lastx, lasty);
			*dz = 0;
			*bp = lastdn;
			return MOUSE_NODATA;		/* absolute position*/
		}

		if (event.type == SDL_FINGERMOTION) {
			if (lastdn == 0)
				return MOUSE_NODATA;	/* no motion without finger down*/
			*dx = lastx = (int)(event.tfinger.x * WIDTH / sdlZoom);
			*dy = lasty = (int)(event.tfinger.y * HEIGHT / sdlZoom);
//printf("mousemv %d,%d\n", lastx, lasty);
			*dz = 0;
			*bp = lastdn;

			return MOUSE_NODATA;
		}
	}
	return MOUSE_NODATA;
}
#endif
