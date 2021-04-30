
#pragma once

#include "op-common.h"

// Generic interrupt controller

#define INT_LEVEL 0  // level triggered
#define INT_EDGE  1  // rising edge triggered

extern int _int_line_max;
extern int _int_prio_max;

extern int _int_mode [];  // level or edge
extern int _int_prio [];  // priority
extern int _int_vect [];  // vector (0..255 for IA16)
extern int _int_mask [];  // mask flag
extern int _int_req  [];  // requested flag
extern int _int_serv [];  // serviced flag

extern int _int_signal;    // from controller to processor

void int_line_set (int line, int stat);
int int_ack (byte_t * vect);
void int_end_line (int line);
void int_end_prio ();

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

// TODO: move to rom-xxx
void rom_init (void);
