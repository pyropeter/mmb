//! @file

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "defs.h"

static struct timespec tp;

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

void listFree(List *list) {
	free(list->mem);
	free(list);
}

/**
 * Returns the number of elements in the List list
 */
int listLen(List *list) {
	return list->nextFree - list->mem;
}

/**
 * Removes all elements from the List, but keeps the memory
 * 
 * Use this if you are going to refill the list with a similar amount of items.
 */
void listEmpty(List *list) {
	list->nextFree = list->mem;
}

void listInsert(List *list, void *ptr) {
	if (list->end <= list->nextFree + 2) {
		off_t freeOffset = list->nextFree - list->mem;
		ssize_t size = (list->end - list->mem) * 2;
		//printf("listInsert(%p): new size is %llu\n", list,
		//		(long long unsigned int)size);

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

void **listSearch(List *list, void *ptr) {
	// place sentinel
	*(list->nextFree) = ptr;

	void **pos = list->mem;
	for (; ; pos++) {
		if (*pos == ptr) {
			if (pos != list->nextFree)
				return pos;
			else
				return NULL;
		}
	}
	return NULL;
}

void listReplace(List *list, void *old, void *new) {
	void **ptr = listSearch(list, old);
	assert(ptr != NULL);
	*ptr = new;
}

void listRemove(List *list, void *element) {
	void **ptr = listSearch(list, element);
	assert(ptr != NULL);

	// overwrite the to-be-removed value with the last entry of the list
	list->nextFree--;
	*ptr = *(list->nextFree);
}

/**
 * Returns a value that can be passed to stopTimer()
 */
long startTimer() {
	clock_gettime(CLOCK_REALTIME, &tp);
	return (tp.tv_sec % 1000) * 1000000 + tp.tv_nsec / 1000;
}

/**
 * Returns the time since the call to startTimer() in microseconds
 * 
 * This can't be used to measure durations longer than 1000 seconds
 */
long stopTimer(long start) {
	return (startTimer() + 1000000000 - start) % 1000000000;
}

/**
 * Divides numerator by denominator and rounds down
 * 
 * (The devision literal in C always rounds towards zero.)
 * This function only accepts positive denominators (This may change in the
 * future if anyone actually needs to divide by a negative number)
 */
int divRoundDown(int numerator, int denominator) {
	if (numerator < 0)
		return (numerator - denominator + 1) / denominator;
	else
		return numerator / denominator;
}

/**
 * Divides numerator by denominator and returns a positive remainder
 * 
 * (The modulus literal in C always rounds towards zero.)
 */
int modPositive(int numerator, int denominator) {
	int remainder = numerator % denominator;
	if (remainder < 0)
		return remainder + denominator;
	else
		return remainder;
}
