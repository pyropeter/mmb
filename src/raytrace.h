//! @file

#include "vector.h"
#include "chunk.h"
#include "world.h"

typedef struct Ray {
	Vector3i posi;  //!<  (posi + posf) is the initial point of the ray
	Vector3f posf;  //!<  (see posi)
	Vector3f dir;   //!<  direction of the ray
	Chunk *chunk;   //!<  the current Chunk
	World *world;   //!<  the world the ray belongs to

	int face;       //!<  the exit surface of the previous Chunk
	float factor;   //!<  (factor * dir) is the ray vector
	Vector3i first; //!<  first intersected block of current  Chunk
	Vector3i last;  //!<  last  intersected block of previous Chunk
} Ray;

extern void raytraceNext(Ray *ray);

