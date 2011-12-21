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

	world->maxChunksToUpdate = 50;
	world->chunksToUpdate = listNew();

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
 */
void chunkUpdate(Metachunk *world, Chunk *chunk) {
	if (chunk->status == 0)
		return;

	listInsert(world->chunksToUpdate, chunk);
}

/**
 * Updates the Chunks marked by chunkUpdate()
 * 
 * This generates one ChunkGroup adjacent to each marked Chunk.
 * At most world->maxChunksToUpdate Chunks are updated (to limit the lag).
 */
void chunkAfterFrame(Metachunk *world) {
	Chunk** chunk;
	Vector3i pos;
	int count = world->maxChunksToUpdate;

	LISTITER(world->chunksToUpdate, chunk, Chunk**) {
#ifdef MMB_DEBUG_CHUNK
		printf("Updating chunk %p, status %i\n",
				*chunk, (*chunk)->status);
#endif

		pos.x = divRoundDown((*chunk)->low.x, world->groupSize.x);
		pos.y = divRoundDown((*chunk)->low.y, world->groupSize.y);
		pos.z = divRoundDown((*chunk)->low.z, world->groupSize.z);

		if      ((*chunk)->status & DIR_XG)
			pos.x++;
		else if ((*chunk)->status & DIR_XS)
			pos.x--;
		else if ((*chunk)->status & DIR_YG)
			pos.y++;
		else if ((*chunk)->status & DIR_YS)
			pos.y--;
		else if ((*chunk)->status & DIR_ZG)
			pos.z++;
		else if ((*chunk)->status & DIR_ZS)
			pos.z--;
		else
			continue;

		pos = VEC3IOP(pos, *, world->groupSize);
		chunkgenCreate(world, pos);

		if (--count == 0)
			break;
	}

	listEmpty(world->chunksToUpdate);
}

/**
 * Returns the point of intersection between a ray and a Chunk.
 * 
 * @param pos	Where the ray begins (relative to chunk->low)
 * @param dir	Direction of the ray
 */
Vector3i chunkTraceRay(Chunk *chunk, Vector3f *pos, Vector3f dir)
{
	float factor;
	Vector3f size, distLow, distHigh, result;

	size = VEC3FOP(chunk->high, -, chunk->low);
	size.x++; size.y++; size.z++;
	distLow = *pos; // low -> pos
	distHigh = VEC3FOP(distLow, -, size); // high -> pos
	/*VECFPRINT(size, " - ");
	VECFPRINT(distLow, " - ");
	VECFPRINT(distHigh, "\n");*/

	if (dir.x <= 0) {
		factor = distLow.x / dir.x;
		result.y = dir.y * factor;
		if (distLow.y >= result.y && result.y >= distHigh.y) {
			result.z = dir.z * factor;
			if (distLow.z >= result.z && result.z >= distHigh.z) {
				pos->x = 0;
				pos->y -= result.y;
				pos->z -= result.z;
				//return DIR_XS;
				return (Vector3i){
					chunk->low.x + 0,
					chunk->low.y + floor(pos->y),
					chunk->low.z + floor(pos->z)};
			}
		}
	}
	if (dir.x >= 0) {
		factor = distHigh.x / dir.x;
		result.y = dir.y * factor;
		if (distLow.y >= result.y && result.y >= distHigh.y) {
			result.z = dir.z * factor;
			if (distLow.z >= result.z && result.z >= distHigh.z) {
				pos->x = size.x;
				pos->y -= result.y;
				pos->z -= result.z;
				//return DIR_XG;
				return (Vector3i){
					chunk->low.x + size.x,
					chunk->low.y + floor(pos->y),
					chunk->low.z + floor(pos->z)};
			}
		}
	}

	if (dir.y <= 0) {
		factor = distLow.y / dir.y;
		result.x = dir.x * factor;
		if (distLow.x >= result.x && result.x >= distHigh.x) {
			result.z = dir.z * factor;
			if (distLow.z >= result.z && result.z >= distHigh.z) {
				pos->y = 0;
				pos->x -= result.x;
				pos->z -= result.z;
				//return DIR_YS;
				return (Vector3i){
					chunk->low.x + floor(pos->x),
					chunk->low.y + 0,
					chunk->low.z + floor(pos->z)};
			}
		}
	}
	if (dir.y >= 0) {
		factor = distHigh.y / dir.y;
		result.x = dir.x * factor;
		if (distLow.x >= result.x && result.x >= distHigh.x) {
			result.z = dir.z * factor;
			if (distLow.z >= result.z && result.z >= distHigh.z) {
				pos->y = size.y;
				pos->x -= result.x;
				pos->z -= result.z;
				//return DIR_YG;
				return (Vector3i){
					chunk->low.x + floor(pos->x),
					chunk->low.y + size.y,
					chunk->low.z + floor(pos->z)};
			}
		}
	}

	if (dir.z <= 0) {
		factor = distLow.z / dir.z;
		result.x = dir.x * factor;
		if (distLow.x >= result.x && result.x >= distHigh.x) {
			result.y = dir.y * factor;
			if (distLow.y >= result.y && result.y >= distHigh.y) {
				pos->z = 0;
				pos->x -= result.x;
				pos->y -= result.y;
				//return DIR_ZS;
				return (Vector3i){
					chunk->low.x + floor(pos->x),
					chunk->low.y + floor(pos->y),
					chunk->low.z + 0};
			}
		}
	}
	if (dir.z >= 0) {
		factor = distHigh.z / dir.z;
		result.x = dir.x * factor;
		if (distLow.x >= result.x && result.x >= distHigh.x) {
			result.y = dir.y * factor;
			if (distLow.y >= result.y && result.y >= distHigh.y) {
				pos->z = size.z;
				pos->x -= result.x;
				pos->y -= result.y;
				//return DIR_ZG;
				return (Vector3i){
					chunk->low.x + floor(pos->x),
					chunk->low.y + floor(pos->y),
					chunk->low.z + size.z};
			}
		}
	}
	printf("This is bullshit!\n");
	return (Vector3i){0,0,0};
}

Chunk *chunkSearchNextChunk(Chunk *startChunk, Vector3i pos) {
	Chunk **chunk;
	printf("search: ");
	VECPRINT(startChunk->low, " - ");
	VECPRINT(startChunk->high, " - ");
	VECPRINT(pos, "\n");
	LISTITER(startChunk->adjacent, chunk, Chunk**) {
		if (VEC3CMP((*chunk)->low, <=, pos)
		 && VEC3CMP((*chunk)->high, >=, pos)) {
			return *chunk;
		}
	}
	printf("This is cowshit!\n");
	return NULL;
}

Chunk *chunkFindSolid(Chunk *chunk, Vector3f startPos, Vector3f dir) {
	Chunk *nextChunk;
	Vector3i nextPos, diff;
	Vector3f pos = VEC3FOP(startPos, -, chunk->low);
	/*pos.x = startPos.x - (float)(chunk->low.x);
	pos.y = startPos.y - (float)(chunk->low.y);
	pos.z = startPos.z - (float)(chunk->low.z);*/

	printf("begin --------------------\n");
	VECFPRINT(startPos, "\n");

	for (;;) {
		printf("iter\n");
		VECFPRINT(pos, "\n");
		nextPos = chunkTraceRay(chunk, &pos, dir);
		VECPRINT(nextPos, " - ");
		VECFPRINT(pos, "\n");
		if (nextPos.x == 0 && nextPos.y == 0 && nextPos.z == 0)
			return NULL;
		nextChunk = chunkSearchNextChunk(chunk, nextPos);
		if (!nextChunk)
			return NULL;
		if (nextChunk->blocks) {
			printf("found!!!!!!\n");
			return nextChunk;
		}
		
		diff = VEC3IOP(chunk->low, -, nextChunk->low);
		pos = VEC3FOP(pos, +, diff);
		chunk = nextChunk;
	}

	return NULL;
}
