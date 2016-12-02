#ifndef LLIST_H
#define LLIST_H
#include <stdint.h>

// this is a macro-based implementation of a queue based on a linked list.

// use this define before using the LLIST macros.
#define LLIST_DECLARE(type) \
struct llist_ ## type { \
	uint8_t exists; \
	type value; \
	struct llist_ ## type * next; \
}; \
int llist_ ## type ## _push (struct llist_ ## type *list, type value) { \
	if (!list->exists) { list->exists = 1, list->value = value; return 0; } \
	else { \
		struct llist_ ## type *tail = list; \
		while (tail->next != 0) tail = tail->next; \
		struct llist_ ## type *newitem = malloc(sizeof(struct llist_ ## type)); \
		if (newitem == 0) { printf("Unable to allocate memory for new queue item.\n"); exit(1); } \
		newitem->exists = 1; newitem->value = value; newitem->next = 0; \
		tail->next = newitem; \
		return 0; \
	} \
}; \
type llist_ ## type ## _pop ( struct llist_ ## type *list ) { \
	if (list->next == 0) { \
		list->exists = 0; return list->value; \
	} else { \
		list->exists = list->next->exists; type dest = list->value; \
		struct llist_ ## type *temp = list->next; \
		list->value = temp->value; \
		list->next = temp->next; free(temp); \
		return dest; \
	} \
}

// this is the preferred way of creating a new empty LLIST.
#define LLIST_NEW(type,name) \
struct llist_ ## type name = {.exists = 0, .next = 0}

// this macro calls a function that pushes a new value at the end of an LLIST.
#define LLIST_PUSH(type,name,newvalue) llist_ ## type ## _push (&name, newvalue)

// this macro checks if there are still elements in the LLIST left.
#define LLIST_EXISTS(name) (name.exists > 0)

// this macro calls a function that removes the first element of the LLIST and returns it.
#define LLIST_POP(type,name) llist_ ## type ## _pop (&name)

// to destroy an LLIST, simply LLIST_POP all the elements until LLIST_EXISTS returns false.

#endif
