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

typedef char Block;

extern void *knalloc(size_t size);

typedef struct List {
	ssize_t elementSize;
	off_t nextFree;

	ssize_t memSize;
	void *mem;
} List;

#endif /* _MMB_DEFS_H */
