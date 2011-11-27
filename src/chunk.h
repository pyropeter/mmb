#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"

typedef struct ChunkGroup {
	Point low;
	List *chunksXS, *chunksXG;
	List *chunksYS, *chunksYG;
	List *chunksZS, *chunksZG;
} ChunkGroup;

typedef struct Chunk {
	Point low, high;
	int status;
	int cookie;

	List *adjacent;

	Block **blocks;
} Chunk;

typedef struct AnnotatedBlock {
	Block *block;
	int lowx, lowy, highx, highy;
	int low2x, low2y, high2x, high2y;
	Chunk *chunk;
} AnnotatedBlock;

typedef struct Metachunk {
	Block *(*generator)(Point);

	int cookie;
	List *chunks;
	
	Point3i groupSize;
	List *chunkGroups;

	Chunk *lastChunk;
	Point lastPos;
} Metachunk;

extern Metachunk *chunkInit(Block *(*gen)(Point), Point pos);
extern Chunk *chunkGet(Metachunk *world, Point pos);
extern void chunkUpdate(Metachunk *world, Chunk *chunk);

#endif /* _MMB_CHUNK_H */
