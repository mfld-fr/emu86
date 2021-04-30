/* serial-sdl.h*/

int sdl_init(void);
void sdl_close(void);
int sdl_pollevents(void);
void sdl_draw(int x, int y, int width, int height);
void sdl_textout(unsigned char c);

/* graphics*/
typedef unsigned short MWIMAGEBITS;/* bitmap image unit size*/

/* MWIMAGEBITS macros*/
#define MWIMAGE_WORDS(x)	(((x)+15)/16)
#define MWIMAGE_BYTES(x)	(MWIMAGE_WORDS(x)*sizeof(MWIMAGEBITS))
/* size of image in words*/
#define	MWIMAGE_SIZE(width, height)  	\
	((height) * (((width) + MWIMAGE_BITSPERIMAGE - 1) / MWIMAGE_BITSPERIMAGE))
#define	MWIMAGE_BITSPERIMAGE	(sizeof(MWIMAGEBITS) * 8)
#define	MWIMAGE_BITVALUE(n)	((MWIMAGEBITS) (((MWIMAGEBITS) 1) << (n)))
#define	MWIMAGE_FIRSTBIT	(MWIMAGE_BITVALUE(MWIMAGE_BITSPERIMAGE - 1))
#define	MWIMAGE_NEXTBIT(m)	((MWIMAGEBITS) ((m) >> 1))
#define	MWIMAGE_TESTBIT(m)	((m) & MWIMAGE_FIRSTBIT)  /* use with shiftbit*/
#define	MWIMAGE_SHIFTBIT(m)	((MWIMAGEBITS) ((m) << 1))  /* for testbit*/

/* keyboard*/
typedef unsigned int MWKEYMOD;
typedef unsigned short MWKEY;
typedef unsigned short MWSCANCODE;

/* keyboard read return codes*/
#define KBD_FAIL			-1			/* read failed, no data*/
#define KBD_QUIT			-2			/* quit key pressed, terminate*/
#define KBD_NODATA			0			/* no data returned*/
#define KBD_KEYPRESS		1			/* key pressed*/
#define KBD_KEYRELEASE		2			/* key released*/

#define MWKEY_UNKNOWN		0
/* Following special control keysyms are mapped to ASCII*/
#define MWKEY_BACKSPACE		8
#define MWKEY_TAB		9
#define MWKEY_ENTER		13
#define MWKEY_ESCAPE		27
/* Keysyms from 32-126 are mapped to ASCII*/

#define MWKEY_NONASCII_MASK	0xFF00
/* Following keysyms are mapped to private use portion of Unicode-16*/
/* arrows + home/end pad*/
#define MWKEY_FIRST		0xF800
#define MWKEY_LEFT		0xF800
#define MWKEY_RIGHT		0xF801
#define MWKEY_UP		0xF802
#define MWKEY_DOWN		0xF803
#define MWKEY_INSERT		0xF804
#define MWKEY_DELETE		0xF805
#define MWKEY_HOME		0xF806
#define MWKEY_END		0xF807
#define MWKEY_PAGEUP		0xF808
#define MWKEY_PAGEDOWN		0xF809

/* Numeric keypad*/
#define MWKEY_KP0		0xF80A
#define MWKEY_KP1		0xF80B
#define MWKEY_KP2		0xF80C
#define MWKEY_KP3		0xF80D
#define MWKEY_KP4		0xF80E
#define MWKEY_KP5		0xF80F
#define MWKEY_KP6		0xF810
#define MWKEY_KP7		0xF811
#define MWKEY_KP8		0xF812
#define MWKEY_KP9		0xF813
#define MWKEY_KP_PERIOD		0xF814
#define MWKEY_KP_DIVIDE		0xF815
#define MWKEY_KP_MULTIPLY	0xF816
#define MWKEY_KP_MINUS		0xF817
#define MWKEY_KP_PLUS		0xF818
#define MWKEY_KP_ENTER		0xF819
#define MWKEY_KP_EQUALS		0xF81A

/* Function keys */
#define MWKEY_F1		0xF81B
#define MWKEY_F2		0xF81C
#define MWKEY_F3		0xF81D
#define MWKEY_F4		0xF81E
#define MWKEY_F5		0xF81F
#define MWKEY_F6		0xF820
#define MWKEY_F7		0xF821
#define MWKEY_F8		0xF822
#define MWKEY_F9		0xF823
#define MWKEY_F10		0xF824
#define MWKEY_F11		0xF825
#define MWKEY_F12		0xF827

/* Key state modifier keys*/
#define MWKEY_NUMLOCK		0xF828
#define MWKEY_CAPSLOCK		0xF829
#define MWKEY_SCROLLOCK		0xF82A
#define MWKEY_LSHIFT		0xF82B
#define MWKEY_RSHIFT		0xF82C
#define MWKEY_LCTRL		0xF82D
#define MWKEY_RCTRL		0xF82E
#define MWKEY_LALT		0xF82F
#define MWKEY_RALT		0xF830
#define MWKEY_LMETA		0xF831
#define MWKEY_RMETA		0xF832
#define MWKEY_ALTGR		0xF833

/* Misc function keys*/
#define MWKEY_PRINT		0xF834
#define MWKEY_SYSREQ		0xF835
#define MWKEY_PAUSE		0xF836
#define MWKEY_BREAK		0xF837
#define MWKEY_QUIT		0xF838	/* virtual key*/
#define MWKEY_MENU		0xF839	/* virtual key*/
#define MWKEY_REDRAW		0xF83A	/* virtual key*/

/* Handheld function keys*/
/* #define MWKEY_RECORD		0xF840 -- Replaced by HAVi code */
/* #define MWKEY_PLAY		0xF841 -- Replaced by HAVi code */
#define MWKEY_CONTRAST		0xF842
#define MWKEY_BRIGHTNESS	0xF843
#define MWKEY_SELECTUP		0xF844
#define MWKEY_SELECTDOWN	0xF845
#define MWKEY_ACCEPT		0xF846
#define MWKEY_CANCEL		0xF847
#define MWKEY_APP1		0xF848
#define MWKEY_APP2		0xF849
#define MWKEY_APP3              0xF84A
#define MWKEY_APP4              0xF84B
#define MWKEY_SUSPEND           0xF84C
#define MWKEY_END_NORMAL	0xF84D	/* insert additional keys before this*/

/* keyboard state modifiers*/
#define MWKMOD_NONE  		0x0000
#define MWKMOD_LSHIFT		0x0001
#define MWKMOD_RSHIFT		0x0002
#define MWKMOD_LCTRL 		0x0040
#define MWKMOD_RCTRL 		0x0080
#define MWKMOD_LALT  		0x0100
#define MWKMOD_RALT  		0x0200
#define MWKMOD_LMETA 		0x0400		/* Windows key*/
#define MWKMOD_RMETA 		0x0800		/* Windows key*/
#define MWKMOD_NUM   		0x1000
#define MWKMOD_CAPS  		0x2000
#define MWKMOD_ALTGR 		0x4000
#define MWKMOD_SCR			0x8000

#define MWKMOD_CTRL	(MWKMOD_LCTRL|MWKMOD_RCTRL)
#define MWKMOD_SHIFT	(MWKMOD_LSHIFT|MWKMOD_RSHIFT)
#define MWKMOD_ALT	(MWKMOD_LALT|MWKMOD_RALT)
#define MWKMOD_META	(MWKMOD_LMETA|MWKMOD_RMETA)

int sdl_readkbd(MWKEY *kbuf, MWKEYMOD *modifiers, MWSCANCODE *scancode);
int sdl_pollkbd(void);

/* mouse*/

/* mouse read return codes*/
#define MOUSE_FAIL			-1			/* read failed, no data*/
#define MOUSE_NODATA		0			/* no data returned*/
#define MOUSE_RELPOS		1			/* relative position returned*/
#define MOUSE_ABSPOS		2			/* absolute position returned*/
#define MOUSE_NOMOVE		3			/* abs positioned returned, don't move mouse cursor*/

/* Mouse button bits, compatible with Windows*/
#define MWBUTTON_L		  0x01		/* left button*/
#define MWBUTTON_R		  0x02		/* right button*/
#define MWBUTTON_M		  0x10		/* middle*/
#define MWBUTTON_SCROLLUP 0x20		/* wheel up*/
#define MWBUTTON_SCROLLDN 0x40		/* wheel down*/

int sdl_readmouse(int *dx, int *dy, int *dz, int *bp);
int sdl_pollmouse(void);
