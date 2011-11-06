#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"

typedef struct Chunk {
	Point low, high;
	int status;
	int lastRender;

	int adjacentCount;
	struct Chunk **adjacent;

	Block **blocks;
} Chunk;

typedef struct Metachunk {
	Block *(*generator)(Point);

	List *chunks;

	Chunk *lastChunk;
	Point lastPos;
} Metachunk;

extern Metachunk *chunkInit(Block *(*gen)(Point), Point pos);
extern Chunk *chunkGet(Metachunk *world, Point pos);
extern void chunkUpdate(Metachunk *world, Chunk *chunk);

#endif /* _MMB_CHUNK_H */
