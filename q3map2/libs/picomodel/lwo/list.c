/*
   ======================================================================
   list.c

   Generic linked list operations.

   Ernie Wright  17 Sep 00
   ====================================================================== */

#include "../picointernal.h"
#include "lwo2.h"


/*
   ======================================================================
   lwListFree()

   Free the items in a list.
   ====================================================================== */

void lwListFree( void *list, ListFreeFunc freefunc ){
	lwNode *node, *next;

	node = ( lwNode * ) list;
	while ( node ) {
		next = node->next;
		freefunc( node );
		node = next;
	}
}


/*
   ======================================================================
   lwListAdd()

   Append a node to a list.
   ====================================================================== */

void lwListAdd( void **list, void *node ){
	lwNode *head, *tail;

	head = *( ( lwNode ** ) list );
	if ( !head ) {
		*list = node;
		return;
	}
	while ( head ) {
		tail = head;
		head = head->next;
	}
	tail->next = ( lwNode * ) node;
	( ( lwNode * ) node )->prev = tail;
}


/*
   ======================================================================
   lwListInsert()

   Insert a node into a list in sorted order.
   ====================================================================== */

void lwListInsert( void **vlist, void *vitem, ListCompareFunc compare ){
	lwNode **list, *item, *node, *prev;

	if ( !*vlist ) {
		*vlist = vitem;
		return;
	}

	list = ( lwNode ** ) vlist;
	item = ( lwNode * ) vitem;
	node = *list;
	prev = NULL;

	while ( node ) {
		if ( 0 < compare( node, item ) ) {
			break;
		}
		prev = node;
		node = node->next;
	}

	if ( !prev ) {
		*list = item;
		node->prev = item;
		item->next = node;
	}
	else if ( !node ) {
		prev->next = item;
		item->prev = prev;
	}
	else {
		item->next = node;
		item->prev = prev;
		prev->next = item;
		node->prev = item;
	}
}
