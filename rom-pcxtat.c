//------------------------------------------------------------------------------
// EMU86 - PC/XT/AT ROM stub (BIOS)
//------------------------------------------------------------------------------

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-con.h"
#include "emu-int.h"

#include "mem-io-pcxtat.h"
#include "rom-bios.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>


extern int info_level;


// BIOS video services

static int int_10h ()
	{
	int err = 0;

	byte_t ah = reg8_get (REG_AH);

	byte_t al;

	word_t es;
	word_t bp;
	word_t cx;

	addr_t a;
	byte_t r;  // row
	byte_t c;  // column
	byte_t r2, c2, n, at;

	switch (ah)
		{
		// Set video mode

		case 0x00:
			al = reg8_get (REG_AL);  // mode
			if (al != 0x03 && al != 0x07)  // 80x25 color or monochrome
				{
				printf ("\nerror: INT 10h AH=00h: unsupported mode %hhXh\n", al);
				err = -1;
				}

			mem_stat [BDA_VIDEO_MODE] = al;
			break;

		// Set cursor type

		case 0x01:
			//puts ("\nwarning: INT 10h AH=01h: no cursor type supported");
			break;

		// Set cursor position

		case 0x02:
			r = reg8_get (REG_DH);  // row
			c = reg8_get (REG_DL);  // column
			con_pos_set (r,c);
			break;

		// Get cursor position

		case 0x03:
			con_pos_get (&r, &c);
			reg16_set (REG_DX, (r << 8) | c);
			reg16_set (REG_CX, 0);  // null cursor
			break;

		// Select active page

		case 0x05:
			break;

		// Scroll up/down

		case 0x06:		// up
		case 0x07:		// down
			n = reg8_get (REG_AL); 	// # lines
			at = reg8_get (REG_BH); // attribute
			r = reg8_get (REG_CH);  // upper L/R
			c = reg8_get (REG_CL);
			r2 = reg8_get (REG_DH);	// lower L/R
			c2 = reg8_get (REG_DL);
			con_scroll (ah == 0x07, n, at, r, c, r2, c2);
			break;

		// Read character at current cursor position

		case 0x08:
			reg16_set (REG_AX, ATTR_DEFAULT);  // dummy character & attribute
			break;

		// Write character & attribute at current cursor position

		case 0x09:
			con_put_char (reg8_get (REG_AL), reg8_get (REG_BL));
			break;

		// Write character only at current cursor position

		case 0x0A:
			con_put_char (reg8_get (REG_AL), ATTR_DEFAULT);
			break;

		// Set color palette

		case 0x0B:
			puts ("\nwarning: INT 10h AH=0Bh: no palette supported");
			break;

		// Write as teletype to current page
		// Page ignored

		case 0x0E:
			con_put_char (reg8_get (REG_AL), ATTR_DEFAULT);
			break;

		// Get video mode

		case 0x0F:
			reg8_set (REG_AL, mem_stat [BDA_VIDEO_MODE]);
			reg8_set (REG_AH, 80);  // 80 columns
			reg8_set (REG_BH, 0);   // page 0 active
			break;

		// Get EGA video configuration

		case 0x12:
			reg8_set (REG_BH, 1);	// mono mode
			reg8_set (REG_BL, 0);	// 64k EGA
			reg8_set (REG_CH, 0);	// feature bits
			reg8_set (REG_CL, 0);	// switch settings
			break;

		// Get VGA video configuration

		case 0x1A:
			if (reg8_get(REG_AL) != 0x00)
				goto notimp;
			reg8_set (REG_AL, 0);	// no VGA
			break;

		// Write string

		case 0x13:
			es = seg_get (SEG_ES);
			bp = reg16_get (REG_BP);
			cx = reg16_get (REG_CX);
			a = addr_seg_off (es, bp);

			while (cx--)
				con_put_char (mem_read_byte (a++), ATTR_DEFAULT);

			break;

		default:
		notimp:
			printf ("\nerror: INT 10h AH=%hhXh not implemented\n", ah);
			err = -1;
		}

	return err;
	}


// BIOS device list

static int int_11h ()
	{
	// 1 floppy drive
	// 80x25 monochrome <- FIXME: default is 80x25 16 color (EGA)

	reg16_set (REG_AX, 0x0031);
	return 0;
	}


// BIOS memory services

static int int_12h ()
	{
	// 512 KiB of low memory
	// no extended memory

	// TODO: extend to 640K for PC

	reg16_set (REG_AX, 512);
	return 0;
	}


// Image info

struct diskinfo
	{
	byte_t drive;
	word_t cylinders;
	byte_t heads;
	byte_t sectors;
	int fd;
	};

struct diskinfo diskinfo[4] = {
{    0, 0, 0, 0, -1 },
{    0, 0, 0, 0, -1 },
{    0, 0, 0, 0, -1 },
{    0, 0, 0, 0, -1 }
};

#define SECTOR_SIZE 512

static struct diskinfo * find_drive (byte_t drive)
	{
	struct diskinfo *dp;

	for (dp = diskinfo; dp < &diskinfo[sizeof(diskinfo)/sizeof(diskinfo[0])]; dp++)
		if (dp->fd != -1 && dp->drive == drive)
			return dp;
	return NULL;
	}


int rom_image_load (char * path)
	{
	byte_t d = 0, h, s;
	word_t c;
	struct diskinfo *dp;
	struct stat sbuf;

	int fd = open (path, O_RDWR);
	if (fd < 0)
		{
		printf ("Can't open disk image: %s\n", path);
		return 1;
		}
	if (fstat (fd, &sbuf) < 0)
		{
		perror(path);
		return 1;
		}
	off_t size = sbuf.st_size / 1024;
	switch (size)
		{
		case 360:   c = 40; h = 2; s =  9; break;
		case 720:   c = 80; h = 2; s =  9; break;
		case 1440:  c = 80; h = 2; s = 18; break;
		case 2880:  c = 80; h = 2; s = 36; break;
		default:
			if (size < 2880)
				{
				printf ("Image size not supported: %s\n", path);
				close (fd);
				return 1;
				}
			d = 0x80;
			if (size <= 1032192UL)	// = 1024 * 16 * 63
				{
				c = 63; h = 16; s = 63;
				}
			else
				{
				c = (size / 4032) + 1;
				h = 64;
				s = 63;
				}
			break;
		}

		for (dp = diskinfo; dp < &diskinfo[sizeof(diskinfo)/sizeof(diskinfo[0])]; dp++)
			{
			if (dp->fd == -1)
				{
				if (find_drive (d)) d++;
				dp->drive = d;
				dp->cylinders = c;
				dp->heads = h;
				dp->sectors = s;
				dp->fd = fd;
				printf ("info: disk image %s (DCHS %d/%d/%d/%d)\n", path, d, c, h, s);
				return 0;
				}
			}

		printf("Too many disk images: %s ignored\n", path);
		close (fd);
		return 1;
	}

void rom_term (void)
		{
		struct diskinfo *dp;
		for (dp = diskinfo; dp < &diskinfo[sizeof(diskinfo)/sizeof(diskinfo[0])]; dp++)
			{
			if (dp->fd != -1)
				close (dp->fd);
			}
		}


// Read/write disk

static int readwrite_sector (byte_t drive, int wflag, unsigned long lba,
			word_t seg, word_t off)
	{
	int err = -1;
	struct diskinfo *dp = find_drive (drive);
	if (!dp) return err;

	while (1)
		{
		ssize_t l;

		off_t o = lseek (dp->fd, lba * SECTOR_SIZE, SEEK_SET);
		if (o == -1) break;

		if (wflag) l = write(dp->fd, mem_get_addr (addr_seg_off (seg, off)), SECTOR_SIZE);
		else l = read(dp->fd, mem_get_addr (addr_seg_off (seg, off)), SECTOR_SIZE);
		if (l != SECTOR_SIZE) break;

		// success

		err = 0;
		break;
		}

	return err;
	}


// BIOS disk services

static int int_13h ()
	{
	byte_t ah = reg8_get (REG_AH);
	byte_t d, h, s, n;
	word_t c, seg, off;
	// FIXME: make emulator and BIOS errors distinct
	int err = -1;
	struct diskinfo *dp;

	switch (ah)
		{
		// Reset drive

		case 0x00:
			err = 0;
			break;

		case 0x02:  // read disk
		case 0x03:  // write disk
			d = reg8_get (REG_DL);        // drive
			c = reg8_get (REG_CH) | ((reg8_get (REG_CL) & 0xC0) << 2);
			h = reg8_get (REG_DH);        // head
			s = reg8_get (REG_CL) & 0x3F; // sector, base 1
			n = reg8_get (REG_AL);        // count
			seg = seg_get (SEG_ES);
			off = reg16_get (REG_BX);

			if (info_level & 1) printf ("%s: DCHS %d/%d/%d/%d @%x:%x, %d sectors\n",
				ah==2? "read_sector": "write_sector", d, c, h, s, seg, off, n);

			dp = find_drive (d);
			if (!dp || c >= dp->cylinders || h >= dp->heads || s > dp->sectors)
				{
				printf("INT 13h AH=%hhXh: invalid DCHS %d/%d/%d/%d\n", ah, d, c, h, s);
				break;
				}

			if (s + n > dp->sectors + 1)
				{
				printf("INT 13h AH=%hhXh: multi-track I/O operation rejected\n", ah);
				break;
				}

			off_t lba = (s-1) + dp->sectors * (h + c * dp->heads);
			err = 0;
			while (n-- != 0)
				{
				err |= readwrite_sector (d, ah==3, lba, seg, off);
				lba++;
				off += SECTOR_SIZE;
				}
			break;

		// Verify sectors

		case 0x04:
			reg8_set (REG_AH, 0);  // 'no error' status
			err = 0;
			break;

		case 0x08:  // get drive parms
			d = reg8_get (REG_DL);
			dp = find_drive (d);
			if (!dp) break;
			c = dp->cylinders - 1;
			n = find_drive (d+1)? 2: 1;
			reg8_set (REG_BL, 4);   // CMOS 1.44M floppy
			reg8_set (REG_DL, n);   // # drives
			reg8_set (REG_CH, c & 0xFF);
			reg8_set (REG_CL, dp->sectors | (((c >> 8) & 0x03) << 6));
			reg8_set (REG_DH, dp->heads - 1);
			seg_set (SEG_ES, 0xFF00);   // fake DDPT, same as INT 1Eh
			reg16_set (REG_DI, 0x0000);
			err = 0;
			break;

		// Read DASD type
		// Unsupported on old PC

		case 0x15:
			puts ("\nwarning: INT 13h AH=15h: no DASD type");
			reg8_set (REG_AH, 0x01);  // 'invalid command' status
			break;

		default:
			printf ("\nerror: INT 13h AH=%hhXh not implemented\n", ah);
			return -1;
		}

	flag_set (FLAG_CF, err? 1: 0);
	return 0;
	}


// BIOS asynchronous communication services

static int int_14h ()
	{
	int err = 0;

	byte_t ah = reg8_get (REG_AH);

	switch (ah)
		{
		// Initialize port

		case 0x00:
			reg16_set(REG_AX, 0x0000); // dummy modem & line statuses
			break;

		default:
			printf ("\nerror: INT 14h AH=%hhXh not implemented\n", ah);
			err = -1;
		}

	return err;
	}


// BIOS misc services

static int int_15h ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Return CF=1 for all non implemented functions
		// as recommended by Alan Cox on the ELKS mailing list

		default:
			flag_set (FLAG_CF, 1);

		}

	return 0;
	}


// BIOS keyboard services

// Reverse scancode table, so we have a scancode to return with the ASCII
// TODO: move to a keyboard driver

static const unsigned char scancodes[] = {
	// NUL SOH   STX   ETX   EOT   ENQ   ACK   BEL    BS    HT    LF    VT    FF    CR    SO    SI
	0x03, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x0E, 0x0F, 0x24, 0x25, 0x26, 0x1C, 0x31, 0x18,
	// DLE DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN    EM   SUB   ESC    FS    GS    RS    US
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x01, 0x2B, 0x1B, 0x07, 0x0C,
	// SP    !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
	0x39, 0x02, 0x28, 0x04, 0x05, 0x06, 0x08, 0x28, 0x0A, 0x0B, 0x09, 0x0D, 0x33, 0x0C, 0x34, 0x35,
	// 0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
	0x0B, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x27, 0x27, 0x33, 0x0D, 0x34, 0x35,
	// @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
	0x03, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	// P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B, 0x07, 0x0C,
	// `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
	0x29, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	// p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~   DEL
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B, 0x29, 0x53
};


static word_t key_prev = 0;

static int int_16h ()
	{
	int err = 0;

	word_t k = 0;

	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Normal keyboard read

		// WARNING: non-blocking call with SDL console
		// while blocking in official BIOS.

		case 0x00:
		case 0x10:
			if (key_prev)
				{
				reg8_set (REG_AL, (byte_t) key_prev);
				key_prev = 0;
				}
			else
				{
				err = con_get_key (&k);
				if (err) break;

				reg8_set (REG_AL, (byte_t) k);  // ASCII code
				}

			// Convert ASCII to scan code

			convert:

			if (reg8_get (REG_AL) < 0x80)
				reg8_set (REG_AH, scancodes [reg8_get(REG_AL)]);
			else
				reg8_set (REG_AH, 0);  // no scan code

			break;

		// Peek character

		case 0x01:
		case 0x11:
			// Do we have a character previously read?
			if (key_prev == 0)
				{
				// Nope, ask the console for a new one
				if (!con_poll_key ())
					{
						flag_set (FLAG_ZF, 1);  // no character in queue
						break;
					}

				err = con_get_key (&key_prev);
				if (err) break;
				}

			flag_set (FLAG_ZF, 0);
			reg8_set (REG_AL, (byte_t) key_prev);
			goto convert;

		// Get key modifier status

		case 0x02:
			reg8_set (REG_AL, 0);  // no modifier
			break;

		// Set typematic rate - ignore

		case 0x03:
			break;

		default:
			printf ("\nerror: INT 16h AH=%hhXh not implemented\n", ah);
			err = -1;
		}

	return err;
	}


// BIOS printer services

static int int_17h ()
	{
	int err = 0;

	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Initialize Printer Port

		case 0x1:
			reg8_set (REG_AH, 0x00); // dummy printer status
			break;

		default:
			printf ("\nerror: INT 17h AH=%hhXh not implemented\n", ah);
			err = -1;
		}

	return err;
	}


// Disk boot (internal)

static int int_FEh ()
	{
	// Boot from first sector of first disk
	byte_t d = diskinfo[0].drive;

	if (readwrite_sector (d, 0, 0L, 0x0000, 0x7C00)) return -1;

	reg8_set (REG_DL, d);  // DL = BIOS boot drive number
	seg_set (SEG_CS, 0x0000);
	reg16_set (REG_IP, 0x7C00);

	return 0;
	}


// BIOS time services

static int int_1Ah ()
	{
	int err = 0;

	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Get system time

		case 0x00:
			reg16_set (REG_CX, 0);  // stay on 0
			reg16_set (REG_DX, 0);
			reg8_set (REG_AL, 0);   // no day wrap
			break;

		// Set system time

		case 0x01:
			break;

		// Read time from RTC

		case 0x02:
			reg8_set (REG_CH, 0x12);  // hours (in BCD)
			reg8_set (REG_CL, 0x34);  // minutes (in BCD)
			reg8_set (REG_DH, 0x56);  // seconds (in BCD)
			reg8_set (REG_DL, 0);     // no daylight savings time
			break;

		// Set time in RTC

		case 0x03:
			break;

		// Read date from RTC

		case 0x04:
			reg8_set (REG_CH, 0x20);  // century (in BCD)
			reg8_set (REG_CL, 0x21);  // year (in BCD)
			reg8_set (REG_DH, 0x05);  // month (in BCD)
			reg8_set (REG_DL, 0x09);  // day (in BCD)
			break;

		// Set date in RTC

		case 0x05:
			break;

		default:
			printf ("\nerror: INT 1Ah AH=%hhXh not implemented\n", ah);
			err = -1;
		}

	return err;
	}


// Interrupt handler table

int_num_hand_t _int_tab [] = {
	{ 0x03, int_03h },
	{ 0x10, int_10h },  // BIOS video services
	{ 0x11, int_11h },  // BIOS equipment service
	{ 0x12, int_12h },
	{ 0x13, int_13h },  // BIOS disk services
	{ 0x14, int_14h },  // BIOS asynchronous communication services
	{ 0x15, int_15h },
	{ 0x16, int_16h },
	{ 0x17, int_17h },
//	{ 0x19, int_19h },  // OS boot
	{ 0x1A, int_1Ah },
	{ 0xFE, int_FEh },  // disk boot
//	{ 0xFF, int_FFh },  // reserved for MON86
	{ 0,    NULL    }
	};

// BIOS boot sequence starts @ F000:0h

static addr_t bios_boot = 0xF0000;

// BIOS extension check

static void bios_check (addr_t b, int size)
	{
	byte_t sum = 0;

	for (addr_t a = b; a < (b + size); a++)
		sum += mem_read_byte (a);

	if (sum == 0)
		{
		// Append a far call to any detected extension to the BIOS boot sequence

		mem_write_byte (bios_boot, 0x9A, 1);  // CALLF
		mem_write_word (bios_boot + 1, 0x0003, 1);
		mem_write_word (bios_boot + 3, b >> 4, 1);
		bios_boot += 5;
		}
	}


// ROM stub (BIOS) initialization

void rom_init (void)
	{
	// Generic BIOS

	rom_init_0 ();

	// Point vector 1Eh to a dummy disk drive parameter table (DDPT)
	// Required by the disk boot sector
	// Starting @ F000:2000h

	mem_write_word (0x1E*4+0, 0x2000, 1);
	mem_write_word (0x1E*4+2, 0xF000, 1);

	// Implement a default INT 19h handler
	// that redirects to INT FEh for disk boot
	// Starting @ F000:3000h

	mem_write_byte (0xF3000, 0xCD, 1);  // INT FEh
	mem_write_byte (0xF3001, 0xFE, 1);
	mem_write_byte (0xF3002, 0xCF, 1);  // IRET

	// Point vector 19h to that stub

	mem_write_word (0x19*4+0, 0x3000, 1);
	mem_write_word (0x19*4+2, 0xF000, 1);

	// Scan ROM area for any BIOS extension
	// Any extension must be aligned to 2K

	for (addr_t a = 0xC8000; a < 0xE0000; a += 0x00800)
		{
		if (mem_read_word (a) == 0xAA55)
			{
			int len = mem_read_byte (a + 2) * 512;
			bios_check (a, len);
			}
		}

	// Any extension @ E000:0h must be 64K

	if (mem_read_word (0xE0000) == 0xAA55)
		{
		bios_check (0xE0000, 0x10000);
		}

	// End BIOS boot initialization sequence with INT 19h for OS boot

	mem_write_byte (bios_boot++, 0xCD, 1);  // INT 19h
	mem_write_byte (bios_boot++, 0x19, 1);

	// CPU boot starts @ FFFF:0h
	// Jump to BIOS boot

	mem_write_byte (0xFFFF0, 0xEA,   1);  // JMPF
	mem_write_word (0xFFFF1, 0x0000, 1);
	mem_write_word (0xFFFF3, 0xF000, 1);

	// BIOS Data Area (BDA) setup for EGA/MDA adaptors

	mem_stat [BDA_VIDEO_MODE] = 3;  // video mode (3=EGA 7=MDA)
	*(byte_t *) (mem_stat+BDA_BASE+0x4a) =  VID_COLS;		// console width
	*(word_t *) (mem_stat+BDA_BASE+0x4c) =  VID_PAGE_SIZE;	// page size
	*(word_t *) (mem_stat+BDA_BASE+0x63) =  CRTC_CTRL_PORT;	// 6845 CRTC

	*(byte_t *) (mem_stat+BDA_BASE+0x10) =  0x81;			// 1 floppy

	memset (mem_stat+vid_base(), 0x00, VID_PAGE_SIZE);		// clear text RAM
	}
