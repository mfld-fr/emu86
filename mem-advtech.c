
#include "emu-mem-io.h"

// Memory access

void mem_write_byte (addr_t a, byte_t b, byte_t init)
	{
	// No memory mapped device

	mem_write_byte_0 (a, b, init);
	}

void mem_write_word (addr_t a, word_t w, byte_t init)
	{
	// No memory mapped device

	mem_write_word_0 (a, w, init);
	}
