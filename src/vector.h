/**
 * @file
 */

#ifndef _MMB_VECTOR_H
#define _MMB_VECTOR_H

#include <math.h>

/**
 * 3D vector with int members x,y,z
 */
typedef struct Vector3i {
	int x, y, z;
} Vector3i;

/**
 * 2D vector with int members x,y
 */
typedef struct Vector2i {
	int x, y;
} Vector2i;

#define VEC3CMP(a,o,b) (a.x o b.x && a.y o b.y && a.z o b.z)
#define VEC2CMP(a,o,b) (a.x o b.x && a.y o b.y)

#define VEC3IOP(a,o,b) ((Vector3i){a.x o b.x, a.y o b.y, a.z o b.z})
#define VEC2IOP(a,o,b) ((Vector2i){a.x o b.x, a.y o b.y})

#define VECPRINT(v, tail) printf("%i/%i/%i%s", v.x,v.y,v.z, tail);

#endif /* _MMB_VECTOR_H */
