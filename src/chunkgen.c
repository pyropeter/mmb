/**
 * @file
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "defs.h"
#include "chunkgen.h"

/**
 * Allocates and initializes memory for one Chunk
 * 
 * This also adds the Chunk to the world's list of chunks.
 */
Chunk *newChunk(World *world) {
	Chunk *chunk = knalloc(sizeof(Chunk));
	chunk->status = 0;
	chunk->cookie = -1;

	chunk->adjacent = listNew();
	chunk->blocks = NULL;

	listInsert(world->chunks, chunk);
	return chunk;
}

/**
 * Allocates and initializes memory for one ChunkGroup
 * 
 * This also adds the ChunkGroup to the world's list of ChunkGroups.
 */
ChunkGroup *newChunkGroup(World *world, Vector3i low) {
	ChunkGroup *chunkGroup = knalloc(sizeof(ChunkGroup));

	chunkGroup->low = low;
	chunkGroup->chunksXS = listNew();
	chunkGroup->chunksXG = listNew();
	chunkGroup->chunksYS = listNew();
	chunkGroup->chunksYG = listNew();
	chunkGroup->chunksZS = listNew();
	chunkGroup->chunksZG = listNew();

	listInsert(world->chunkGroups, chunkGroup);
	return chunkGroup;
}

/**
 * Adds blocks to a chunk
 * 
 * This also adds references to the chunk in the corresponding AnnotatedBlocks.
 */
void addBlocksToChunk(AnnotatedBlock *blocks, Vector3i size, int z,
		Vector2i low, Vector2i high, Chunk *chunk)
{
	AnnotatedBlock *block;
	int blocksIndex = 0;
	int x,y;

	for (x = low.x; x <= high.x; x++) {
	for (y = low.y; y <= high.y; y++) {
		block = blocks  +  x * size.y * size.z  +  y * size.z  +  z;

		assert(block->chunk == NULL);
		assert(VEC2CMP(block->high2, ==, high));
		assert(VEC2CMP(block->low2,  ==, low));

		block->chunk = chunk;
		if (chunk->blocks != NULL) {
			chunk->blocks[blocksIndex++] = block->block;
		}
	}
	}
}

/**
 * Creates a new Chunk, adds Blocks to it, and merges the Chunk with the others
 */
void handleChunk(World *world, AnnotatedBlock *blocks,
		ChunkGroup *chunkGroup, Vector3i size,
		Vector3i low, int z, AnnotatedBlock *first)
{
	Chunk *chunk = newChunk(world);

	// set low and high points of the new chunk
	chunk->low.x = low.x + first->low2.x;
	chunk->low.y = low.y + first->low2.y;
	chunk->low.z = low.z + z;
	chunk->high.x = low.x + first->high2.x;
	chunk->high.y = low.y + first->high2.y;
	chunk->high.z = low.z + z;

	// transparent or not?
	// allocate memory for block pointers
	if (*(first->block) != ' ') {
		int chunkSizeX = first->high2.x - first->low2.x + 1;
		int chunkSizeY = first->high2.y - first->low2.y + 1;
		chunk->blocks = knalloc(sizeof(Block*) * 
				chunkSizeX * chunkSizeY * 1);
	}

	// add blocks to the chunk
	addBlocksToChunk(blocks, size, z, first->low2, first->high2, chunk);

	// ===== Handle adjacent chunks =====
	// Only the chunks in XS, YS and ZS direction are relevant, the others
	// are not yet generated.
	// There is always only one adjacent chunk in X and one in Y direction
	// (this is one property of the chunk generating algorithm).
	// In the Z direction the number of adjacent chunks is between one and
	// the number of blocks in the current chunk.

	// add the chunks in XS and YS direction
	if (first->low2.x > 0) {
		listInsert(chunk->adjacent, (first - size.y * size.z)->chunk);
		listInsert((first - size.y * size.z)->chunk->adjacent, chunk);
	}
	if (first->low2.y > 0) {
		listInsert(chunk->adjacent, (first - size.z)->chunk);
		listInsert((first - size.z)->chunk->adjacent, chunk);
	}

	// add the chunks in ZS direction
	if (z > 0) {
		world->cookie++;
		AnnotatedBlock *block;
		int x, y;
		for (x = first->low2.x; x <= first->high2.x; x++) {
		for (y = first->low2.y; y <= first->high2.y; y++) {
			block = blocks  +  x * size.y * size.z
					+  y * size.z  +  z  -  1;
			if (block->chunk->cookie != world->cookie) {
				listInsert(chunk->adjacent, block->chunk);
				listInsert(block->chunk->adjacent, chunk);
				block->chunk->cookie = world->cookie;
			}
		}
		}
	}

	// Add chunk to the ChunkGroup if it touches the ChunkGroup's borders
	if (first->low2.x == 0) {
		chunk->status += DIR_XS;
		listInsert(chunkGroup->chunksXS, chunk);
	}
	if (first->low2.y == 0) {
		chunk->status += DIR_YS;
		listInsert(chunkGroup->chunksYS, chunk);
	}
	if (z == 0) {
		chunk->status += DIR_ZS;
		listInsert(chunkGroup->chunksZS, chunk);
	}
	if (first->high2.x == size.x - 1) {
		chunk->status += DIR_XG;
		listInsert(chunkGroup->chunksXG, chunk);
	}
	if (first->high2.y == size.y - 1) {
		chunk->status += DIR_YG;
		listInsert(chunkGroup->chunksYG, chunk);
	}
	if (z == size.z - 1) {
		chunk->status += DIR_ZG;
		listInsert(chunkGroup->chunksZG, chunk);
	}
}

void mergeGroup(World *world, AnnotatedBlock *blocks, Vector3i batchLow,
		Vector3i size, List *chunks, int dir) {
	Vector3i p, low, high;
	Chunk **chunk;
	AnnotatedBlock *block;

#ifdef MMB_DEBUG_CHUNKGEN
	printf("Merging with chunk in dir %i\n", dir);
#endif

	LISTITER(chunks, chunk, Chunk**) {
		world->cookie++;
		low.x = dir == DIR_XS ? 0 : (dir == DIR_XG ? size.x - 1 :
				(*chunk)->low.x - batchLow.x);
		low.y = dir == DIR_YS ? 0 : (dir == DIR_YG ? size.y - 1 :
				(*chunk)->low.y - batchLow.y);
		low.z = dir == DIR_ZS ? 0 : (dir == DIR_ZG ? size.z - 1 :
				(*chunk)->low.z - batchLow.z);
		high.x = dir == DIR_XS ? 0 : (dir == DIR_XG ? size.x - 1 :
				(*chunk)->high.x - batchLow.x);
		high.y = dir == DIR_YS ? 0 : (dir == DIR_YG ? size.y - 1 :
				(*chunk)->high.y - batchLow.y);
		high.z = dir == DIR_ZS ? 0 : (dir == DIR_ZG ? size.z - 1 :
				(*chunk)->high.z - batchLow.z);
		for (p.x = low.x; p.x <= high.x; p.x++) {
		for (p.y = low.y; p.y <= high.y; p.y++) {
		for (p.z = low.z; p.z <= high.z; p.z++) {
			block = blocks  +  p.x * size.y * size.z
					+  p.y * size.z  +  p.z;
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

void setDistances(AnnotatedBlock *blocks, Vector3i size) {
	AnnotatedBlock *tmp, *tmpX, *tmpY;
	int x, y, z;

	// set high
	tmp = blocks + size.x * size.y * size.z - 1;
	tmpX = tmp + size.z * size.y;
	tmpY = tmp + size.z;
	for (x = size.x - 1; x >= 0; x--) {
	for (y = size.y - 1; y >= 0; y--) {
	for (z = size.z - 1; z >= 0; z--) {
		if (x < size.x - 1) {
			// compare with next block in XG-dir.
			if (*(tmp->block) == *(tmpX->block)) {
				tmp->high.x = tmpX->high.x;
			} else {
				tmp->high.x = x;
			}
		}
		if (y < size.y - 1) {
			// compare with next block in YG-dir.
			if (*(tmp->block) == *(tmpY->block)) {
				tmp->high.y = tmpY->high.y;
			} else {
				tmp->high.y = y;
			}
		}
		tmp--;
		tmpX--; tmpY--;
	}
	}
	}

	// set low and low2
	tmp = blocks;
	tmpX = tmp - size.z * size.y;
	tmpY = tmp - size.z;
	for (x = 0; x < size.x; x++) {
	for (y = 0; y < size.y; y++) {
	for (z = 0; z < size.z; z++) {
		if (x > 0) {
			// compare with next block in XS-dir.
			if (*(tmp->block) == *(tmpX->block)) {
				tmp->low.x = tmpX->low.x;
			} else {
				tmp->low.x = x;
			}
		}
		if (y > 0) {
			// compare with neyt block in YS-dir.
			if (*(tmp->block) == *(tmpY->block)) {
				tmp->low.y = tmpY->low.y;
			} else {
				tmp->low.y = y;
			}
		}
		if (x > 0) {
			// compare with next block in XS-dir.
			if (tmp->low.x == tmpX->low.x
			 && tmp->low.y == tmpX->low.y
			 && tmp->high.x == tmpX->high.x
			 && tmp->high.y == tmpX->high.y) {
				tmp->low2.x = tmpX->low2.x;
			} else {
				tmp->low2.x = x;
			}
		}
		if (y > 0) {
			// compare with neyt block in YS-dir.
			if (tmp->low.x == tmpY->low.x
			 && tmp->low.y == tmpY->low.y
			 && tmp->high.x == tmpY->high.x
			 && tmp->high.y == tmpY->high.y) {
				tmp->low2.y = tmpY->low2.y;
			} else {
				tmp->low2.y = y;
			}
		}
		tmp++;
		tmpX++; tmpY++;
	}
	}
	}

	// set high2
	tmp = blocks + size.x * size.y * size.z - 1;
	tmpX = tmp + size.z * size.y;
	tmpY = tmp + size.z;
	for (x = size.x - 1; x >= 0; x--) {
	for (y = size.y - 1; y >= 0; y--) {
	for (z = size.z - 1; z >= 0; z--) {
		if (x < size.x - 1) {
			// compare with next block in XS-dir.
			if (tmp->low.x == tmpX->low.x
			 && tmp->low.y == tmpX->low.y
			 && tmp->high.x == tmpX->high.x
			 && tmp->high.y == tmpX->high.y) {
				tmp->high2.x = tmpX->high2.x;
			} else {
				tmp->high2.x = x;
			}
		}
		if (y < size.y - 1) {
			// compare with neyt block in YS-dir.
			if (tmp->low.x == tmpY->low.x
			 && tmp->low.y == tmpY->low.y
			 && tmp->high.x == tmpY->high.x
			 && tmp->high.y == tmpY->high.y) {
				tmp->high2.y = tmpY->high2.y;
			} else {
				tmp->high2.y = y;
			}
		}
		tmp--;
		tmpX--; tmpY--;
	}
	}
	}
}

/**
 * Creates all Chunks in the cuboid defined by low and world->groupSize
 */
void chunkgenCreate(World *world, Vector3i low) {
	Vector3i size = world->groupSize;
	Vector3i high = VEC3IOP(low, +, size);
	int blockcount = size.x * size.y * size.z;
	int x,y,z;
	AnnotatedBlock *blocks = world->annotatedBlocks;
	AnnotatedBlock *tmp;
	int total = 0;
	Vector3i p;
	ChunkGroup *chunkGroup = newChunkGroup(world, low);

	// load blocks from map generator
	tmp = blocks;
	for (p.x = low.x; p.x < high.x; p.x++) {
	for (p.y = low.y; p.y < high.y; p.y++) {
	for (p.z = low.z; p.z < high.z; p.z++) {
		tmp->chunk = NULL;
		tmp->block = world->generator(p);
		tmp++;
	}
	}
	}

#ifdef MMB_DEBUG_CHUNKGEN
	// dump blocks for debugging
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

	setDistances(world->annotatedBlocks, world->groupSize);

	// search for chunks
	// It is important to loop through the Z direction first, because
	// handleChunk needs the chunks in ZS direction to be generated.
	for (z = 0; z < size.z; z++) {
		tmp = blocks + z;
		for (x = 0; x < size.x; x++) {
			for (y = 0; y < size.y; y++) {
				if (tmp->low2.x == x
				 && tmp->low2.y == y) {
					handleChunk(world, blocks, chunkGroup,
							size, low, z, tmp);
					total++;
				}
				tmp += size.z;
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
			mergeGroup(world, blocks, low, size,
					(*otherGroup)->chunksXG, DIR_XS);
		else if (diff.x == world->groupSize.x && !diff.y && !diff.z)
			mergeGroup(world, blocks, low, size,
					(*otherGroup)->chunksXS, DIR_XG);
		else if (!diff.x && -diff.y == world->groupSize.y && !diff.z)
			mergeGroup(world, blocks, low, size,
					(*otherGroup)->chunksYG, DIR_YS);
		else if (!diff.x && diff.y == world->groupSize.y && !diff.z)
			mergeGroup(world, blocks, low, size,
					(*otherGroup)->chunksYS, DIR_YG);
		else if (!diff.x && !diff.y && -diff.z == world->groupSize.z)
			mergeGroup(world, blocks, low, size,
					(*otherGroup)->chunksZG, DIR_ZS);
		else if (!diff.x && !diff.y && diff.z == world->groupSize.z)
			mergeGroup(world, blocks, low, size,
					(*otherGroup)->chunksZS, DIR_ZG);
	}

#ifdef MMB_DEBUG_CHUNKGEN
	printf(" x  y  z | low   | hig   | lo2   | hi2   | chunk | status\n");
	printf("---------+-------+-------+-------+-------+-------+-------\n");
	tmp = blocks;
	for (x = 0; x < size.x; x++) {
	for (y = 0; y < size.y; y++) {
	for (z = 0; z < size.z; z++) {
		printf("%2i %2i %2i | %2i %2i | %2i %2i | "
				"%2i %2i | %2i %2i | %p | %i\n",
				x, y, z,
				tmp->low.x, tmp->low.y,
				tmp->high.x, tmp->high.y,
				tmp->low2.x, tmp->low2.y,
				tmp->high2.x, tmp->high2.y,
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
 * Initializes the World members needed by chunkgen.c
 */
void chunkgenInit(World *world) {
	world->annotatedBlocks = knalloc(
			world->groupSize.x * world->groupSize.y
			* world->groupSize.z * sizeof(AnnotatedBlock));

	Vector2i p;
	int z;
	AnnotatedBlock *tmp = world->annotatedBlocks;
	for (p.x = 0; p.x < world->groupSize.x; p.x++) {
	for (p.y = 0; p.y < world->groupSize.y; p.y++) {
	for (z = 0; z < world->groupSize.z; z++) {
		tmp->low = tmp->high = tmp->low2 = tmp->high2 = p;
		tmp++;
	}
	}
	}
}

/**
 * Frees the World members needed by chunkgen.c
 */
void chunkgenDeinit(World *world) {
	free(world->annotatedBlocks);
}
