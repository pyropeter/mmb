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

List *listNew() {
	List *list = knalloc(sizeof(List));

	ssize_t size = 8;
	list->mem = knalloc(size * sizeof(void*));
	list->nextFree = list->mem;
	list->end = list->mem + size;

	return list;
}

void listInsert(List *list, void *ptr) {
	if (list->end <= list->nextFree + sizeof(void*)) {
		off_t freeOffset = list->nextFree - list->mem;
		ssize_t size = (list->end - list->mem) * 2;
		printf("listInsert(): new size is %llu\n",
				(long long unsigned int)size);

		list->mem = realloc(list->mem, size * sizeof(void*));
		list->end = list->mem + size;
		list->nextFree = list->mem + freeOffset;

		if (list->mem == NULL) {
			fprintf(stderr, "realloc() failed. out of memory?\n");
			exit(EXIT_FAILURE);
		}
	}

	*(list->nextFree) = ptr;
	list->nextFree++;
}






