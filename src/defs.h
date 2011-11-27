#ifndef _MMB_DEFS_H
#define _MMB_DEFS_H

#include <sys/types.h>

#define MAXCOMP 4294967295L
#define HALFCOMP 2147483647L
typedef u_int32_t Comp;

typedef struct Point {
	Comp x, y, z;
} Point;
#define POINTCMP(a,o,b) (a.x o b.x && a.y o b.y && a.z o b.z)
#define POINTOP(a,o,b) ((Point){a.x o b.x, a.y o b.y, a.z o b.z})
void pointPrint(Point point, char* tail);

typedef struct Point3i {
	int x, y, z;
} Point3i;

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

extern long startTimer();
extern long stopTimer(long start);
extern void explode();

#endif /* _MMB_DEFS_H */
