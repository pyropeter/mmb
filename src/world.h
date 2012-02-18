/**
 * @file
 */

#ifndef _MMB_WORLD_H
#define _MMB_WORLD_H

#include "defs.h"
#include "vector.h"
#include "chunk.h"

typedef struct World {
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
} World;

extern World *worldInit(Block *(*gen)(Vector3i));
extern Chunk *worldGetChunk(World *world, Vector3i pos);
extern void worldUpdateChunk(World *world, Chunk *chunk);
extern void worldAfterFrame(World *world);

#endif /* _MMB_WORLD_H */
