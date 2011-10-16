#include <stdlib.h>
#include <stdio.h>

#include "defs.h"

void *knalloc(size_t size) {
	/* The german word "Knall" means bang/explosion.
	 * This function's name is thus to be considered a pun. */
	void *mem = malloc(size);
	if (mem == NULL) {
		fprintf(stderr, "malloc() failed. out of memory?\n");
		exit(EXIT_FAILURE);
	}
	return mem;
}

List *listNew(ssize_t elementSize, ssize_t initialCount) {
	List *list = knalloc(sizeof(List));
	list->nextFree = 0;

	list->elementSize = elementSize;

	list->memSize = elementSize * initialCount;
	list->mem = knalloc(list->memSize);

	return list;
}
