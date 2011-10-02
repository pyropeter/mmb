#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/freeglut.h>

#include "defs.h"
#include "render.h"
#include "generator.h"

static Render *render;
static Camera *camera;

void blockDrawDist(Block *block, Comp x_, Comp y_, Comp z_,
		int distX, int distY, int distZ) {
	if (*block == ' ')
		return;

	glPushMatrix();

	int x = (long long int)x_ - HALFCOMP;
	int y = (long long int)y_ - HALFCOMP;
	int z = (long long int)z_ - HALFCOMP;

	glBegin(GL_QUADS); {
		if (distX > 0) {
			glNormal3f(-1,0,0);
			glVertex3f(0+x,0+y,0+z);
			glVertex3f(0+x,0+y,1+z);
			glVertex3f(0+x,1+y,1+z);
			glVertex3f(0+x,1+y,0+z);
			render->vertices += 4;
		}

		if (distY > 0) {
			glNormal3f(0,-1,0);
			glVertex3f(0+x,0+y,0+z);
			glVertex3f(0+x,0+y,1+z);
			glVertex3f(1+x,0+y,1+z);
			glVertex3f(1+x,0+y,0+z);
			render->vertices += 4;
		}

		if (distZ > 0) {
			glNormal3f(0,0,-1);
			glVertex3f(0+x,0+y,0+z);
			glVertex3f(0+x,1+y,0+z);
			glVertex3f(1+x,1+y,0+z);
			glVertex3f(1+x,0+y,0+z);
			render->vertices += 4;
		}

		if (distX < 0) {
			glNormal3f(1,0,0);
			glVertex3f(1+x,0+y,0+z);
			glVertex3f(1+x,0+y,1+z);
			glVertex3f(1+x,1+y,1+z);
			glVertex3f(1+x,1+y,0+z);
			render->vertices += 4;
		}

		if (distY < 0) {
			glNormal3f(0,1,0);
			glVertex3f(0+x,1+y,0+z);
			glVertex3f(0+x,1+y,1+z);
			glVertex3f(1+x,1+y,1+z);
			glVertex3f(1+x,1+y,0+z);
			render->vertices += 4;
		}

		if (distZ < 0) {
			glNormal3f(0,0,1);
			glVertex3f(0+x,0+y,1+z);
			glVertex3f(0+x,1+y,1+z);
			glVertex3f(1+x,1+y,1+z);
			glVertex3f(1+x,0+y,1+z);
			render->vertices += 4;
		}
	}; glEnd();

	glPopMatrix();
}

void blockDraw(Block *block, Comp x, Comp y, Comp z) {
	blockDrawDist(block, x,y,z,
			(int)x - camera->ix,
			(int)y - camera->iy,
			(int)z - camera->iz);
}

void worldDraw(void *foo) {
	int r = 20;
	Comp x, y, z;
	for (x = camera->ix - r; x < camera->ix + r; x++) {
		for (y = camera->iy - r; y < camera->iy + r; y++) {
			for (z = camera->iz - r; z < camera->iz + r; z++) {
				blockDraw(generatorGetBlock(x,y,z), x,y,z);
			}
		}
	}
}

int main(int argc, char *argv[]) {
	render = renderInit(argc, argv);
	camera = &(render->camera);

	generatorInit();

	renderHookDraw((int (*)(void *))&worldDraw, NULL);
	renderRun();
	return EXIT_SUCCESS;
}
