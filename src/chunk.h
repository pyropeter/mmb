/**
 * @file
 */

#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"

typedef struct Chunk {
	Vector3i low, high;
	int status;
	int cookie;

	List *adjacent;

	Block **blocks;
} Chunk;

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
