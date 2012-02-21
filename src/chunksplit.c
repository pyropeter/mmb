//! @file

#include "defs.h"
#include "chunksplit.h"

void moveBlocks(Chunk *old, Chunk *new) {
	if (old->blocks == NULL) {
		new->blocks = NULL;
		return;
	}

	Vector3i size = (Vector3i){
		new->high.x - new->low.x + 1,
		new->high.y - new->low.y + 1,
		new->high.z - new->low.z + 1};
	int numBlocks = size.x * size.y * size.z;
	new->blocks = malloc(numBlocks * sizeof Block*);

	// FIXME: can't be tested before the generator uses more
	// than one block type; write proper code then
	int i;
	for (i=0; i<numBlocks; i++) {
		new->blocks[i] = old->blocks[0];
	}
}

void updateWorldChunks(World *world, Chunk *chunk, Chunk *one, Chunk *two) {
	// replace the pointer to the old chunk with a pointer to the first
	// new chunk:
	Chunk **oldPos = listSearch(world->chunks, chunk);
	assert(oldPos != NULL);
	*oldPos = one;

	// append a pointer to the second new chunk:
	listInsert(world->chunks, two);
}

// world->chunksToUpdate is not updated, keep that in mind.
void chunkSplit(World *world, Chunk *chunk, int cutDir, int cutPos) {
	Chunk *one = knalloc(sizeof Chunk);
	Chunk *two = knalloc(sizeof Chunk);

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

	// move chunk->blocks to one/two->blocks
	moveBlocks(chunk, one);
	moveBlocks(chunk, two);

	// for each adj as chunk->adjacent: update adj->adjacent
	// in the same step: create one/two->adjacent


	// update world->chunks
	updateWorldChunks(world, chunk, one, two);

	// update world->lastChunk and world->lastPos

	// update world->chunkGroups

	return;
}

