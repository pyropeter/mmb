/**
 * @file
 */

#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "vector.h"
#include "chunk.h"
#include "chunkgen.h"

/**
 * Creates and initializes a metachunk
 * 
 * @param gen		A block generator function
 */
Metachunk *chunkInit(Block *(*gen)(Vector3i)) {
	Metachunk *world = knalloc(sizeof(Metachunk));
	world->cookie = 0;
	world->chunks = listNew(sizeof(Chunk*));

	world->generator = gen;

	world->lastChunk = NULL;

	world->chunkToUpdate = NULL;

	// chunkgen
	world->chunkGroups = listNew(sizeof(ChunkGroup*));
	world->groupSize = (Vector3i){8, 8, 8};
	chunkgenInit(world);

	return world;
}

Chunk *searchAllChunks(Metachunk *world, Vector3i pos) {
	Chunk **chunk;
	LISTITER(world->chunks, chunk, Chunk**) {
		if (VEC3CMP((*chunk)->low, <=, pos) &&
				VEC3CMP((*chunk)->high, >=, pos)) {
			world->lastPos = pos;
			world->lastChunk = *chunk;
			return world->lastChunk;
		}
	}
	return NULL;
}

/**
 * Returns the chunk at position pos
 * 
 * If no chunk at that position exists, it is generated and returned.
 */
Chunk *chunkGet(Metachunk *world, Vector3i pos) {
	if (world->lastChunk) {
		// check if position has changed at all
		if (VEC3CMP(world->lastPos, ==, pos))
			return world->lastChunk;

		// check if point is still in last chunk
		if (VEC3CMP(world->lastChunk->low, <=, pos)
		 && VEC3CMP(world->lastChunk->high, >=, pos)) {
			world->lastPos = pos;
			return world->lastChunk;
		}

		// check chunks adjacent to the chunks adjacent to the
		// last chunk
		Chunk **middle, **adjacent;
		LISTITER(world->lastChunk->adjacent, middle, Chunk**) {
			LISTITER((*middle)->adjacent, adjacent, Chunk**) {
				if (VEC3CMP((*adjacent)->low, <=, pos)
				 && VEC3CMP((*adjacent)->high, >=, pos)) {
					world->lastChunk = *adjacent;
					world->lastPos = pos;
					return *adjacent;
				}
			}
		}

		// last resort
		Chunk *res = searchAllChunks(world, pos);
		if (res)
			return res;
	}

	// create chunks at the requested position
	Vector3i batchPos;
	batchPos.x = divRoundDown(pos.x, world->groupSize.x);
	batchPos.y = divRoundDown(pos.y, world->groupSize.y);
	batchPos.z = divRoundDown(pos.z, world->groupSize.z);
	batchPos = VEC3IOP(batchPos, *, world->groupSize);
	chunkgenCreate(world, batchPos);

	return searchAllChunks(world, pos);
}

/**
 * Marks a chunk for updating by chunkAfterFrame()
 * 
 * The real updating is done by chunkAfterFrame(). Only the first chunk with
 * missing adjacent chunks is marked. This ensures that only one ChunkGroup
 * is generated every frame (to limit the lag).
 */
void chunkUpdate(Metachunk *world, Chunk *chunk) {
	if (world->chunkToUpdate != NULL || chunk->status == 0)
		return;

	world->chunkToUpdate = chunk;
}

/**
 * Updates the chunk marked by chunkUpdate()
 * 
 * This generates one ChunkGroup adjacent to the marked chunk.
 */
void chunkAfterFrame(Metachunk *world) {
	if (world->chunkToUpdate == NULL)
		return;

#ifdef MMB_DEBUG_CHUNK
	printf("Updating chunk %p, status %i\n",
			world->chunkToUpdate, world->chunkToUpdate->status);
#endif

	Vector3i pos;
	pos.x = divRoundDown(world->chunkToUpdate->low.x, world->groupSize.x);
	pos.y = divRoundDown(world->chunkToUpdate->low.y, world->groupSize.y);
	pos.z = divRoundDown(world->chunkToUpdate->low.z, world->groupSize.z);

	if      (world->chunkToUpdate->status & DIR_XG)
		pos.x++;
	else if (world->chunkToUpdate->status & DIR_XS)
		pos.x--;
	else if (world->chunkToUpdate->status & DIR_YG)
		pos.y++;
	else if (world->chunkToUpdate->status & DIR_YS)
		pos.y--;
	else if (world->chunkToUpdate->status & DIR_ZG)
		pos.z++;
	else if (world->chunkToUpdate->status & DIR_ZS)
		pos.z--;
	else
		exit(EXIT_FAILURE);

	pos = VEC3IOP(pos, *, world->groupSize);
	chunkgenCreate(world, pos);

	world->chunkToUpdate = NULL;
}
