#ifndef _MMB_RENDER_H
#define _MMB_RENDER_H

#include "defs.h"
#include "vector.h"

typedef struct {
	double x, y, z;
	Vector3i pos;

	float dx, dy, dz;
	float ax, ay;

	int range;
	Vector3i low, high;
} Camera;

typedef struct {
	int window;
	int mouseX;
	int mouseY;
	float mouseSens;
	Camera camera;
	void (*onDraw)(void *data);
	void *onDrawData;
	void (*onKey)(void *data, unsigned char key);
	void *onKeyData;
	int vertices;
} Render;

extern Render *renderInit(int argc, char *argv[]);
extern void renderRun();
extern void renderHookDraw(void (*func)(void *data), void *data);
extern void renderHookKey(void (*func)(void *data, unsigned char key),
		void *data);

#endif /* _MMB_RENDER_H */
