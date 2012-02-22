#ifndef _MMB_CHUNKSPLIT_H
#define _MMB_CHUNKSPLIT_H

#include "chunk.h"
#include "world.h"

extern void chunksplitSplit(World *world, Chunk *chunk,
		int cutDir, int cutPos);

#endif /* _MMB_CHUNKSPLIT_H */
