/**
 * @file
 */

#ifndef _MMB_CHUNKGEN_H
#define _MMB_CHUNKGEN_H

#include "vector.h"
#include "world.h"

extern void chunkgenInit(World *world);
extern void chunkgenDeinit(World *world);
extern void chunkgenCreate(World *world, Vector3i low);

#endif /* _MMB_CHUNKGEN_H */
