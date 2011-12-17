/**
 * @file
 */

#ifndef _MMB_CHUNKGEN_H
#define _MMB_CHUNKGEN_H

#include "vector.h"
#include "chunk.h"

extern void chunkgenInit(Metachunk *world);
extern void chunkgenDeinit(Metachunk *world);
extern void chunkgenCreate(Metachunk *world, Vector3i low);

#endif /* _MMB_CHUNKGEN_H */
