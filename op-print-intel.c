// LIB86 - 80x86 library
// Operation classes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "op-id.h"
#include "op-id-name.h"
#include "op-common.h"
#include "op-class.h"


// Opcode helpers

extern word_t op_code_off;

char op_code_str [3 * OPCODE_MAX + 2];
byte_t op_code_pos;


// Register class

static char * reg8_names  [] = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" };
static char * reg16_names [] = { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI" };
static char * seg_names   [] = { "ES", "CS", "SS", "DS" };

static void print_reg (byte_t type, byte_t num)
	{
	switch (type)
		{
		case RT_REG8:
			print_string (reg8_names [num]);
			break;

		case RT_REG16:
			print_string (reg16_names [num]);
			break;

		case RT_SEG:
			print_string (seg_names [num]);
			break;
		}
	}


static void print_mem (byte_t flags, short rel)
	{
	byte_t reg;

	reg = 0;
	putchar ('[');

	if (flags & AF_BX)
		{
		print_string ("BX");
		reg = 1;
		}

	if (flags & AF_BP)
		{
		print_string ("BP");
		reg = 1;
		}

	if (flags & AF_SI)
		{
		if (reg) putchar ('+');
		print_string ("SI");
		reg = 1;
		}

	if (flags & AF_DI)
		{
		if (reg) putchar ('+');
		print_string ("DI");
		reg = 1;
		}

	if (flags & AF_DISP)
		{
		if (reg)
			{
			print_rel (1, rel);  // with prefix
			}
		else
			{
			printf ("%hXh", (word_t) rel);
			}
		}

	putchar (']');
	}


// Variable class

static void print_var (op_var_t * var)
	{
	switch (var->type)
		{
		case VT_IMM:
			if (var->s)
				{
				print_rel (0, var->val.s);
				break;
				}

			if (var->w)
				{
				printf ("%.4Xh", var->val.w);
				break;
				}

			printf ("%.2Xh", var->val.b);
			break;

		case VT_REG:
			print_reg (var->w ? RT_REG16 : RT_REG8, var->val.r);
			break;

		case VT_SEG:
			print_reg (RT_SEG, var->val.r);
			break;

		case VT_MEM:
			print_mem (var->flags, var->val.s);
			break;

		case VT_LOC:
			if (var->far)
				{
				// Far address is absolute

				printf ("%.4Xh",var->seg);
				putchar (':');
				printf ("%.4Xh",var->val.w);
				break;
				}

			// Near address is relative

			printf ("%.4Xh", (word_t) ((short) op_code_off + var->val.s));
			break;

		}
	}


// Print a relative number with optional sign prefix

void print_rel (byte_t prefix, short rel)
	{
	if (rel >= 0)
		{
		if (prefix) putchar ('+');
		}
	else
		{
		putchar ('-');
		rel = -rel;
		}

	printf ("%hXh", (word_t) rel);
	}


// Operation classes

void print_op (op_desc_t * op_desc)
	{
	char *name = op_id_to_name (OP_ID);
	if (!name) name = "???";
	print_column (name, OPNAME_MAX + 2);

	byte_t count = op_desc->var_count;

	if (!count && op_desc->var_wb)
		{
		printf ((op_desc->var_wb == VP_WORD) ? "WORD": "BYTE");
		}

	if (count == 1)
		{
		if (op_desc->var_wb) printf ((op_desc->var_wb == VP_WORD) ? "WORD ": "BYTE ");
		print_var (&(op_desc->var_to));
		}

	if (count == 2)
		{
		print_var (&(op_desc->var_to));
		putchar (',');
		if (op_desc->var_wb) printf ((op_desc->var_wb == VP_WORD) ? "WORD ": "BYTE ");
		print_var (&(op_desc->var_from));
		}
	}
