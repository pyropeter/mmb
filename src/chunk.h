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

#endif /* _MMB_CHUNK_H */
