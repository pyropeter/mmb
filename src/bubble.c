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
	bubble->status = 0;
	bubble->cookie = 0;

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

