#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "chunk.h"

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

void chunkTagBlocks(AnnotatedBlock *blocks, int sizeY, int sizeZ,
		Point3i low, Point3i high, Chunk *chunk) {
	AnnotatedBlock *block;
	int x,y,z;
	for (x = low.x; x <= high.x; x++) {
	for (y = low.y; y <= high.y; y++) {
	for (z = low.z; z <= high.z; z++) {
		block = blocks + x*sizeY*sizeZ + y*sizeZ + z;
		if (block->chunk != NULL) {
			fprintf(stderr, "FUUUUUUUUUUUU\n");
			//exit(EXIT_FAILURE);
		}
		if (!POINTCMP(block->high2, ==, high)) {
			fprintf(stderr, "FU FU FU FU\n");
			//exit(EXIT_FAILURE);
		}
		if (!POINTCMP(block->low2, ==, low)) {
			fprintf(stderr, "FU FU FU FU low\n");
			//exit(EXIT_FAILURE);
		}
		block->chunk = chunk;
	}
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

	// init: set some values to 0 or size[XYZ]-1
	// the following loop is extremly stupid, but I can't think properly
	// at the moment and I really want to know if the 1st pass loop works
	AnnotatedBlock *tmp = blocks;
	for (x = 0; x < sizeX; x++) {
	for (y = 0; y < sizeY; y++) {
	for (z = 0; z < sizeZ; z++) {
		if (x == sizeX - 1) {
			tmp->high.x = x;
			tmp->high2.x = x;
		}
		if (y == sizeY - 1) {
			tmp->high.y = y;
			tmp->high2.y = y;
		}
		if (z == sizeZ - 1) {
			tmp->high.z = z;
			tmp->high2.z = z;
		}
		if (x == 0) {
			tmp->low.x = x;
			tmp->low2.x = x;
		}
		if (y == 0) {
			tmp->low.y = y;
			tmp->low2.y = y;
		}
		if (z == 0) {
			tmp->low.z = z;
			tmp->low2.z = z;
		}
		tmp->chunk = NULL;
		tmp++;
	}
	}
	}

	Point p;
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

	// first pass
	tmp = blocks + blockcount - 1; // start at the last block
	AnnotatedBlock *tmpX = tmp + sizeZ * sizeY;
	AnnotatedBlock *tmpY = tmp + sizeZ;
	AnnotatedBlock *tmpZ = tmp + 1;
	for (x = sizeX - 1; x >= 0; x--) {
	for (y = sizeY - 1; y >= 0; y--) {
	for (z = sizeZ - 1; z >= 0; z--) {
		if (x < sizeX - 1) {
			// compare with next block in .xG-dir.
			if (*(tmp->block) == *(tmpX->block)) {
				tmp->high.x = tmpX->high.x;
			} else {
				tmp->high.x = x;
			}
		}
		if (y < sizeY - 1) {
			// compare with neyt block in .yG-dir.
			if (*(tmp->block) == *(tmpY->block)) {
				tmp->high.y = tmpY->high.y;
			} else {
				tmp->high.y = y;
			}
		}
		if (z < sizeZ - 1) {
			// compare with nezt block in .zG-dir.
			if (*(tmp->block) == *(tmpZ->block)) {
				tmp->high.z = tmpZ->high.z;
			} else {
				tmp->high.z = z;
			}
		}
		tmp--;
		tmpX--; tmpY--; tmpZ--;
	}
	}
	}

	// second pass
	// similar to the first, but in opposite direction
	// additionally, chunk-like stuff gets extracted and saved
	tmp = blocks;
	tmpX = tmp - sizeZ * sizeY;
	tmpY = tmp - sizeZ;
	tmpZ = tmp - 1;
	//List *chunks = listNew();
	for (x = 0; x < sizeX; x++) {
	for (y = 0; y < sizeY; y++) {
	for (z = 0; z < sizeZ; z++) {
		if (x > 0) {
			// compare with next block in .xS-dir.
			if (*(tmp->block) == *(tmpX->block)) {
				tmp->low.x = tmpX->low.x;
			} else {
				tmp->low.x = x;
			}
		}
		if (y > 0) {
			// compare with neyt block in .yS-dir.
			if (*(tmp->block) == *(tmpY->block)) {
				tmp->low.y = tmpY->low.y;
			} else {
				tmp->low.y = y;
			}
		}
		if (z > 0) {
			// compare with nezt block in .zS-dir.
			if (*(tmp->block) == *(tmpZ->block)) {
				tmp->low.z = tmpZ->low.z;
			} else {
				tmp->low.z = z;
			}
		}
		// ------------------
		if (x > 0) {
			// compare with next block in XS-dir.
			if (POINTCMP(tmp->high, ==, tmpX->high)
			 && POINTCMP(tmp->low, ==, tmpX->low)) {
				tmp->low2.x = tmpX->low2.x;
			} else {
				tmp->low2.x = x;
			}
		}
		if (y > 0) {
			// compare with neyt block in YS-dir.
			if (POINTCMP(tmp->high, ==, tmpY->high)
			 && POINTCMP(tmp->low, ==, tmpY->low)) {
				tmp->low2.y = tmpY->low2.y;
			} else {
				tmp->low2.y = y;
			}
		}
		if (z > 0) {
			// compare with nezt block in ZS-dir.
			if (POINTCMP(tmp->high, ==, tmpZ->high)
			 && POINTCMP(tmp->low, ==, tmpZ->low)) {
				tmp->low2.z = tmpZ->low2.z;
			} else {
				tmp->low2.z = z;
			}
		}
		tmp++;
		tmpX++; tmpY++; tmpZ++;
	}
	}
	}

	Chunk *newChunk;
	int chunkSizeX, chunkSizeY, chunkSizeZ;

	int total = 0;
	// third pass
	tmp = blocks + blockcount - 1; // start at the last block
	tmpX = tmp + sizeZ * sizeY;
	tmpY = tmp + sizeZ;
	tmpZ = tmp + 1;
	for (x = sizeX - 1; x >= 0; x--) {
	for (y = sizeY - 1; y >= 0; y--) {
	for (z = sizeZ - 1; z >= 0; z--) {
		if (x < sizeX - 1) {
			// compare with next block in XG-dir.
			if (POINTCMP(tmp->high, ==, tmpX->high)
			 && POINTCMP(tmp->low, ==, tmpX->low)) {
				tmp->high2.x = tmpX->high2.x;
			} else {
				tmp->high2.x = x;
			}
		}
		if (y < sizeY - 1) {
			// compare with neyt block in YG-dir.
			if (POINTCMP(tmp->high, ==, tmpY->high)
			 && POINTCMP(tmp->low, ==, tmpY->low)) {
				tmp->high2.y = tmpY->high2.y;
			} else {
				tmp->high2.y = y;
			}
		}
		if (z < sizeZ - 1) {
			// compare with nezt block in ZG-dir.
			if (POINTCMP(tmp->high, ==, tmpZ->high)
			 && POINTCMP(tmp->low, ==, tmpZ->low)) {
				tmp->high2.z = tmpZ->high2.z;
			} else {
				tmp->high2.z = z;
			}
		}
		if (tmp->low2.x == x && tmp->low2.y == y && tmp->low2.z == z) {
			// found a chunk!
			total++;
			printf("Chunk: %i/%i/%i  %i/%i/%i\n",
				tmp->low2.x, tmp->low2.y, tmp->low2.z,
				tmp->high2.x, tmp->high2.y, tmp->high2.z);
			newChunk = chunkCreateBare(world);
			newChunk->low.x = low.x + tmp->low2.x;
			newChunk->low.y = low.y + tmp->low2.y;
			newChunk->low.z = low.z + tmp->low2.z;
			newChunk->high.x = low.x + tmp->high2.x;
			newChunk->high.y = low.y + tmp->high2.y;
			newChunk->high.z = low.z + tmp->high2.z;
			chunkSizeX = tmp->high2.x - tmp->low2.x;
			chunkSizeY = tmp->high2.y - tmp->low2.y;
			chunkSizeZ = tmp->high2.z - tmp->low2.z;
			chunkTagBlocks(blocks, sizeY, sizeZ,
					tmp->low2, tmp->high2, newChunk);
		}
		tmp--;
		tmpX--; tmpY--; tmpZ--;
	}
	}
	}

	printf("x y z | low   | high  | low2  | high2 | chunk\n");
	printf("------+-------+-------+-------+-------+------\n");
	tmp = blocks;
	for (x = 0; x < sizeX; x++) {
	for (y = 0; y < sizeY; y++) {
	for (z = 0; z < sizeZ; z++) {
		printf("%i %i %i | %i %i %i | %i %i %i | %i %i %i | %i %i %i"
				" | %p\n",
				x, y, z,
				tmp->low.x, tmp->low.y, tmp->low.z,
				tmp->high.x, tmp->high.y, tmp->high.z,
				tmp->low2.x, tmp->low2.y, tmp->low2.z,
				tmp->high2.x, tmp->high2.y, tmp->high2.z,
				tmp->chunk);
		tmp++;
	}
	}
	}
	printf("\n");

	printf("Found %i chunks in %i blocks (%i%%)\n", total,
			blockcount,
			(int)((float)total / blockcount * 100));

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

