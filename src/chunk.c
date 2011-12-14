/**
 * @file
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "defs.h"
#include "vector.h"
#include "chunk.h"

Chunk *chunkCreateBare(Metachunk *world) {
	Chunk *chunk = knalloc(sizeof(Chunk));
	chunk->status = 0;
	chunk->cookie = -1;

	chunk->adjacent = listNew();
	chunk->blocks = NULL;

	listInsert(world->chunks, chunk);
	return chunk;
}

ChunkGroup *newChunkGroup(Metachunk *world, Vector3i low) {
	ChunkGroup *chunkGroup = knalloc(sizeof(ChunkGroup));
	listInsert(world->chunkGroups, chunkGroup);
	
	chunkGroup->low = low;
	chunkGroup->chunksXS = listNew();
	chunkGroup->chunksXG = listNew();
	chunkGroup->chunksYS = listNew();
	chunkGroup->chunksYG = listNew();
	chunkGroup->chunksZS = listNew();
	chunkGroup->chunksZG = listNew();
	
	return chunkGroup;
}

void chunkTagBlocks(AnnotatedBlock *blocks, int sizeY, int sizeZ, int z,
		int lowx, int lowy, int highx, int highy, Chunk *chunk) {
	AnnotatedBlock *block;
	int blocksIndex = 0;
	int x,y;
	for (x = lowx; x <= highx; x++) {
	for (y = lowy; y <= highy; y++) {
		block = blocks + x*sizeY*sizeZ + y*sizeZ + z;

		assert(block->chunk == NULL);
		assert(block->high2x == highx && block->high2y == highy);
		assert(block->low2x == lowx && block->low2y == lowy);

		block->chunk = chunk;
		if (chunk->blocks != NULL) {
			chunk->blocks[blocksIndex++] = block->block;
		}
	}
	}
}

void handleChunk(Metachunk *world, AnnotatedBlock *blocks,
		ChunkGroup *chunkGroup, int sizeX, int sizeY, int sizeZ,
		Vector3i low, int z, AnnotatedBlock *first) {
	Chunk *chunk;
	int chunkSizeX, chunkSizeY;

	/*printf("Chunk: %2i %2i/%2i  %2i/%2i\n",
		z,
		first->low2x, first->low2y,
		first->high2x, first->high2y);*/

	chunk = chunkCreateBare(world);

	// set low and high points of the new chunk
	chunk->low.x = low.x + first->low2x;
	chunk->low.y = low.y + first->low2y;
	chunk->low.z = low.z + z;
	chunk->high.x = low.x + first->high2x;
	chunk->high.y = low.y + first->high2y;
	chunk->high.z = low.z + z;

	// transparent or not?
	// allocate memory for block pointers
	if (*(first->block) == ' ') {
		chunk->blocks = NULL;
	} else {
		chunkSizeX = first->high2x - first->low2x + 1;
		chunkSizeY = first->high2y - first->low2y + 1;
		chunk->blocks = knalloc(
				chunkSizeX * chunkSizeY * 1
				* sizeof(Block*));
	}

	// add blocks to the chunk
	chunkTagBlocks(blocks, sizeY, sizeZ, z,
			first->low2x, first->low2y,
			first->high2x, first->high2y,
			chunk);

	// Handle adjacent chunks:
	// Only the chunks in XS, YS and ZS direction are relevant, the others
	// are not yet generated.
	// There is always only one adjacent chunk in X and one in Y direction
	// (this is one property of the chunk generating algorithm).
	// In the Z direction the number of adjacent chunks is between one and
	// the number of blocks in the current chunk.

	// add the chunks in XS and YS direction
	if (first->low2x > 0) {
		listInsert(chunk->adjacent, (first - sizeY * sizeZ)->chunk);
		listInsert((first - sizeY * sizeZ)->chunk->adjacent, chunk);
	}
	if (first->low2y > 0) {
		listInsert(chunk->adjacent, (first - sizeZ)->chunk);
		listInsert((first - sizeZ)->chunk->adjacent, chunk);
	}

	// add the chunks in ZS direction
	if (z > 0) {
		world->cookie++;
		AnnotatedBlock *block;
		int x,y;
		for (x = first->low2x; x <= first->high2x; x++) {
		for (y = first->low2y; y <= first->high2y; y++) {
			block = blocks + x*sizeY*sizeZ + y*sizeZ + z - 1;
			if (block->chunk->cookie != world->cookie) {
				listInsert(chunk->adjacent, block->chunk);
				listInsert(block->chunk->adjacent, chunk);
				block->chunk->cookie = world->cookie;
			}
		}
		}
	}

	// Add chunk to the ChunkGroup if neccessary
	if (first->low2x == 0) {
		chunk->status += DIR_XS;
		listInsert(chunkGroup->chunksXS, chunk);
	}
	if (first->low2y == 0) {
		chunk->status += DIR_YS;
		listInsert(chunkGroup->chunksYS, chunk);
	}
	if (z == 0) {
		chunk->status += DIR_ZS;
		listInsert(chunkGroup->chunksZS, chunk);
	}
	if (first->high2x == sizeX - 1) {
		chunk->status += DIR_XG;
		listInsert(chunkGroup->chunksXG, chunk);
	}
	if (first->high2y == sizeY - 1) {
		chunk->status += DIR_YG;
		listInsert(chunkGroup->chunksYG, chunk);
	}
	if (z == sizeZ - 1) {
		chunk->status += DIR_ZG;
		listInsert(chunkGroup->chunksZG, chunk);
	}
}

void mergeGroup(Metachunk *world, AnnotatedBlock *blocks, Vector3i batchLow,
		int sizeX, int sizeY, int sizeZ, List *chunks, int dir) {
	Vector3i p, low, high;
	Chunk **chunk;
	AnnotatedBlock *block;

#ifdef MMB_DEBUG_CHUNK
	printf("Merging with chunk in dir %i\n", dir);
#endif

	LISTITER(chunks, chunk, Chunk**) {
		world->cookie++;
		low.x = dir == DIR_XS ? 0 : (dir == DIR_XG ? sizeX - 1 :
				(*chunk)->low.x - batchLow.x);
		low.y = dir == DIR_YS ? 0 : (dir == DIR_YG ? sizeY - 1 :
				(*chunk)->low.y - batchLow.y);
		low.z = dir == DIR_ZS ? 0 : (dir == DIR_ZG ? sizeZ - 1 :
				(*chunk)->low.z - batchLow.z);
		high.x = dir == DIR_XS ? 0 : (dir == DIR_XG ? sizeX - 1 :
				(*chunk)->high.x - batchLow.x);
		high.y = dir == DIR_YS ? 0 : (dir == DIR_YG ? sizeY - 1 :
				(*chunk)->high.y - batchLow.y);
		high.z = dir == DIR_ZS ? 0 : (dir == DIR_ZG ? sizeZ - 1 :
				(*chunk)->high.z - batchLow.z);
		for (p.x = low.x; p.x <= high.x; p.x++) {
		for (p.y = low.y; p.y <= high.y; p.y++) {
		for (p.z = low.z; p.z <= high.z; p.z++) {
			block = blocks + p.x*sizeY*sizeZ + p.y*sizeZ + p.z;
			if (block->chunk->cookie != world->cookie) {
				block->chunk->status &= ~dir;
				listInsert(block->chunk->adjacent, *chunk);
				listInsert((*chunk)->adjacent, block->chunk);
				block->chunk->cookie = world->cookie;
			}
		}
		}
		}
		(*chunk)->status &= ~DIR_OPPOSITE(dir);
	}
}

void chunkCreateBatch(Metachunk *world, Vector3i low) {
	Vector3i high = VEC3IOP(low, +, world->groupSize);
	int x,y,z;
	int sizeX = high.x - low.x;
	int sizeY = high.y - low.y;
	int sizeZ = high.z - low.z;
	int blockcount = sizeX * sizeY * sizeZ;
	AnnotatedBlock *blocks = knalloc(blockcount * sizeof(AnnotatedBlock));
	AnnotatedBlock *tmp, *tmpX, *tmpY;
	int total = 0;
	Vector3i p;
	ChunkGroup *chunkGroup = newChunkGroup(world, low);

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

#ifdef MMB_DEBUG_CHUNK
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
#endif

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
	// It is important to loop through the Z direction first, because
	// handleChunk needs the chunks in ZS direction to be generated.
	for (z = 0; z < sizeZ; z++) {
		tmp = blocks + z;
		for (x = 0; x < sizeX; x++) {
			for (y = 0; y < sizeY; y++) {
				if (tmp->low2x == x
				 && tmp->low2y == y) {
					handleChunk(world, blocks, chunkGroup,
							sizeX, sizeY, sizeZ,
							low, z, tmp);
					total++;
				}
				tmp += sizeZ;
			}
		}
	}

	// merge this ChunkGroup with adjacent ChunkGroups
	ChunkGroup **otherGroup;
	Vector3i diff;
	LISTITER(world->chunkGroups, otherGroup, ChunkGroup**) {
		if (*otherGroup == chunkGroup)
			continue;

		diff = VEC3IOP((*otherGroup)->low, -, low);

		if      (-diff.x == world->groupSize.x && !diff.y && !diff.z)
			mergeGroup(world, blocks, low, sizeX, sizeY, sizeZ,
					(*otherGroup)->chunksXG, DIR_XS);
		else if (diff.x == world->groupSize.x && !diff.y && !diff.z)
			mergeGroup(world, blocks, low, sizeX, sizeY, sizeZ,
					(*otherGroup)->chunksXS, DIR_XG);
		else if (!diff.x && -diff.y == world->groupSize.y && !diff.z)
			mergeGroup(world, blocks, low, sizeX, sizeY, sizeZ,
					(*otherGroup)->chunksYG, DIR_YS);
		else if (!diff.x && diff.y == world->groupSize.y && !diff.z)
			mergeGroup(world, blocks, low, sizeX, sizeY, sizeZ,
					(*otherGroup)->chunksYS, DIR_YG);
		else if (!diff.x && !diff.y && -diff.z == world->groupSize.z)
			mergeGroup(world, blocks, low, sizeX, sizeY, sizeZ,
					(*otherGroup)->chunksZG, DIR_ZS);
		else if (!diff.x && !diff.y && diff.z == world->groupSize.z)
			mergeGroup(world, blocks, low, sizeX, sizeY, sizeZ,
					(*otherGroup)->chunksZS, DIR_ZG);
	}

#ifdef MMB_DEBUG_CHUNK
	printf(" x  y  z | low   | hig   | lo2   | hi2   | chunk | status\n");
	printf("---------+-------+-------+-------+-------+-------+-------\n");
	tmp = blocks;
	for (x = 0; x < sizeX; x++) {
	for (y = 0; y < sizeY; y++) {
	for (z = 0; z < sizeZ; z++) {
		printf("%2i %2i %2i | %2i %2i | %2i %2i | "
				"%2i %2i | %2i %2i | %p | %i\n",
				x, y, z,
				tmp->lowx, tmp->lowy,
				tmp->highx, tmp->highy,
				tmp->low2x, tmp->low2y,
				tmp->high2x, tmp->high2y,
				tmp->chunk, tmp->chunk->status);
		tmp++;
	}
	}
	}
	printf("\n");
#endif

	printf("chunk.c: Found %i chunks in %i blocks (%i%%): ",
			total,
			blockcount,
			(int)((float)total / blockcount * 100));
	VECPRINT(low, "\n");

	return;
}

/**
 * Creates and initializes a metachunk
 * 
 * @param gen		A block generator function
 */
Metachunk *chunkInit(Block *(*gen)(Vector3i)) {
	Metachunk *world = knalloc(sizeof(Metachunk));
	world->cookie = 0;
	world->chunks = listNew(sizeof(Chunk*));
	world->chunkGroups = listNew(sizeof(ChunkGroup*));
	world->groupSize = (Vector3i){10, 10, 10};

	world->generator = gen;

	world->lastChunk = NULL;

	world->chunkToUpdate = NULL;

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
	chunkCreateBatch(world, batchPos);

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
	chunkCreateBatch(world, pos);

	world->chunkToUpdate = NULL;
}
