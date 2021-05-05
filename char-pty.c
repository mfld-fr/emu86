//-------------------------------------------------------------------------------
// EMU86 - PTY character backend
//-------------------------------------------------------------------------------

#define _DEFAULT_SOURCE    // for cfmakeraw()
#define _XOPEN_SOURCE 600  // for posix_openpt() & ptsname()

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#include "emu-char.h"


static int _ptm = -1;


int char_send (byte_t c)
	{
	int err = 0;

	if (_ptm >= 0)
		{
		int n = write (_ptm, &c, 1);
		if (n != 1)
			{
			perror ("warning: cannot write to PTM:");
			err = -1;
			}
		}

	return err;
	}


int char_recv (byte_t * c)
	{
	int err = 0;

	if (_ptm >= 0)
		{
		int n = read (_ptm, c, 1);
		if (n != 1)
			{
			perror ("warning: cannot read from PTM:");
			err = -1;
			}
		}

	return err;
	}


int char_poll ()
	{
	fd_set fdsr;
	FD_ZERO (&fdsr);
	FD_SET (_ptm, &fdsr);
	struct timeval tv = { 0L, 0L };  // immediate
	int s = select (_ptm + 1, &fdsr, NULL, NULL, &tv);
	if (s < 0) return -1;
	if (FD_ISSET (_ptm, &fdsr)) return 1;  // has char
	return 0;  // no char
	}


void char_raw ()
	{
	}

void char_normal ()
	{
	}


int char_init ()
	{
	while (1)
		{
		// Create pseudo terminal for serial emulation

		_ptm = posix_openpt (O_RDWR);
		if (_ptm < 0)
			{
			perror ("warning: cannot create PTM:");
			break;
			}

		// The following functions return -1 on success !?!
		grantpt (_ptm);
		unlockpt (_ptm);

		// Set slave in raw mode (avoid echo and other cooking)

		struct termios tios;
		tcgetattr (_ptm, &tios);
		cfmakeraw (&tios);
		tcsetattr (_ptm, TCSANOW, &tios);

		char * path = ptsname (_ptm);
		printf ("info: PTS for serial emulation: %s\n", path);

		int f = open ("emu86.pts", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		write (f, path, strlen (path));
		close (f);

		break;
		}

	return 0;
	}


void char_term ()
	{
	unlink ("emu86.pts");

	// Close pseudo terminal

	if (_ptm >= 0)
		{
		close (_ptm);
		_ptm = -1;
		}
	}
