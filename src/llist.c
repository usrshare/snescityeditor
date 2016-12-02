#include <stdio.h>
#include <stdlib.h>
#include "llist.h"
	
LLIST_DECLARE(long);

void main() {

	LLIST_NEW(long, test);
	LLIST_PUSH(long, test, 123);
	LLIST_PUSH(long, test, 456);
	LLIST_PUSH(long, test, 789);
	LLIST_PUSH(long, test, 1337);

	long a = 0;

	while (LLIST_EXISTS(test)) {
	a = LLIST_POP(long, test);
	printf("%d\n",a); }

	return;
}
