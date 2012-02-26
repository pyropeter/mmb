/**
 * @file
 */

#ifndef _MMB_CHUNK_H
#define _MMB_CHUNK_H

#include "defs.h"
#include "vector.h"
#include "block.h"

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

#endif /* _MMB_CHUNK_H */
