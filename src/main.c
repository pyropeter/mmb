#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "GL/freeglut.h"

#include "defs.h"
#include "vector.h"
#include "render.h"
#include "generator.h"
#include "world.h"
#include "worldrender.h"

static Render *render;
static Camera *camera;
static World *world;
static Ray *ray;

void onDisplay()
{
	worldrenderDraw(world, camera);
}

void onReshape(int w, int h)
{
	worldrenderReshape(world, camera, w, h);
}

void onMouse(int button, int state, void *data)
{
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		worldSetBlock(world, ray->first, blockGet(BLOCKTYPE_AIR));
	} else if (state == GLUT_UP && button == GLUT_RIGHT_BUTTON) {
		worldSetBlock(world, ray->last, blockGet(BLOCKTYPE_CHEST));
	}
	worldrenderRefresh();
}

int main(int argc, char *argv[]) {
	printf("MMB version %s, \"%s\"\n\n", VERSION, CODENAME);

	render = renderInit(argc, argv);
	camera = &(render->camera);

	generatorInit();

	world = worldInit(generatorGetBlock);

	worldrenderInit(world, camera);
	ray = worldrenderGetRay(world, camera);
	glutDisplayFunc(&onDisplay);
	glutReshapeFunc(&onReshape);

	renderHookMouse(&onMouse, NULL);
	renderRun();
	return EXIT_SUCCESS;
}
