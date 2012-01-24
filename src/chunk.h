/**
 * @file
 */

#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"
#include "vector.h"

typedef struct Chunk {
	Vector3i low, high;
	int status;
	int cookie;

	List /*Chunk*/ *adjacent;

	Block **blocks;
} Chunk;

typedef struct ChunkGroup {
	Vector3i low;
	List /*Chunk*/ *chunksXS, *chunksXG;
	List /*Chunk*/ *chunksYS, *chunksYG;
	List /*Chunk*/ *chunksZS, *chunksZG;
} ChunkGroup;

typedef struct AnnotatedBlock {
	Block *block;
	Vector2i low, high;
	Vector2i low2, high2;
	Chunk *chunk;
} AnnotatedBlock;

typedef struct Metachunk {
	Block *(*generator)(Vector3i);

	int cookie;
	List /*Chunk*/ *chunks;
	
	Vector3i groupSize;
	List /*ChunkGroup*/ *chunkGroups;

	Chunk *lastChunk;
	Vector3i lastPos;
	
	// chunkUpdate
	int maxChunksToUpdate;
	List /*Chunk*/ *chunksToUpdate;
	
	// chunkgen
	AnnotatedBlock *annotatedBlocks;
} Metachunk;

extern Metachunk *chunkInit(Block *(*gen)(Vector3i));
extern Chunk *chunkGet(Metachunk *world, Vector3i pos);
extern void chunkUpdate(Metachunk *world, Chunk *chunk);
extern void chunkAfterFrame(Metachunk *world);

#endif /* _MMB_CHUNK_H */
