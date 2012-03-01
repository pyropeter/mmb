#ifndef _MMB_WORLDRENDER_H
#define _MMB_WORLDRENDER_H

#include "render.h"
#include "world.h"
#include "raytrace.h"

void worldrenderInit(World *world, Camera *camera);
void worldrenderDraw(World *world, Camera *camera);
void worldrenderReshape(World *world, Camera *camera, int w, int h);
Ray * worldrenderGetRay(World *world, Camera *camera);
void worldrenderRefresh();

#endif /* _MMB_WORLDRENDER_H */

