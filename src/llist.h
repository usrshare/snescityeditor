#ifndef LLIST_H
#define LLIST_H
#include <stdint.h>

// this is a macro-based implementation of a queue based on a linked list.

#define LLIST_DECLARE(type) \
struct llist_ ## type { \
	uint8_t exists; \
	type value; \
	struct llist_ ## type * next; \
}; \

#define LLIST_NEW(type,name) \
struct llist_ ## type name = {.exists = 0, .next = 0};

#define LLIST_PUSH(type,name,newvalue) \
{ \
	if (!name.exists) { name.exists = 1, name.value = newvalue; } \
	else { \
		struct llist_ ## type *tail = &name; \
		while (tail->next != 0) tail = tail->next; \
		struct llist_ ## type *newitem = malloc(sizeof(struct llist_ ## type)); \
		if (newitem == 0) { printf("Unable to allocate memory for new queue item.\n"); exit(1); } \
		newitem->exists = 1; newitem->value = newvalue; newitem->next = 0; \
		tail->next = newitem; \
	} \
}

#define LLIST_EXISTS(name) (name.exists > 0)

#define LLIST_POP(type,name,dest) \
{ \
	if (name.next == 0) { \
		name.exists = 0; dest = name.value; \
	} else { \
		name.exists = name.next->exists; dest = name.value; \
		struct llist_ ## type *temp = name.next; \
		name.value = temp->value; \
		name.next = temp->next; free(temp); \
	} \
}

#endif
