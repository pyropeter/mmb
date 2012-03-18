//! @file

#ifndef _MMB_VECTOR_H
#define _MMB_VECTOR_H

#include <math.h>

//! 4D vector with int members x,y,z,w
typedef struct Vector4i {
	int x, y, z, w;
} Vector4i;

//! 4D vector with float members x,y,z,w
typedef struct Vector4f {
	float x, y, z, w;
} Vector4f;

//! 3D vector with int members x,y,z
typedef struct Vector3i {
	int x, y, z;
} Vector3i;

//! 3D vector with float members x,y,z
typedef struct Vector3f {
	float x, y, z;
} Vector3f;

//! 2D vector with int members x,y
typedef struct Vector2i {
	int x, y;
} Vector2i;

//! 2D vector with float members x,y
typedef struct Vector2f {
	float x, y;
} Vector2f;

#define VEC3CMP(a,o,b) (a.x o b.x && a.y o b.y && a.z o b.z)
#define VEC2CMP(a,o,b) (a.x o b.x && a.y o b.y)

#define VEC3IOP(a,o,b) ((Vector3i){a.x o b.x, a.y o b.y, a.z o b.z})
#define VEC3FOP(a,o,b) ((Vector3f){a.x o b.x, a.y o b.y, a.z o b.z})
#define VEC2IOP(a,o,b) ((Vector2i){a.x o b.x, a.y o b.y})

#define VECPRINT(v, tail) printf("%i/%i/%i%s", v.x,v.y,v.z, tail);
#define VECFPRINT(v, tail) printf("%1.1f/%1.1f/%1.1f%s", v.x,v.y,v.z, tail);

#endif /* _MMB_VECTOR_H */
