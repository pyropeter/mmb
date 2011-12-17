#ifndef _MMB_DEFS_H
#define _MMB_DEFS_H

#include <sys/types.h>

typedef char Block;

extern void *knalloc(size_t size);

typedef struct List {
	void **mem;
	void **end;
	void **nextFree;
} List;

extern List *listNew();
extern void listFree(List *list);
extern void listInsert(List *list, void *element);
extern void **listSearch(List *list, void *element);
#define LISTITER(list, var, type) \
	for (var = (type)(list)->mem; (void**)var != (list)->nextFree; var++)

#define DIR_XG  1
#define DIR_XS  2
#define DIR_YG  4
#define DIR_YS  8
#define DIR_ZG 16
#define DIR_ZS 32
#define DIR_OPPOSITE(dir) ((dir & 21) ? dir << 1 : dir >> 1)

extern long startTimer();
extern long stopTimer(long start);

extern int divRoundDown(int numerator, int denominator);
extern int modPositive(int numerator, int denominator);

#endif /* _MMB_DEFS_H */
