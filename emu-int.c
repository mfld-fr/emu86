//------------------------------------------------------------------------------
// EMU86 - Generic interrupt
//------------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-int.h"

//------------------------------------------------------------------------------
// Interrupt controller
//------------------------------------------------------------------------------

int _int_signal = 0;

// Interrupt controller procedure

static void int_proc ()
	{
	int sig = 0;

	for (int line = 0; line < _int_line_max; line++)
		{
		// Ignore requested if already serviced

		if (_int_req [line] && !_int_serv [line])
			{
			// At least one to signal

			sig = 1;
			break;
			}
		}

	_int_signal = sig;
	}

// Set interrupt line

void int_line_set (int line, int stat)
	{
	if (stat)
		{
		// Ignore the signal if masked

		if (!_int_mask [line]) _int_req [line] = 1;
		}
	else
		{
		// Cancel request in level mode

		if (_int_mode [line] == INT_LEVEL) _int_req [line] = 0;
		}

	int_proc ();
	}

// Interrupt acknowledge
// Provide vector to processor
// then set interrupt as serviced

int int_ack (byte_t * vect)
	{
	int err;

	while (1) {
		// Get requested of top priority

		int req = -1;
		int prio_min = _int_prio_max;

		for (int line = 0; line < _int_line_max; line++) {
			int prio = _int_prio [line];
			if (_int_req [line] && prio < prio_min) {
				req = line;
				prio_min = prio;
				}
			}

		// TODO: manage spurious request
		if (req < 0) assert (0);

		if (_int_mode [req] == INT_EDGE) _int_req [req] = 0;
		*vect = _int_vect [req];
		_int_serv [req] = 1;

		int_proc ();
		err = 0;
		break;
		}

	return err;
	}

// End of interrupt service
// Explicit mode

void int_end (int line)
	{
	// TODO: manage spurious end
	assert (_int_serv [line] == 1);

	_int_serv [line] = 0;

	int_proc ();
	}

//------------------------------------------------------------------------------

// Dummy INT3 as we already run under emulator

byte_t _break_int_flag = 0;

int int_03h (void)
	{
	//puts ("warning: INT3");
	_break_int_flag = 1;
	return 0;
	}

//------------------------------------------------------------------------------

// Search for emulated interrupt handler

int int_hand (byte_t i)
	{
	int err;

	int_num_hand_t * desc = _int_tab;

	while (1)
		{
		byte_t num = desc->num;
		int_hand_t hand = desc->hand;

		if (!num || !hand)
			{
			err = 1;  // no emulated handler
			break;
			}

		if (num == i)
			{
			err = (*hand) ();
			break;
			}

		desc++;
		}

	return err;
	}

//------------------------------------------------------------------------------
