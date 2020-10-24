
#pragma once

#include "op-common.h"

// Generic interrupt controller

extern int _int_line_max;
extern int _int_prio_max;

// TODO: remove useless line array

extern int _int_line [];
extern int _int_prio [];
extern int _int_vect [];
extern int _int_mask [];
extern int _int_req  [];
extern int _int_serv [];

extern int _int_req_flag;

void int_line_set (int line, int stat);
int int_ack (byte_t * vect);
void int_end (int line);

// Breakpoint interrupt

extern byte_t _break_int_flag;

int int_03h (void);

// Emulated interrupt handlers

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

