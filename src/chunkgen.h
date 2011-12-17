/**
 * @file
 */

#ifndef _MMB_CHUNKGEN_H
#define _MMB_CHUNKGEN_H

#include "defs.h"
#include "chunk.h"

typedef struct ChunkGroup {
	Vector3i low;
	List *chunksXS, *chunksXG;
	List *chunksYS, *chunksYG;
	List *chunksZS, *chunksZG;
} ChunkGroup;

typedef struct AnnotatedBlock {
	Block *block;
	int lowx, lowy, highx, highy;
	int low2x, low2y, high2x, high2y;
	Chunk *chunk;
} AnnotatedBlock;

extern void chunkCreateBatch(Metachunk *world, Vector3i low);

#endif /* _MMB_CHUNKGEN_H */
