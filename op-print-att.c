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

struct map { int addr; char *name; };
extern struct map map[];

// map near text address to symbol
// TODO: far_text_symbol and data_name
char *text_symbol(int addr)
{
	int i = 1;
	static char buf[32];

	while (addr >= map[i].addr)
	{
		if (map[i].addr == -1)
		{
			sprintf(buf, "%.4x", addr);	// fail, display hex address
			return buf;
		}
		i++;
	}
	if (addr - map[i-1].addr)
		sprintf(buf, "%s+%x", map[i-1].name, addr - map[i-1].addr);
	else sprintf(buf, "%s", map[i-1].name);
	return buf;
}


// Register class

static char * reg8_names  [] = { "%al", "%cl", "%dl", "%bl", "%ah", "%ch", "%dh", "%bh" };
static char * reg16_names [] = { "%ax", "%cx", "%dx", "%bx", "%sp", "%bp", "%si", "%di" };
static char * seg_names   [] = { "%es", "%cs", "%ss", "%ds" };

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

	if (flags & AF_DISP)
		{
		if (flags & (AF_BX|AF_BP|AF_SI|AF_DI))
			{
			print_rel (1, rel);
			}
		else
			{
			printf ("0x%hx", (word_t) rel);
			}
		}

	if (flags & (AF_BX|AF_BP|AF_SI|AF_DI))
		{
		putchar ('(');
		}

	if (flags & AF_BX)
		{
		print_string ("%bx");
		reg = 1;
		}

	if (flags & AF_BP)
		{
		print_string ("%bp");
		reg = 1;
		}

	if (flags & AF_SI)
		{
		if (reg) putchar (',');
		print_string ("%si");
		reg = 1;
		}

	if (flags & AF_DI)
		{
		if (reg) putchar (',');
		print_string ("%di");
		}

	if (flags & (AF_BX|AF_BP|AF_SI|AF_DI))
		{
		putchar (')');
		}
	}


// Variable class

static void print_var (op_var_t * var)
	{
	switch (var->type)
		{
		case VT_IMM:
			if (var->s)
				{
				printf ("$0x%.2x", var->val.s & 0xFFFF);
				break;
				}

			if (var->w)
				{
				printf ("$0x%.4x", var->val.w);
				break;
				}

			printf ("$0x%.2x", var->val.b);
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

				printf ("0x%.4x",var->seg);
				putchar (':');
				printf ("0x%.4x",var->val.w);
				break;
				}

			// Near address is relative

			printf ("%s", text_symbol((word_t) ((short) op_code_off + var->val.s)));
			break;

		}
	}


// Print a relative number with optional sign prefix

void print_rel (byte_t prefix, short rel)
	{
	if (rel < 0)
		{
		putchar ('-');
		rel = -rel;
		}

	printf ("0x%hx", (word_t) rel);
	}


// Operation classes

void print_op (op_desc_t * op_desc)
	{
	char * name = op_id_to_name (OP_ID);

	char name2 [OPNAME_MAX + 2];

	if (!name)
		{
		strcpy (name2, "???");
		}
	else
		{
		// Lower case operation name

		char * p = name;
		char * q = name2;
		while (*p)
			{
			if (*p >= 'A')
				*q++ = *p++ - 'A' + 'a';
			else
				*q++ = *p++;
			}

		// Operation name suffix to explicit operand size

		if (op_desc->var_wb == VP_BYTE)
			*q++ = 'b';

		if (op_desc->var_wb == VP_WORD)
			*q++ = 'w';

		*q = 0;
		}

	print_column (name2, OPNAME_MAX + 2);  // 1 space + optional suffix

	byte_t count = op_desc->var_count;

	if (count >= 3)
		{
		print_var (&(op_desc->var_from2));
		putchar (',');
		}

	if (count >= 2)
		{
		print_var (&(op_desc->var_from));
		putchar (',');
		}

	if (count >= 1)
		{
		print_var (&(op_desc->var_to));
		}
	}
