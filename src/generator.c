//! @file

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "defs.h"
#include "vector.h"
#include "generator.h"
#include "simplex.h"

SimplexState *state;

void generatorInit() {
	state = simplexInit(time(NULL));
	return;
}

Block *generatorGetBlock(Vector3i pos) {
	int height = simplex2D(state, (double)pos.x/100.0, (double)pos.z/100.0) * 10;

	if (height < pos.y)
		return blockGet(BLOCKTYPE_AIR);
	else if (height == pos.y)
		return blockGet(BLOCKTYPE_GRASS);
	else if (height < pos.y + 2)
		return blockGet(BLOCKTYPE_DIRT);
	else
		return blockGet(BLOCKTYPE_STONE);
}
