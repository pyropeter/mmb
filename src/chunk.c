#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"

Chunk *chunkCreate(Metachunk *world, Point pos) {
	//printf("chunkCreate(): ");
	//pointPrint(pos, "\n");

	Chunk *chunk = knalloc(sizeof(Chunk));
	chunk->status = 0;
	chunk->lastRender = 0;

	chunk->adjacentCount = 0;
	chunk->adjacent = knalloc(6 * sizeof(Chunk*));

	chunk->low = pos;
	chunk->high = pos;

	Block *block = world->generator(pos);
	if (*block == ' ') {
		chunk->blocks = NULL;
	} else {
		chunk->blocks = knalloc(sizeof(Block*));
		chunk->blocks[0] = block;
	}

	listInsert(world->chunks, chunk);
	return chunk;
}

Metachunk *chunkInit(Block *(*gen)(Point), Point pos) {
	Metachunk *world = knalloc(sizeof(Metachunk));
	world->chunks = listNew(sizeof(Chunk*));

	world->generator = gen;

	Chunk *chunk = chunkCreate(world, pos);
	world->lastPos = pos;
	world->lastChunk = chunk;

	return world;
}

Chunk *chunkGet(Metachunk *world, Point pos) {
	// check if position has changed at all
	if (POINTCMP(world->lastPos, ==, pos))
		return world->lastChunk;

	// check if point is still in last chunk
	if (POINTCMP(world->lastChunk->low, <=, pos) &&
			POINTCMP(world->lastChunk->high, >=, pos)) {
		world->lastPos = pos;
		return world->lastChunk;
	}

	// check chunks adjacent to the chunks adjacent to the last chunk
	int i, j;
	for (i = 0; i < world->lastChunk->adjacentCount; i++) {
		Chunk *middle = world->lastChunk->adjacent[i];
		for (j = 0; j < middle->adjacentCount; j++) {
			Chunk *adjacent = middle->adjacent[j];
			if (POINTCMP(adjacent->low, <=, pos) &&
					POINTCMP(adjacent->high, >=, pos)) {
				world->lastChunk = adjacent;
				world->lastPos = pos;
				return adjacent;
			}
		}
	}

	// just check all chunks (boring, slow but simple approach)
	Chunk **chunk = (Chunk**)world->chunks->mem;
	for (; (void**)chunk != world->chunks->nextFree; chunk++) {
		if (POINTCMP((*chunk)->low, <=, pos) &&
				POINTCMP((*chunk)->high, >=, pos)) {
			world->lastPos = pos;
			world->lastChunk = *chunk;
			return world->lastChunk;
		}
	}

	printf("Camera moved out of known chunks, crashing\n");
	exit(EXIT_FAILURE);
	return NULL;
}

void chunkUpdate(Metachunk *world, Chunk *chunk) {
	// chunkUpdate() finds all the neighbors of a chunk (and creates them,
	// if they were not created already)

	if (chunk->status < 1) {
		// the following code assumes the chunk is one block in size
		//long timer = startTimer();

		chunk->adjacentCount = 0;
		int existingChunks = 0;

		Chunk **otherChunk = (Chunk**)world->chunks->mem;
		for (; (void**)otherChunk != world->chunks->nextFree;
				otherChunk++) {
			if (chunk->high.x + 1 == (*otherChunk)->low.x
			&& chunk->high.y >= (*otherChunk)->low.y
			&& chunk->high.z >= (*otherChunk)->low.z
			&& chunk->low.y <= (*otherChunk)->high.y
			&& chunk->low.z <= (*otherChunk)->high.z) {
				existingChunks += DIR_XG;
			} else if (chunk->low.x - 1 == (*otherChunk)->high.x
			&& chunk->high.y >= (*otherChunk)->low.y
			&& chunk->high.z >= (*otherChunk)->low.z
			&& chunk->low.y <= (*otherChunk)->high.y
			&& chunk->low.z <= (*otherChunk)->high.z) {
				existingChunks += DIR_XS;
			} else if (chunk->high.y + 1 == (*otherChunk)->low.y
			&& chunk->high.x >= (*otherChunk)->low.x
			&& chunk->high.z >= (*otherChunk)->low.z
			&& chunk->low.x <= (*otherChunk)->high.x
			&& chunk->low.z <= (*otherChunk)->high.z) {
				existingChunks += DIR_YG;
			} else if (chunk->low.y - 1 == (*otherChunk)->high.y
			&& chunk->high.x >= (*otherChunk)->low.x
			&& chunk->high.z >= (*otherChunk)->low.z
			&& chunk->low.x <= (*otherChunk)->high.x
			&& chunk->low.z <= (*otherChunk)->high.z) {
				existingChunks += DIR_YS;
			} else if (chunk->high.z + 1 == (*otherChunk)->low.z
			&& chunk->high.x >= (*otherChunk)->low.x
			&& chunk->high.y >= (*otherChunk)->low.y
			&& chunk->low.x <= (*otherChunk)->high.x
			&& chunk->low.y <= (*otherChunk)->high.y) {
				existingChunks += DIR_ZG;
			} else if (chunk->low.z - 1 == (*otherChunk)->high.z
			&& chunk->high.x >= (*otherChunk)->low.x
			&& chunk->high.y >= (*otherChunk)->low.y
			&& chunk->low.x <= (*otherChunk)->high.x
			&& chunk->low.y <= (*otherChunk)->high.y) {
				existingChunks += DIR_ZS;
			} else {
				continue;
			}
			chunk->adjacent[chunk->adjacentCount++] = *otherChunk;
		}

		if ((existingChunks & DIR_XG) == 0)
			chunk->adjacent[chunk->adjacentCount++] = chunkCreate(
					world, (Point){chunk->low.x + 1,
					chunk->low.y, chunk->low.z});
		if ((existingChunks & DIR_XS) == 0)
			chunk->adjacent[chunk->adjacentCount++] = chunkCreate(
					world, (Point){chunk->low.x - 1,
					chunk->low.y, chunk->low.z});
		if ((existingChunks & DIR_YG) == 0)
			chunk->adjacent[chunk->adjacentCount++] = chunkCreate(
					world, (Point){chunk->low.x,
					chunk->low.y + 1, chunk->low.z});
		if ((existingChunks & DIR_YS) == 0)
			chunk->adjacent[chunk->adjacentCount++] = chunkCreate(
					world, (Point){chunk->low.x,
					chunk->low.y - 1, chunk->low.z});
		if ((existingChunks & DIR_ZG) == 0)
			chunk->adjacent[chunk->adjacentCount++] = chunkCreate(
					world, (Point){chunk->low.x,
					chunk->low.y, chunk->low.z + 1});
		if ((existingChunks & DIR_ZS) == 0)
			chunk->adjacent[chunk->adjacentCount++] = chunkCreate(
					world, (Point){chunk->low.x,
					chunk->low.y, chunk->low.z - 1});

		//printf("Search for adjacent took %lims\n", stopTimer(timer));
		chunk->status = 1;
		//printf("chunkUpdate(): found %i for ", chunk->adjacentCount);
		//pointPrint(chunk->low, "\n");
	}
}

