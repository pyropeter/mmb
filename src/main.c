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
static int frameNumber;

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

void findChunks(Chunk *startChunk, List *chunkList) {
	// out of range?
	if (!isVisible(startChunk))
		return;

	chunkUpdate(metachunk, startChunk);

	Chunk *chunk;
	int i;
	for (i = 0; i < startChunk->neighborCount; i++) {
		chunk = startChunk->neighbors[i];

		if (chunk->lastRender != frameNumber) {
			chunk->lastRender = frameNumber;
			listInsert(chunkList, chunk);

			if (chunk->blocks == NULL) // transparent block
				findChunks(chunk, chunkList);
		}
	}
}

void drawChunk(Chunk *chunk) {
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

void worldDrawChunked(void *foo) {
	if (++frameNumber == 0)
		frameNumber++;

	List *chunkList = listNew();

	Chunk *startChunk = chunkGet(metachunk, (Point){camera->ix,
			camera->iy, camera->iz});
	startChunk->lastRender = frameNumber;
	listInsert(chunkList, startChunk);

	findChunks(startChunk, chunkList);

	Chunk **chunk = (Chunk**)chunkList->mem;
	for (; (void**)chunk != chunkList->nextFree; chunk++) {
		drawChunk(*chunk);
	}

	listFree(chunkList);
}

int main(int argc, char *argv[]) {
	render = renderInit(argc, argv);
	camera = &(render->camera);

	generatorInit();
	
	metachunk = chunkInit(generatorGetBlock, (Point){
			camera->ix, camera->iy, camera->iz});

	renderHookDraw(&worldDrawChunked, NULL);
	renderRun();
	return EXIT_SUCCESS;
}
