#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "emu-char.h"

static struct termios def_termios;

int char_send (byte_t c)
	{
	int err = 0;

	int n = write (1, &c, 1);
	if (n != 1)
		{
		perror ("warning: cannot write to stdio");
		err = -1;
		}

	return err;
	}


int char_recv (byte_t * c)
	{
	int err = 0;

	if (!char_poll())
		{
		fd_set fdsr;
		FD_ZERO (&fdsr);
		FD_SET (0, &fdsr);
		int s = select (1, &fdsr, NULL, NULL, NULL);
		if (s < 0)
			return -1;		// required for updated source when ^C hit on read
		}

	int n = read (0, c, 1);
	if (n == 0) return 0;
	if (n != 1)
		{
		perror ("warning: cannot read from console");
		err = -1;
		}

	if (*c == 0x7f) *c = '\b';	// convert DEL to BS

	return err;
	}


int char_poll ()
	{
	fd_set fdsr;
	FD_ZERO (&fdsr);
	FD_SET (0, &fdsr);
	struct timeval tv = { 0L, 0L };  // immediate
	int s = select (1, &fdsr, NULL, NULL, &tv);
	if (s < 0) return -1;
	if (FD_ISSET (0, &fdsr)) return 1;  // has char
	return 0;  // no char
	}


void catch_int ()
{
	extern int _flag_prompt;
	// FIXME: SIGINT should be caught in main module
	_flag_prompt = 1;
}


void char_raw ()
	{
	struct termios termios;
	fflush(stdout);
	tcgetattr(0, &termios);
	termios.c_iflag &= ~(ICRNL|IGNCR|INLCR);
	termios.c_lflag &= ~(ECHO|ECHOE|ECHONL|ICANON);
	termios.c_lflag |= ISIG;
	tcsetattr(0, TCSADRAIN, &termios);

	int nonblock = 1;
	ioctl(0, FIONBIO, &nonblock);
	}


void char_normal ()
	{
	int nonblock = 0;
	fflush(stdout);
	ioctl(0, FIONBIO, &nonblock);
	tcsetattr(0, TCSADRAIN, &def_termios);
	}


// Keep this signal catching until all the assert() are removed
// to avoid leaving the EMU86 terminal in raw mode after abort

// See GitHub issue #15

static void catch_abort(int sig)
	{
	char_term();
	exit(1);
	}


int char_init ()
	{
#ifdef __APPLE__
	static char buf[1];
	setvbuf(stdout, buf, _IOFBF, sizeof(buf));
#endif
	tcgetattr(0, &def_termios);

	// TODO: move to main()
	signal(SIGINT, catch_int);
	siginterrupt(SIGINT, 0);
	char_raw();

	signal(SIGABRT, catch_abort);
	return 0;
	}


void char_term ()
	{
	if (def_termios.c_oflag) char_normal();
	}
