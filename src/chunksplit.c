//! @file

#include <stdlib.h>
#include <assert.h>

#include "defs.h"
#include "chunk.h"
#include "world.h"
#include "chunksplit.h"

void moveBlocks(Chunk *old, Chunk *new)
{
	if (old->blocks == NULL) {
		new->blocks = NULL;
		return;
	}

	Vector3i size = (Vector3i){
		new->high.x - new->low.x + 1,
		new->high.y - new->low.y + 1,
		new->high.z - new->low.z + 1};
	int numBlocks = size.x * size.y * size.z;
	new->blocks = knalloc(numBlocks * sizeof(Block*));

	// FIXME: can't be tested before the generator uses more
	// than one block type; write proper code then
	int i;
	for (i=0; i<numBlocks; i++) {
		new->blocks[i] = old->blocks[0];
	}
}

#define UPDATEADJACENT(plane, x) \
void updateAdjacent ## plane(Chunk *old, Chunk *one, Chunk *two) \
{ \
	Chunk **chunk; \
 \
	one->adjacent = listNew(); \
	two->adjacent = listNew(); \
 \
	LISTITER(old->adjacent, chunk, Chunk**) { \
		if ((*chunk)->low.x <= one->high.x) { \
			/* current chunk touches chunk one */ \
			listReplace((*chunk)->adjacent, old, one); \
			listInsert(one->adjacent, *chunk); \
			if ((*chunk)->high.x >= two->low.x) { \
				/* current chunk also touches chunk two */ \
				listInsert((*chunk)->adjacent, two); \
				listInsert(two->adjacent, *chunk); \
			} \
		} else { \
			/* current chunk does not touch chunk one */ \
			/* ==> it must touch chunk two */ \
			listReplace((*chunk)->adjacent, old, two); \
			listInsert(two->adjacent, *chunk); \
		} \
	} \
}
UPDATEADJACENT(YZ, x)
UPDATEADJACENT(XZ, y)
UPDATEADJACENT(XY, z)

void updateWorldChunks(World *world, Chunk *chunk, Chunk *one, Chunk *two)
{
	// replace the pointer to the old chunk with a pointer to the first
	// new chunk:
	listReplace(world->chunks, chunk, one);

	// append a pointer to the second new chunk:
	listInsert(world->chunks, two);
}

void updateWorldGetChunk(World *world, Chunk *chunk, Chunk *one)
{
	if (world->lastChunk == chunk) {
		world->lastChunk = one;
		world->lastPos = one->low;
	}
}

void updateChunkGroup(World *world, ChunkGroup *group,
		Chunk *chunk, Chunk *one, Chunk *two)
{
	Vector3i low = group->low;
	Vector3i high = (Vector3i){
		low.x + world->groupSize.x - 1,
		low.y + world->groupSize.y - 1,
		low.z + world->groupSize.z - 1};

#define UPDATECHUNKGROUPCHECK(chunksXX, var) \
	if (chunk->var == var) { \
		listRemove(group->chunksXX, chunk); \
		if (one->var == var) \
			listInsert(group->chunksXX, one); \
		if (two->var == var) \
			listInsert(group->chunksXX, two); \
	}

	UPDATECHUNKGROUPCHECK(chunksXS, low.x)
	UPDATECHUNKGROUPCHECK(chunksYS, low.y)
	UPDATECHUNKGROUPCHECK(chunksZS, low.z)
	UPDATECHUNKGROUPCHECK(chunksXG, high.x)
	UPDATECHUNKGROUPCHECK(chunksYG, high.y)
	UPDATECHUNKGROUPCHECK(chunksZG, high.z)
}

void updateWorldChunkGroups(World *world,
		Chunk *chunk, Chunk *one, Chunk *two)
{
	ChunkGroup **group;
	Vector3i low;

	low.x = divRoundDown(chunk->low.x, world->groupSize.x);
	low.y = divRoundDown(chunk->low.y, world->groupSize.y);
	low.z = divRoundDown(chunk->low.z, world->groupSize.z);
	low = VEC3IOP(low, *, world->groupSize);

	LISTITER(world->chunkGroups, group, ChunkGroup**) {
		if (VEC3CMP((*group)->low, ==, low)) {
			updateChunkGroup(world, *group, chunk, one, two);
		}
	}
}

// world->chunksToUpdate is not updated, keep that in mind.
void chunksplitSplit(World *world, Chunk *chunk, int cutDir, int cutPos)
{
	Chunk *one = knalloc(sizeof(Chunk));
	Chunk *two = knalloc(sizeof(Chunk));

	one->status = two->status = chunk->status;
	one->cookie = two->cookie = chunk->cookie;

	one->low = two->low = chunk->low;
	one->high = two->high = chunk->high;

	switch (cutDir) {
		case PLANE_XY:
			one->high.z = cutPos;
			two->low.z = cutPos + 1;
			break;
		case PLANE_XZ:
			one->high.y = cutPos;
			two->low.y = cutPos + 1;
			break;
		case PLANE_YZ:
			one->high.x = cutPos;
			two->low.x = cutPos + 1;
			break;
	}

	if ( ! ( VEC3CMP(one->low, <=, one->high)
			&& VEC3CMP(two->low, <=, two->high))) {
		// some witzbold tries to create a chunk with zero thickness
		assert(0);
		return;
	}

	// move chunk->blocks to one/two->blocks
	moveBlocks(chunk, one);
	moveBlocks(chunk, two);

	// for each adj as chunk->adjacent: update adj->adjacent
	// in the same step: create one/two->adjacent
	switch (cutDir) {
		case PLANE_XY:
			updateAdjacentXY(chunk, one, two);
			break;
		case PLANE_XZ:
			updateAdjacentXZ(chunk, one, two);
			break;
		case PLANE_YZ:
			updateAdjacentYZ(chunk, one, two);
			break;
	}

	// link one and two
	listInsert(one->adjacent, two);
	listInsert(two->adjacent, one);

	// update world->chunks
	updateWorldChunks(world, chunk, one, two);

	// update world->lastChunk and world->lastPos
	updateWorldGetChunk(world, chunk, one);

	// update world->chunkGroups
	updateWorldChunkGroups(world, chunk, one, two);

	// TODO: free old chunk
}

