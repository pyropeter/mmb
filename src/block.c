//! @file

#include <stdlib.h>
#include <stdio.h>

#include "block.h"

static Block blocks[] = {
	{BLOCKTYPE_AIR,   0},
	{BLOCKTYPE_GRASS, 1, {3, 0}, {2, 0}, {3, 0}, {3, 0}, {0, 0}, {3, 0}},
	{BLOCKTYPE_DIRT,  1, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}},
	{BLOCKTYPE_STONE, 1, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}},
	{BLOCKTYPE_CHEST, 1, {11, 1}, {9, 1}, {10, 1},
				{10, 1}, {9, 1}, {10, 1}},
	{0}
};

Block * blockGet(int type)
{
	Block *block = blocks;
	while (block->type) {
		if (block->type == type)
			return block;
		block++;
	}

	return NULL;
}

void blockInit()
{
	return;
}

