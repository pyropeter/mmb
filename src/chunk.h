#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"

typedef struct Chunk {
	Comp lx, ly, lz;
	Comp hx, hy, hz;
	char type;

	int neighborCount;
	struct Chunk **neighbors;

	Block **blocks;
} Chunk;

extern Chunk *chunkGet(Comp x, Comp y, Comp z);
extern void chunkUpdate(Chunk *chunk);

#endif /* _MMB_CHUNK_H */
