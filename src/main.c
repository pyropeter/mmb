#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/freeglut.h>

#include "defs.h"
#include "vector.h"
#include "render.h"
#include "generator.h"
#include "chunk.h"
#include "world.h"

static Render *render;
static Camera *camera;
static World *world;

void blockDrawDist(Block *block, int x, int y, int z,
		int distX, int distY, int distZ) {
	if (*block == ' ')
		return;

	glPushMatrix();

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

//! draw block's bounding rectangle as wireframe
void drawBlockBorder(Vector3i pos) {
	int x1 = pos.x;
	int y1 = pos.y;
	int z1 = pos.z;
	int x2 = pos.x + 1;
	int y2 = pos.y + 1;
	int z2 = pos.z + 1;

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glScalef(1.2, 1.2, 1.2);
	glTranslatef(-0.1, -0.1, -0.1);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES); {
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 1);
		glVertex3f(0, 1, 0);
		glVertex3f(0, 1, 1);
		glVertex3f(1, 0, 0);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 1, 0);
		glVertex3f(1, 1, 1);

		glVertex3f(0, 0, 0);
		glVertex3f(0, 1, 0);
		glVertex3f(0, 0, 1);
		glVertex3f(0, 1, 1);
		glVertex3f(1, 0, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 1, 1);

		glVertex3f(0, 0, 0);
		glVertex3f(1, 0, 0);
		glVertex3f(0, 0, 1);
		glVertex3f(1, 0, 1);
		glVertex3f(0, 1, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(0, 1, 1);
		glVertex3f(1, 1, 1);
	}; glEnd();
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void drawChunk(Chunk *chunk) {
	if (chunk->blocks == NULL)
		return;

	Block **block = chunk->blocks;

	int x, y, z;
	int dx, dy, dz;

	dx = chunk->low.x - camera->pos.x;
	for (x = chunk->low.x; x <= chunk->high.x; x++) {
		dy = chunk->low.y - camera->pos.y;
		for (y = chunk->low.y; y <= chunk->high.y; y++) {
			dz = chunk->low.z - camera->pos.z;
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
	worldUpdateChunk(world, startChunk);

	Chunk **chunk = (Chunk**)startChunk->adjacent->mem;
	for (; (void**)chunk != startChunk->adjacent->nextFree; chunk++) {
		if ((*chunk)->cookie != world->cookie) {
			(*chunk)->cookie = world->cookie;

			// out of range?
			if (!isVisible(*chunk))
				continue;

			drawChunk(*chunk);

			if ((*chunk)->blocks == NULL) // transparent block
				findChunks(*chunk);
		}
	}
}

void hilightSelection() {
	int i;
	Vector3f pos, dir;
	Vector3i posi, lastposi;
	Chunk *chunk, *prevChunk;

	// we will abuse chunkGet(), so backup it's state
	Chunk *lastChunk = world->lastChunk;
	Vector3i lastPos = world->lastPos;

	dir = (Vector3f){camera->dx, camera->dy, camera->dz};
	pos = (Vector3f){camera->x, camera->y, camera->z};
	prevChunk = worldGetChunk(world, camera->pos);

	// just assume dir is always of length 1
	for (i = 0; i < 20; i++) {
		pos = VEC3FOP(pos, +, dir);
		posi = (Vector3i){floor(pos.x), floor(pos.y), floor(pos.z)};
		chunk = worldGetChunk(world, posi);

		if (chunk != prevChunk) {
			if (chunk->blocks) {
				// found a solid chunk
				drawBlockBorder(posi);
				break;
			}
			prevChunk = chunk;
		}
	}

	// restore chunkGet()'s state
	world->lastChunk = lastChunk;
	world->lastPos = lastPos;
}

void worldDrawChunked(void *foo) {
	world->cookie++;

#ifdef MMB_DEBUG_MAIN
	long timer = startTimer();
#endif

	Chunk *startChunk = worldGetChunk(world, camera->pos);
	startChunk->cookie = world->cookie;
	findChunks(startChunk);
	hilightSelection();

	worldAfterFrame(world);

#ifdef MMB_DEBUG_MAIN
	printf("frame done, time: %i s^-6\n", stopTimer(timer));
#endif

	fflush(stdout);
}

int main(int argc, char *argv[]) {
	printf("MMB version %s, \"White Cubes\"\n\n", VERSION);

	render = renderInit(argc, argv);
	camera = &(render->camera);

	generatorInit();

	world = worldInit(generatorGetBlock);

	renderHookDraw(&worldDrawChunked, NULL);
	renderRun();
	return EXIT_SUCCESS;
}
