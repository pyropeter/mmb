#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"

struct {
	Comp lx, ly, lz;
	Comp hx, hy, hz;
	char type;
} _Chunk;

typedef struct _Chunk Chunk;

#endif /* _MMB_CHUNK_H */
