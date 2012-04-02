//! @file

#include <stdlib.h>
#include <assert.h>

#include "defs.h"
#include "bubble.h"
#include "world.h"

void bubbleGenRecurse(World *world, Vector3i low, Vector3i high,
		Bubble *bubble, Chunk *chunk)
{
	chunk->bubble = bubble;

	// set edge flags
	if (chunk->high.x == high.x)
		bubble->edge |= DIR_XG;
	if (chunk->high.y == high.y)
		bubble->edge |= DIR_YG;
	if (chunk->high.z == high.z)
		bubble->edge |= DIR_ZG;
	if (chunk->low.x == low.x)
		bubble->edge |= DIR_XS;
	if (chunk->low.y == low.y)
		bubble->edge |= DIR_YS;
	if (chunk->low.z == low.z)
		bubble->edge |= DIR_ZS;

	Chunk **other;
	LISTITER(chunk->adjacent, other, Chunk**) {
		if ((*other)->bubble == NULL
				&& VEC3CMP(low, <=, (*other)->low)
				&& VEC3CMP((*other)->high, <=, high)
				&& (*other)->blocks == NULL)
			bubbleGenRecurse(world, low, high, bubble, *other);
	}
}

Bubble * bubbleGen(World *world, Chunk *chunk)
{
	Bubble *bubble = knalloc(sizeof(Bubble));
	bubble->chunk = chunk;
	bubble->adjacent = listNew();
	bubble->edge = 0;
	bubble->cookie = 0;

	bubble->chunkGroup = worldGetChunkGroup(world, chunk->low);
	assert(bubble->chunkGroup != NULL);

	Vector3i low, high;
	low.x = divRoundDown(chunk->low.x, world->groupSize.x);
	low.y = divRoundDown(chunk->low.y, world->groupSize.y);
	low.z = divRoundDown(chunk->low.z, world->groupSize.z);
	low = VEC3IOP(low, *, world->groupSize);

	high.x = low.x + world->groupSize.x - 1;
	high.y = low.y + world->groupSize.y - 1;
	high.z = low.z + world->groupSize.z - 1;

	bubbleGenRecurse(world, low, high, bubble, chunk);

	return bubble;
}

void bubbleUpdateRecurse(World *world, Bubble *bubble, Chunk *chunk)
{
	chunk->cookie = world->cookie;

	Bubble *otherBubble;
	Chunk **other;
	LISTITER(chunk->adjacent, other, Chunk**) {
		if ((*other)->cookie == world->cookie)
			continue;

		if ((*other)->blocks != NULL)
			continue;

		otherBubble = worldGetBubbleFromChunk(world, *other);

		if (otherBubble != bubble) {
			assert(otherBubble->chunkGroup != bubble->chunkGroup);

			if (otherBubble->cookie == world->cookie)
				continue;

			listInsert(otherBubble->adjacent, bubble);
			listInsert(bubble->adjacent, otherBubble);

			otherBubble->cookie = world->cookie;
		} else {
			bubbleUpdateRecurse(world, bubble, *other);
		}
	}
}

void bubbleUpdate(World *world, Bubble *bubble)
{
	assert((bubble->edge & bubble->chunkGroup->status) == 0);

	world->cookie++;
	bubbleUpdateRecurse(world, bubble, bubble->chunk);
}

