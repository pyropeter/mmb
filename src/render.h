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
	void (*onMouse)(int button, int state, void *data);
	void *onMouseData;
	int vertices;
} Render;

extern Render *renderInit(int argc, char *argv[]);
extern void renderRun();
extern void renderDebug();
extern void renderHookMouse(void (*func)(int button, int state, void *data),
		void *data);

#endif /* _MMB_RENDER_H */
