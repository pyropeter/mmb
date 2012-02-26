#ifndef _MMB_GENERATOR_H
#define _MMB_GENERATOR_H

#include "defs.h"
#include "vector.h"
#include "block.h"

extern void generatorInit();
extern Block *generatorGetBlock(Vector3i pos);

#endif /* _MMB_GENERATOR_H */
