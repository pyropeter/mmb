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

List *listNew(ssize_t elementSize) {
	List *list = knalloc(sizeof(List));
	list->nextFree = 0;

	list->elementSize = elementSize;

	list->memSize = 64;
	list->mem = knalloc(list->memSize);

	return list;
}

void listInsert(List *list, void *element) {
	if (list->memSize < list->nextFree + list->elementSize) {
		list->memSize *= 2;
		printf("listInsert(): new memSize is %i\n", list->memSize);
		list->mem = realloc(list->mem, list->memSize);

		if (list->mem == NULL) {
			fprintf(stderr, "realloc() failed. out of memory?\n");
			exit(EXIT_FAILURE);
		}
	}

	memcpy(list->nextFree, element, list->elementSize);
}






