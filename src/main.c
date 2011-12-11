#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/freeglut.h>

#include "defs.h"
#include "render.h"
#include "generator.h"
#include "chunk.h"

static Render *render;
static Camera *camera;
static Metachunk *metachunk;

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

void drawChunk(Chunk *chunk) {
/*	// draw chunk's bounding rectangle as wireframe

	int x1 = (long long int)chunk->low.x - HALFCOMP;
	int y1 = (long long int)chunk->low.y - HALFCOMP;
	int z1 = (long long int)chunk->low.z - HALFCOMP;
	int x2 = (long long int)chunk->high.x - HALFCOMP + 1;
	int y2 = (long long int)chunk->high.y - HALFCOMP + 1;
	int z2 = (long long int)chunk->high.z - HALFCOMP + 1;

	glDisable(GL_LIGHTING);
	glBegin(GL_LINES); {
		glVertex3f(x1, y1, z1);
		glVertex3f(x1, y1, z2);
		glVertex3f(x1, y2, z1);
		glVertex3f(x1, y2, z2);
		glVertex3f(x2, y1, z1);
		glVertex3f(x2, y1, z2);
		glVertex3f(x2, y2, z1);
		glVertex3f(x2, y2, z2);

		glVertex3f(x1, y1, z1);
		glVertex3f(x1, y2, z1);
		glVertex3f(x1, y1, z2);
		glVertex3f(x1, y2, z2);
		glVertex3f(x2, y1, z1);
		glVertex3f(x2, y2, z1);
		glVertex3f(x2, y1, z2);
		glVertex3f(x2, y2, z2);

		glVertex3f(x1, y1, z1);
		glVertex3f(x2, y1, z1);
		glVertex3f(x1, y1, z2);
		glVertex3f(x2, y1, z2);
		glVertex3f(x1, y2, z1);
		glVertex3f(x2, y2, z1);
		glVertex3f(x1, y2, z2);
		glVertex3f(x2, y2, z2);
	}; glEnd();
	glEnable(GL_LIGHTING);
*/
	if (chunk->blocks == NULL)
		return;

	Block **block = chunk->blocks;

	Comp x, y, z;
	int dx, dy, dz;

	dx = (int)chunk->low.x - camera->ix;
	for (x = chunk->low.x; x <= chunk->high.x; x++) {
		dy = (int)chunk->low.y - camera->iy;
		for (y = chunk->low.y; y <= chunk->high.y; y++) {
			dz = (int)chunk->low.z - camera->iz;
			for (z = chunk->low.z; z <= chunk->high.z; z++) {
				blockDrawDist(*block, x,y,z, dx,dy,dz);
				block++;
				dz++;
			}
			dy++;
		}
		dx++;
	}
}

int isVisible(Chunk *chunk) {
	if(chunk->low.x > camera->high.x
	|| chunk->low.y > camera->high.y
	|| chunk->low.z > camera->high.z
	|| chunk->high.x < camera->low.x
	|| chunk->high.y < camera->low.y
	|| chunk->high.z < camera->low.z)
		return 0;
	return 1;
}

void findChunks(Chunk *startChunk) {
	chunkUpdate(metachunk, startChunk);

	Chunk **chunk = (Chunk**)startChunk->adjacent->mem;
	for (; (void**)chunk != startChunk->adjacent->nextFree; chunk++) {
		if ((*chunk)->cookie != metachunk->cookie) {
			(*chunk)->cookie = metachunk->cookie;

			// out of range?
			if (!isVisible(*chunk))
				continue;

			drawChunk(*chunk);

			if ((*chunk)->blocks == NULL) // transparent block
				findChunks(*chunk);
		}
	}
}

void worldDrawChunked(void *foo) {
	metachunk->cookie++;

	Chunk *startChunk = chunkGet(metachunk, (Point){camera->ix,
			camera->iy, camera->iz});
	startChunk->cookie = metachunk->cookie;

	findChunks(startChunk);
	chunkAfterFrame(metachunk);
	fflush(stdout);
}

int main(int argc, char *argv[]) {
	render = renderInit(argc, argv);
	camera = &(render->camera);

	generatorInit();
	
	metachunk = chunkInit(generatorGetBlock);

	renderHookDraw(&worldDrawChunked, NULL);
	renderRun();
	return EXIT_SUCCESS;
}
