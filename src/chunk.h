//! @file

#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"
#include "vector.h"
#include "block.h"

typedef struct Chunk Chunk;
typedef struct ChunkGroup ChunkGroup;

typedef struct Bubble {
	int edge;
	int status;
	int cookie;

	List /*Bubble*/ *adjacent;

	Chunk *chunk;
	ChunkGroup *chunkGroup;
} Bubble;

typedef struct Chunk {
	Vector3i low, high;
	int status;
	int cookie;

	Bubble *bubble;

	List /*Chunk*/ *adjacent;

	Block **blocks;
} Chunk;

typedef struct ChunkGroup {
	Vector3i low;

	int status;

	List /*Chunk*/ *chunksXS, *chunksXG;
	List /*Chunk*/ *chunksYS, *chunksYG;
	List /*Chunk*/ *chunksZS, *chunksZG;
} ChunkGroup;

#endif /* _MMB_CHUNK_H */
