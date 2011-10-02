
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
	int (*onDraw)(void *data);
	void *onDrawData;
	int vertices;
} Render;

extern Render *renderInit(int argc, char *argv[]);
extern void renderRun();
extern void renderHookDraw(int (*func)(void *data), void *data);
