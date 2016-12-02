#include <stdio.h>
#include <stdlib.h>
#include "llist.h"

void main() {

	LLIST_DECLARE(long)
	LLIST_NEW(long, test)
	LLIST_PUSH(long, test, 123);
	LLIST_PUSH(long, test, 456);
	LLIST_PUSH(long, test, 789);
	LLIST_PUSH(long, test, 1337);

	long a = 0;

	while (LLIST_EXISTS(test)) {
	LLIST_POP(long, test, a);
	printf("%d\n",a); }

	return 0;
}
