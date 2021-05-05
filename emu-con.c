//------------------------------------------------------------------------------
// EMU86 - Generic console
//------------------------------------------------------------------------------

#include "emu-con.h"

#include <stddef.h>
#include <stdlib.h>


// Keyboard queue
// Key enqueued by con_proc() through con_put_key()
// Key dequeued by con_get_key() & con_poll()

typedef struct _key_s
	{
	list_s node;
	word_t k;
	} key_s;

static list_s key_queue;


void con_put_key (word_t k)
	{
	key_s * key = malloc (sizeof (key_s));
	if (key) {
		list_insert_after (&key_queue, &key->node);
		key->k = k;
		}
	}


int con_get_key (word_t * pk)
	{
	list_s * next = key_queue.next;

	// WARNING : non-blocking call with SDL console
	if (next == &key_queue) return 1;  // no key

	key_s * key = structof (key_s, node, next);
	*pk = key->k;

	list_remove (next);
	free (next);

	return 0;  // got key
	}


int con_poll_key (void)
	{
	list_s * next = key_queue.next;
	if (next != &key_queue) return 1;  // has key
	return 0;  // no key
	}


void con_init_key ()
	{
	list_init (&key_queue);
	}


//------------------------------------------------------------------------------
