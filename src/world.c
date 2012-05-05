//! @file

#include <stdlib.h>
#include <stdio.h>

#include "defs.h"
#include "vector.h"
#include "world.h"
#include "chunkgen.h"
#include "chunksplit.h"
#include "bubble.h"

/**
 * Creates and initializes a World
 * 
 * @param gen		A block generator function
 */
World * worldInit(Block *(*gen)(Vector3i))
{
	World *world = knalloc(sizeof(World));
	world->cookie = 0;
	world->chunks = listNew(sizeof(Chunk*));
	world->bubbles = listNew(sizeof(Bubble*));

	world->generator = gen;

	world->lastChunk = NULL;

	world->chunksUpdated = 0;
	world->maxChunksToUpdate = 500;
	world->chunksToUpdate = listNew();

	world->bubblesUpdated = 0;
	world->maxBubblesToUpdate = 2;
	world->bubblesToUpdate = listNew();

	// chunkgen
	world->chunkGroups = listNew(sizeof(ChunkGroup*));
	world->groupSize = (Vector3i){16, 8, 16};
	chunkgenInit(world);

	return world;
}

Chunk * searchAllChunks(World *world, Vector3i pos)
{
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
 * Returns the chunk at position pos or NULL
 * 
 * If no chunk at that position exists, it is _not_ generated.
 * This function only returns chunks near the chunk that was last
 * returned by worldGetChunkFast() or worldGetChunk().
 */
Chunk * worldGetChunkFast(World *world, Vector3i pos)
{
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

		// check chunks adjacent to the last chunk
		Chunk **adjacent;
		LISTITER(world->lastChunk->adjacent, adjacent, Chunk**) {
			if (VEC3CMP((*adjacent)->low, <=, pos)
			 && VEC3CMP((*adjacent)->high, >=, pos)) {
				world->lastChunk = *adjacent;
				world->lastPos = pos;
				return *adjacent;
			}
		}

		// check chunks adjacent to the chunks adjacent to the
		// last chunk
		Chunk **middle;
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
	}

	return NULL;
}

/**
 * Returns the Chunk at position pos
 * 
 * If no Chunk at that position exists, it is created and returned.
 */
Chunk * worldGetChunk(World *world, Vector3i pos)
{
	Chunk *res;

	res = worldGetChunkFast(world, pos);
	if (res)
		return res;

	res = searchAllChunks(world, pos);
	if (res)
		return res;

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
 * Returns the ChunkGroup at position pos
 *
 * If there is no ChunkGroup at that position, return NULL
 */
ChunkGroup * worldGetChunkGroup(World *world, Vector3i pos)
{
	Vector3i low;

	low.x = divRoundDown(pos.x, world->groupSize.x);
	low.y = divRoundDown(pos.y, world->groupSize.y);
	low.z = divRoundDown(pos.z, world->groupSize.z);
	low = VEC3IOP(low, *, world->groupSize);

	ChunkGroup **group;
	LISTITER(world->chunkGroups, group, ChunkGroup**) {
		if (VEC3CMP((*group)->low, ==, low))
			return *group;
	}

	return NULL;
}

/**
 * Returns the Bubble the Chunk chunk belongs to.
 *
 * If chunk does not belong to any bubble, a new one is generated.
 *
 * If chunk is solid, NULL is returned.
 */
Bubble * worldGetBubbleFromChunk(World *world, Chunk *chunk)
{
	if (!chunk->bubble && !chunk->blocks)
		bubbleGen(world, chunk);

	return chunk->bubble;
}

/**
 * Returns the Bubble at position pos
 *
 * If no bubble at that position exists, it is generated.
 *
 * If there is a solid block at pos, NULL is returned.
 */
Bubble * worldGetBubble(World *world, Vector3i pos)
{
	Chunk *chunk = worldGetChunk(world, pos);

	return worldGetBubbleFromChunk(world, chunk);
}

/**
 * Marks a Bubble for updating by worldAfterFrame()
 */
void worldUpdateBubble(World *world, Bubble *bubble)
{
	if (bubble->status == 0)
		listInsert(world->bubblesToUpdate, bubble);
}

/**
 * Updates the Bubbles marked by worldUpdateBubble()
 *
 * TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO TODO
 *
 * This generates one BubbleGroup adjacent to each marked Bubble.
 * At most world->maxBubblesToUpdate Bubbles are updated (to limit the lag).
 */
void worldAfterFrame(World *world)
{
	Bubble** bubble;
	Vector3i pos;
	world->bubblesUpdated = 0;

	LISTITER(world->bubblesToUpdate, bubble, Bubble**) {
		int missing = (*bubble)->edge & (*bubble)->chunkGroup->status;

#ifdef MMB_DEBUG_CHUNK
		printf("Updating bubble %p, missing %i\n",
				*bubble, missing);
#endif

		if (missing & DIR_XG) {
			pos = (*bubble)->chunkGroup->low;
			pos.x += world->groupSize.x;
			chunkgenCreate(world, pos);
		}
		if (missing & DIR_XS) {
			pos = (*bubble)->chunkGroup->low;
			pos.x -= world->groupSize.x;
			chunkgenCreate(world, pos);
		}
		if (missing & DIR_YG) {
			pos = (*bubble)->chunkGroup->low;
			pos.y += world->groupSize.y;
			chunkgenCreate(world, pos);
		}
		if (missing & DIR_YS) {
			pos = (*bubble)->chunkGroup->low;
			pos.y -= world->groupSize.y;
			chunkgenCreate(world, pos);
		}
		if (missing & DIR_ZG) {
			pos = (*bubble)->chunkGroup->low;
			pos.z += world->groupSize.z;
			chunkgenCreate(world, pos);
		}
		if (missing & DIR_ZS) {
			pos = (*bubble)->chunkGroup->low;
			pos.z -= world->groupSize.z;
			chunkgenCreate(world, pos);
		}

		bubbleUpdate(world, *bubble);

		world->bubblesUpdated++;
		if (world->bubblesUpdated == world->maxBubblesToUpdate)
			break;
	}

	listEmpty(world->bubblesToUpdate);
}

/**
 * Sets the blocktype at position pos to block
 */
void worldSetBlock(World *world, Vector3i pos, Block *block)
{
	Chunk *chunk = chunksplitSplit(world, pos);

	if (block->solid == 0) {
		free(chunk->blocks);
		chunk->blocks = NULL;
		bubbleMerge(world, chunk);
	} else {
		if (chunk->blocks == NULL)
			chunk->blocks = knalloc(sizeof(Block*));
		chunk->blocks[0] = block;
		bubbleSplit(world, chunk);
	}
}

