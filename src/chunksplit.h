#ifndef _MMB_CHUNKSPLIT_H
#define _MMB_CHUNKSPLIT_H

#include "vector.h"
#include "world.h"

extern void chunksplitSplitOnce(World *world, Chunk *chunk,
		int cutDir, int cutPos);
extern Chunk * chunksplitSplit(World *world, Vector3i pos);

#endif /* _MMB_CHUNKSPLIT_H */
