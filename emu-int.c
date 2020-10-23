
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-int.h"


// Dummy INT3 as we already run under emulator

byte_t _break_int_flag = 0;

int int_03h ()
	{
	//puts ("warning: INT3");
	_break_int_flag = 1;
	return 0;
	}


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
