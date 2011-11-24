#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "chunk.h"

static int error = 0;

Chunk *chunkCreateBare(Metachunk *world) {
	Chunk *chunk = knalloc(sizeof(Chunk));
	chunk->status = 0;
	chunk->lastRender = 0;

	chunk->adjacent = listNew();
	chunk->blocks = NULL;

	listInsert(world->chunks, chunk);
	return chunk;
}

/*Chunk *chunkCreate(Metachunk *world, Point pos) {
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
}*/

void chunkTagBlocks(AnnotatedBlock *blocks, int sizeY, int sizeZ, int z,
		int lowx, int lowy, int highx, int highy, Chunk *chunk) {
	AnnotatedBlock *block;
	int x,y;
	for (x = lowx; x <= highx; x++) {
	for (y = lowy; y <= highy; y++) {
		block = blocks + x*sizeY*sizeZ + y*sizeZ + z;
		if (block->chunk != NULL) {
			printf("FUUUUUUUUUUUU\n");
			error++;
		}
		if (block->high2x != highx || block->high2y != highy) {
			printf("wrong high: %i %i\n",
					block->high2x,
					block->high2y);
			error++;
		}
		if (block->low2x != lowx || block->low2y != lowy) {
			printf("wrong  low: %i %i\n",
					block->low2x,
					block->low2y);
			error++;
		}
		block->chunk = chunk;
		//printf("----\n");
	}
	}
}

void chunkCreateBatch(Metachunk *world, Point low, Point high) {
	int x,y,z;
	int sizeX = high.x - low.x;
	int sizeY = high.y - low.y;
	int sizeZ = high.z - low.z;
	int blockcount = sizeX * sizeY * sizeZ;
	AnnotatedBlock *blocks = knalloc(blockcount * sizeof(AnnotatedBlock));
	AnnotatedBlock *tmp, *tmpX, *tmpY;
	Chunk *newChunk;
	//int chunkSizeX, chunkSizeY, chunkSizeZ;
	int total = 0;
	Point p;

	// init: set some values to 0 or size[XYZ]-1
	// the following loop is extremly stupid, but I can't think properly
	// at the moment and I really want to know if the 1st pass loop works
	tmp = blocks;
	for (x = 0; x < sizeX; x++) {
	for (y = 0; y < sizeY; y++) {
	for (z = 0; z < sizeZ; z++) {
		tmp->lowx = tmp->highx = x;
		tmp->lowy = tmp->highy = y;
		tmp->low2x = tmp->high2x = x;
		tmp->low2y = tmp->high2y = y;
		tmp->chunk = NULL;
		tmp++;
	}
	}
	}

	// load blocks from map generator into blocks
	tmp = blocks;
	for (p.x = low.x; p.x < high.x; p.x++) {
	for (p.y = low.y; p.y < high.y; p.y++) {
	for (p.z = low.z; p.z < high.z; p.z++) {
		(tmp++)->block = world->generator(p);
	}
	}
	}
	for (p.y = low.y; p.y < high.y; p.y++) {
		printf("y = %2i:\n", p.y - low.y);
		for (p.z = high.z - 1; p.z >= low.z; p.z--) {
			printf("%2i |", p.z - low.z);
			for (p.x = low.x; p.x < high.x; p.x++) {
				printf(" %c", *(world->generator(p)));
			}
			printf(" |\n");
		}
		printf("---+--------------------------------\n");
		printf(" z | x 1 2 3 4 5 6 7 8 9 a b c d e f\n");
		printf("\n");
	}

	// set high
	tmp = blocks + blockcount - 1;
	tmpX = tmp + sizeZ * sizeY;
	tmpY = tmp + sizeZ;
	for (x = sizeX - 1; x >= 0; x--) {
	for (y = sizeY - 1; y >= 0; y--) {
	for (z = sizeZ - 1; z >= 0; z--) {
		if (x < sizeX - 1) {
			// compare with next block in XG-dir.
			if (*(tmp->block) == *(tmpX->block)) {
				tmp->highx = tmpX->highx;
			} else {
				tmp->highx = x;
			}
		}
		if (y < sizeY - 1) {
			// compare with next block in YG-dir.
			if (*(tmp->block) == *(tmpY->block)) {
				tmp->highy = tmpY->highy;
			} else {
				tmp->highy = y;
			}
		}
		tmp--;
		tmpX--; tmpY--;
	}
	}
	}

	// set low and low2
	tmp = blocks;
	tmpX = tmp - sizeZ * sizeY;
	tmpY = tmp - sizeZ;
	for (x = 0; x < sizeX; x++) {
	for (y = 0; y < sizeY; y++) {
	for (z = 0; z < sizeZ; z++) {
		if (x > 0) {
			// compare with next block in XS-dir.
			if (*(tmp->block) == *(tmpX->block)) {
				tmp->lowx = tmpX->lowx;
			} else {
				tmp->lowx = x;
			}
		}
		if (y > 0) {
			// compare with neyt block in YS-dir.
			if (*(tmp->block) == *(tmpY->block)) {
				tmp->lowy = tmpY->lowy;
			} else {
				tmp->lowy = y;
			}
		}
		if (x > 0) {
			// compare with next block in XS-dir.
			if (tmp->lowx == tmpX->lowx
			 && tmp->lowy == tmpX->lowy
			 && tmp->highx == tmpX->highx
			 && tmp->highy == tmpX->highy) {
				tmp->low2x = tmpX->low2x;
			} else {
				tmp->low2x = x;
			}
		}
		if (y > 0) {
			// compare with neyt block in YS-dir.
			if (tmp->lowx == tmpY->lowx
			 && tmp->lowy == tmpY->lowy
			 && tmp->highx == tmpY->highx
			 && tmp->highy == tmpY->highy) {
				tmp->low2y = tmpY->low2y;
			} else {
				tmp->low2y = y;
			}
		}
		tmp++;
		tmpX++; tmpY++;
	}
	}
	}

	// set high2
	tmp = blocks + blockcount - 1;
	tmpX = tmp + sizeZ * sizeY;
	tmpY = tmp + sizeZ;
	for (x = sizeX - 1; x >= 0; x--) {
	for (y = sizeY - 1; y >= 0; y--) {
	for (z = sizeZ - 1; z >= 0; z--) {
		if (x < sizeX - 1) {
			// compare with next block in XS-dir.
			if (tmp->lowx == tmpX->lowx
			 && tmp->lowy == tmpX->lowy
			 && tmp->highx == tmpX->highx
			 && tmp->highy == tmpX->highy) {
				tmp->high2x = tmpX->high2x;
			} else {
				tmp->high2x = x;
			}
		}
		if (y < sizeY - 1) {
			// compare with neyt block in YS-dir.
			if (tmp->lowx == tmpY->lowx
			 && tmp->lowy == tmpY->lowy
			 && tmp->highx == tmpY->highx
			 && tmp->highy == tmpY->highy) {
				tmp->high2y = tmpY->high2y;
			} else {
				tmp->high2y = y;
			}
		}
		tmp--;
		tmpX--; tmpY--;
	}
	}
	}

	// search for chunks
	tmp = blocks;
	for (x = 0; x < sizeX; x++) {
	for (y = 0; y < sizeY; y++) {
	for (z = 0; z < sizeZ; z++) {
		if (tmp->low2x == x
		 && tmp->low2y == y) {
			total++;
			printf("Chunk: %i %i/%i  %i/%i\n",
				z,
				tmp->low2x, tmp->low2y,
				tmp->high2x, tmp->high2y);
			newChunk = chunkCreateBare(world);
			newChunk->low.x = low.x + tmp->low2x;
			newChunk->low.y = low.y + tmp->low2y;
			newChunk->low.z = low.z + z;
			newChunk->high.x = low.x + tmp->high2x;
			newChunk->high.y = low.y + tmp->high2y;
			newChunk->high.z = low.z + z;
			//chunkSizeX = tmp->high4.x - tmp->low4.x;
			//chunkSizeY = tmp->high4.y - tmp->low4.y;
			//chunkSizeZ = tmp->high4.z - tmp->low4.z;
			chunkTagBlocks(blocks, sizeY, sizeZ, z,
					tmp->low2x, tmp->low2y,
					tmp->high2x, tmp->high2y,
					newChunk);
		}
		tmp++;
	}
	}
	}

	printf("x y z | low | hig | lo2 | hi2 | chunk\n");
	printf("------+-----+-----+-----+-----+------\n");
	tmp = blocks;
	for (x = 0; x < sizeX; x++) {
	for (y = 0; y < sizeY; y++) {
	for (z = 0; z < sizeZ; z++) {
		printf("%i %i %i | %i %i | %i %i | %i %i | %i %i | %p\n",
				x, y, z,
				tmp->lowx, tmp->lowy,
				tmp->highx, tmp->highy,
				tmp->low2x, tmp->low2y,
				tmp->high2x, tmp->high2y,
				tmp->chunk);
		tmp++;
	}
	}
	}
	printf("\n");

	printf("Found %i chunks in %i blocks (%i%%)\n", total,
			blockcount,
			(int)((float)total / blockcount * 100));

	printf("error: %i\n", error);

	exit(EXIT_FAILURE);

	return;
}

Metachunk *chunkInit(Block *(*gen)(Point), Point pos) {
	Metachunk *world = knalloc(sizeof(Metachunk));
	world->chunks = listNew(sizeof(Chunk*));

	world->generator = gen;

	world->lastPos = (Point){0, 0, 0};  // unlikely position
	world->lastChunk = NULL;

	chunkCreateBatch(world,
			(Point){pos.x-10, pos.y-10, pos.z-10},
			(Point){pos.x+10, pos.y+10, pos.z+10});
	/*chunkCreateBatch(world,
			(Point){pos.x-4, pos.y-4, pos.z-4},
			(Point){pos.x+5, pos.y+5, pos.z+5});*/
	chunkCreateBatch(world,
			(Point){pos.x-2, pos.y-2, pos.z-2},
			(Point){pos.x+1, pos.y+1, pos.z+1});

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
	Chunk **middle, **adjacent;
	LISTITER(world->lastChunk->adjacent, middle, Chunk**) {
		LISTITER((*middle)->adjacent, adjacent, Chunk**) {
			if (POINTCMP((*adjacent)->low, <=, pos)
			 && POINTCMP((*adjacent)->high, >=, pos)) {
				world->lastChunk = *adjacent;
				world->lastPos = pos;
				return *adjacent;
			}
		}
	}
/*	int i, j;
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
	}*/

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
	return;
}

/*void chunkUpdate(Metachunk *world, Chunk *chunk) {
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
}*/

