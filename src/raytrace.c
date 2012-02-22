//! @file

#include "stdio.h"

#include "vector.h"
#include "chunk.h"
#include "world.h"
#include "raytrace.h"

void getFace(Ray *r)
{
	float diff, factor;

	float oldFactor = -10000;
	r->factor = 10000000;
	r->face = DIR_XS;

	if (r->dir.x > 0) {
		diff = (r->chunk->high.x + 1 - r->posi.x) - r->posf.x;
		factor = diff / r->dir.x;
		if (factor > oldFactor && factor < r->factor) {
			r->factor = factor;
			r->face = DIR_XG;
		}
	} else {
		diff = (r->chunk->low.x - r->posi.x) - r->posf.x;
		factor = diff / r->dir.x;
		if (factor > oldFactor && factor < r->factor) {
			r->factor = factor;
			r->face = DIR_XS;
		}
	}

	if (r->dir.y > 0) {
		diff = (r->chunk->high.y + 1 - r->posi.y) - r->posf.y;
		factor = diff / r->dir.y;
		if (factor > oldFactor && factor < r->factor) {
			r->factor = factor;
			r->face = DIR_YG;
		}
	} else {
		diff = (r->chunk->low.y - r->posi.y) - r->posf.y;
		factor = diff / r->dir.y;
		if (factor > oldFactor && factor < r->factor) {
			r->factor = factor;
			r->face = DIR_YS;
		}
	}

	if (r->dir.z > 0) {
		diff = (r->chunk->high.z + 1 - r->posi.z) - r->posf.z;
		factor = diff / r->dir.z;
		if (factor > oldFactor && factor < r->factor) {
			r->factor = factor;
			r->face = DIR_ZG;
		}
	} else {
		diff = (r->chunk->low.z - r->posi.z) - r->posf.z;
		factor = diff / r->dir.z;
		if (factor > oldFactor && factor < r->factor) {
			r->factor = factor;
			r->face = DIR_ZS;
		}
	}
}

void raytraceNext(Ray *r)
{
	getFace(r);

	r->first.x = r->posi.x + floorf(r->posf.x + r->dir.x * r->factor);
	r->first.y = r->posi.y + floorf(r->posf.y + r->dir.y * r->factor);
	r->first.z = r->posi.z + floorf(r->posf.z + r->dir.z * r->factor);
	if (r->first.x < r->chunk->low.x)  r->first.x = r->chunk->low.x;
	if (r->first.x > r->chunk->high.x) r->first.x = r->chunk->high.x;
	if (r->first.y < r->chunk->low.y)  r->first.y = r->chunk->low.y;
	if (r->first.y > r->chunk->high.y) r->first.y = r->chunk->high.y;
	if (r->first.z < r->chunk->low.z)  r->first.z = r->chunk->low.z;
	if (r->first.z > r->chunk->high.z) r->first.z = r->chunk->high.z;
	r->last = r->first;

	switch (r->face) {
		case DIR_XG:
			r->last.x  = r->chunk->high.x;
			r->first.x = r->chunk->high.x + 1;
			break;
		case DIR_XS:
			r->last.x  = r->chunk->low.x;
			r->first.x = r->chunk->low.x - 1;
			break;
		case DIR_YG:
			r->last.y  = r->chunk->high.y;
			r->first.y = r->chunk->high.y + 1;
			break;
		case DIR_YS:
			r->last.y  = r->chunk->low.y;
			r->first.y = r->chunk->low.y - 1;
			break;
		case DIR_ZG:
			r->last.z  = r->chunk->high.z;
			r->first.z = r->chunk->high.z + 1;
			break;
		case DIR_ZS:
			r->last.z  = r->chunk->low.z;
			r->first.z = r->chunk->low.z - 1;
			break;
	}

	r->chunk = worldGetChunk(r->world, r->first);
}

