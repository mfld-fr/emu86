#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/select.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "emu-serial.h"

static struct termios def_termios;

void serial_send (byte_t c)
	{
	//if (c == '\n') serial_send ('\r');	// may be needed for Linux terminal
	int n = write (1, &c, 1);
	if (n != 1)
		{
		perror ("warning: cannot write to console:");  // TODO: propagate error
		}
	}


byte_t serial_recv ()
	{
	byte_t c = 0xFF;

	int n = read (0, &c, 1);
	if (n == 0) return 0;
	if (n != 1)
		{
		perror ("warning: cannot read from console:");  // TODO: propagate error
		}
	if (c == 0x7f) c = '\b';	// convert DEL to BS

	return c;
	}


byte_t serial_poll ()
	{
	fd_set fdsr;
	FD_ZERO (&fdsr);
	FD_SET (0, &fdsr);
	struct timeval tv = { 0L, 0L };  // immediate
	int s = select (1, &fdsr, NULL, NULL, &tv);
	assert (s >= 0);
	if (FD_ISSET (0, &fdsr)) return 1;
	return 0;
	}


void serial_catch ()
{
	extern int flag_prompt;

	flag_prompt = 1;
}

void serial_raw ()
	{
	struct termios termios;
	tcgetattr(0, &termios);
	termios.c_iflag &= ~(ICRNL|IGNCR|INLCR);
	//termios.c_oflag &= ~(OPOST);
	termios.c_lflag &= ~(ECHO|ECHOE|ECHONL|ICANON);
	tcsetattr(0, TCSANOW, &termios);

	int nonblock = 1;
	ioctl(0, FIONBIO, &nonblock);
	}

void serial_normal ()
	{
	int nonblock = 0;
	ioctl(0, FIONBIO, &nonblock);
	tcsetattr(0, TCSANOW, &def_termios);
	}

static void catch_abort(int sig)
	{
	exit(1);
	}

void serial_init ()
	{
	tcgetattr(0, &def_termios);
	signal(SIGINT, serial_catch);
	serial_raw();
	signal(SIGABRT, catch_abort);
	atexit(serial_term);
	}


void serial_term ()
	{
	if (def_termios.c_oflag) serial_normal();
	}
