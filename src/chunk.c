#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"

Chunk *chunkCreate(Metachunk *world, Point pos) {
	printf("chunkCreate(): %i/%i/%i\n", pos.x, pos.y, pos.z);

	Chunk *chunk = malloc(sizeof(Chunk));
	chunk->status = 0;
	chunk->neighborCount = 0;

	chunk->low = pos;
	chunk->high = pos;

	Block *block = world->generator(pos);
	if (*block == ' ') {
		chunk->blocks = NULL;
	} else {
		chunk->blocks = malloc(sizeof(Block*));
		chunk->blocks[0] = block;
	}

	listInsert(world->chunks, &chunk);
	return chunk;
}

Metachunk *chunkInit(Block *(*gen)(Point), Point pos) {
	Metachunk *world = malloc(sizeof(Metachunk));
	world->chunks = listNew(sizeof(Chunk*));

	world->generator = gen;

	Chunk *chunk = chunkCreate(world, pos);
	world->lastPos = pos;
	world->lastChunk = chunk;

	return world;
}

Chunk *chunkGet(Metachunk *world, Point pos) {
	if (POINTCMP(world->lastPos, ==, pos))
		return world->lastChunk;

	if (POINTCMP(world->lastChunk->low, <=, pos) &&
			POINTCMP(world->lastChunk->high, >=, pos)) {
		world->lastPos = pos;
		return world->lastChunk;
	}

	int i;
	for (i = 0; i < world->lastChunk->neighborCount; i++) {
		Chunk *neighbor = world->lastChunk->neighbors[i];
		if (POINTCMP(neighbor->low, <=, pos) &&
				POINTCMP(neighbor->high, >=, pos)) {
			world->lastChunk = neighbor;
			world->lastPos = pos;
			return neighbor;
		}
	}

	return NULL;
}

void chunkUpdate(Metachunk *world, Chunk *chunk) {
	//printf("chunkUpdate(): %i/%i/%i %i\n",
	//		chunk->low.x, chunk->low.y, chunk->low.z, chunk->status);

	if (chunk->status < 1) {
		Chunk *newChunk = chunkCreate(world, (Point){
			chunk->low.x, chunk->low.y - 1, chunk->low.z});
		newChunk->neighbors[newChunk->neighborCount++] = chunk;
		chunk->neighbors[chunk->neighborCount++] = newChunk;
		chunk->status = 1;
	}
}

