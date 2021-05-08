//------------------------------------------------------------------------------
// EMU86 - ELKS ROM stub (BIOS)
//------------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-con.h"
#include "emu-int.h"
#include "mem-io-elks.h"

extern int info_level;


// BIOS video services

static byte_t num_hex (byte_t n)
	{
	byte_t h = n + '0';
	if (h > '9') h += 'A' - '9';
	return h;
	}


static int int_10h ()
	{
	byte_t ah = reg8_get (REG_AH);

	byte_t al;

	word_t es;
	word_t bp;
	word_t cx;

	addr_t a;
	byte_t r;  // row
	byte_t c;  // column

//printf("INT 10h fn %x\n", ah);
	switch (ah)
		{
		// Set cursor position

		case 0x02:
			r = reg8_get (REG_DH);  // row
			c = reg8_get (REG_DL);  // column
			con_pos_set (r,c);
			break;

		// Get cursor position

		case 0x03:
			con_pos_get(&r, &c);
			reg16_set (REG_DX, (r << 8) | c);
			reg16_set (REG_CX, 0);  // null cursor
			break;

		// Select active page

		case 0x05:
			break;

		// Scroll up

		case 0x06:
			con_scrollup();			// ignores row/col numbers
			break;

		// Write character at current cursor position

		case 0x09:
		case 0x0A:
			con_put_char (reg8_get (REG_AL));
			break;

		// Write as teletype to current page
		// Page ignored in video mode 7

		case 0x0E:
			con_put_char (reg8_get (REG_AL));
			break;

		// Get video mode

		case 0x0F:
			reg8_set (REG_AL, 7);   // text monochrome 80x25
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
				{
				con_put_char (mem_read_byte (a++));
				}

			break;

		// Write byte as hexadecimal

		case 0x1D:
			al = reg8_get (REG_AL);
			c = num_hex (al >> 4);
			con_put_char (c);
			c = num_hex (al & 0x0F);
			con_put_char (c);
			break;

		default:
		notimp:
			fflush(stdout);
			printf ("fatal: INT 10h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
	}


// BIOS device list

static int int_11h ()
	{
	// 1 floppy drive
	// 80x25 monochrome

	reg16_set (REG_AX, 0x0031);
	return 0;
	}


// BIOS memory services

static int int_12h ()
	{
	// 512 KiB of low memory
	// no extended memory

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
		ssize_t (*op)();

		off_t o = lseek (dp->fd, lba * SECTOR_SIZE, SEEK_SET);
		if (o == -1) break;

		op = read;
		if (wflag) op = write;
		l = (*op) (dp->fd, mem_get_addr (addr_seg_off (seg, off)), SECTOR_SIZE);
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
	int err = -1;
	struct diskinfo *dp;

	switch (ah)
		{
		case 0x00:  // reset drive
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
				printf("INT 13h AH=%hxh: invalid DCHS %d/%d/%d/%d\n", ah, d, c, h, s);
				break;
				}

			if (s + n > dp->sectors + 1)
				{
				printf("INT 13h AH=%hxh: multi-track I/O operation rejected\n", ah);
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

		default:
			printf ("fatal: INT 13h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	flag_set (FLAG_CF, err? 1: 0);
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

			reg8_set (REG_AH, 0);  // No scan code
			break;

		// Peek character

		case 0x01:
		case 0x11:
			if (con_poll_key ())
				{
				flag_set (FLAG_ZF, 0);

				err = con_get_key (&key_prev);
				if (err) break;

				reg8_set (REG_AL, (byte_t) key_prev);
				reg8_set (REG_AH, 0);
				}
			else
				{
				flag_set (FLAG_ZF, 1);  // no character in queue
				}

			break;

		// Set typematic rate - ignore

		case 0x03:
			break;

		// Extended keyboard read

		case 0x10:
			err = con_get_key (&k);
			if (err) break;

			reg8_set (REG_AL, (byte_t) k);  // ASCII code
			reg8_set (REG_AH, 0);           // No scan code
			break;

		default:
			printf ("error: INT 16h: AH=%hXh not implemented\n", ah);
			err = -1;
		}

	return err;
	}


// BIOS printer services

static int int_17h ()
	{
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		default:
			printf ("fatal: INT 17h: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
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
	byte_t ah = reg8_get (REG_AH);
	switch (ah)
		{
		// Get system time

		case 0x00:
			reg16_set (REG_CX, 0);  // stay on 0
			reg16_set (REG_DX, 0);
			reg8_set (REG_AL, 0);   // no day wrap
			break;

		default:
			printf ("fatal: INT 1Ah: AH=%hxh not implemented\n", ah);
			assert (0);
		}

	return 0;
	}


// Interrupt handler table

int_num_hand_t _int_tab [] = {
	{ 0x03, int_03h },
	{ 0x10, int_10h },
	{ 0x11, int_11h },
	{ 0x12, int_12h },
	{ 0x13, int_13h },  // BIOS disk services
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
	// ELKS saves and calls initial INT 08h (timer)
	// So implement a stub for INT 08h that just EOI
	// Starting @ F000:1000h

	mem_write_byte (0xF1000, 0xB0, 1);  // MOV AL,20h
	mem_write_byte (0xF1001, 0x20, 1);
	mem_write_byte (0xF1002, 0xE6, 1);  // OUT 20h,AL
	mem_write_byte (0xF1003, 0x20, 1);
	mem_write_byte (0xF1004, 0xCF, 1);  // IRET

	// Point vector 08h to that stub

	mem_write_word (0x08*4+0, 0x1000, 1);
	mem_write_word (0x08*4+2, 0xF000, 1);

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

	memset (mem_stat+BDA_BASE, 0x00, 256);
	*(byte_t *) (mem_stat+BDA_BASE+0x49) =  3; 				// video mode (7=MDA)
	*(byte_t *) (mem_stat+BDA_BASE+0x4a) =  VID_COLS;		// console width
	*(word_t *) (mem_stat+BDA_BASE+0x4c) =  VID_PAGE_SIZE;	// page size
	*(word_t *) (mem_stat+BDA_BASE+0x63) =  CRTC_CTRL_PORT;	// 6845 CRTC
	}
