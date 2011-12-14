/**
 * @file
 */

#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"

typedef struct ChunkGroup {
	Vector3i low;
	List *chunksXS, *chunksXG;
	List *chunksYS, *chunksYG;
	List *chunksZS, *chunksZG;
} ChunkGroup;

typedef struct Chunk {
	Vector3i low, high;
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
	Block *(*generator)(Vector3i);

	int cookie;
	List *chunks;
	
	Vector3i groupSize;
	List *chunkGroups;

	Chunk *lastChunk;
	Vector3i lastPos;
	
	// chunkUpdate
	Chunk *chunkToUpdate;
} Metachunk;

extern Metachunk *chunkInit(Block *(*gen)(Vector3i));
extern Chunk *chunkGet(Metachunk *world, Vector3i pos);
extern void chunkUpdate(Metachunk *world, Chunk *chunk);
extern void chunkAfterFrame(Metachunk *world);

#endif /* _MMB_CHUNK_H */
