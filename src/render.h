#ifndef _MMB_RENDER_H
#define _MMB_RENDER_H

#include "defs.h"

typedef struct {
	double x, y, z;
	Comp ix, iy, iz;
	float dx, dy, dz;
	float ax, ay;
} Camera;

typedef struct {
	int window;
	int mouseX;
	int mouseY;
	float mouseSens;
	Camera camera;
	void (*onDraw)(void *data);
	void *onDrawData;
	int vertices;
} Render;

extern Render *renderInit(int argc, char *argv[]);
extern void renderRun();
extern void renderHookDraw(void (*func)(void *data), void *data);

#endif /* _MMB_RENDER_H */
