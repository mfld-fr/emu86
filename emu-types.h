#pragma once

// Basic IA16 types

typedef unsigned char  byte_t;
typedef unsigned short word_t;
typedef unsigned long  addr_t;
typedef unsigned long  dword_t;

// Container helper

#define structof (type, member, pointer) ( \
	(type *) ((char *) pointer - offsetof (type, member)))
