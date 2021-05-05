// Double-linked list

#pragma once

struct _list_s
	{
	struct _list_s * prev;
	struct _list_s * next;
	};

typedef struct _list_s list_s;

void list_init (list_s * root);

void list_insert_before (list_s * next, list_s * node);
void list_insert_after  (list_s * prev, list_s * node);

void list_remove (list_s * node);
