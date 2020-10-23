
#pragma once

// INT breakpoint

extern byte_t _break_int_flag;

int int_03h (void);

// Interrupt handle table

typedef int (* int_hand_t) ();

struct int_num_hand_s
	{
	byte_t num;
	int_hand_t hand;
	};

typedef struct int_num_hand_s int_num_hand_t;

extern int_num_hand_t _int_tab [];

int int_hand (byte_t i);

void int_init (void);
