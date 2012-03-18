//! @file

#ifndef _MMB_BLOCK_H
#define _MMB_BLOCK_H

#include "vector.h"

typedef struct Block {
	int type;
	int solid;
	Vector2i texXS, texYS, texZS, texXG, texYG, texZG;
} Block;

extern void blockInit();
extern Block * blockGet(int type);

#define BLOCKTYPE_AIR         1
#define BLOCKTYPE_GRASS       2
#define BLOCKTYPE_DIRT        3
#define BLOCKTYPE_STONE       4
#define BLOCKTYPE_CHEST       5

#endif /* _MMB_BLOCK_H */
