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

int searchChunkList(Chunk *chunkList[], int len, Chunk *chunk) {
	int i;
	for (i = 0; i < len; i++) {
		if (chunkList[i] == chunk)
			return i;
	}
	return -1;
}

#define CHUNKLISTLEN 42
void findChunks(Chunk *startChunk, Chunk *chunkList[], int *chunkCount) {
	if (startChunk->blocks != NULL)
		return; // Solid Chunk

	chunkUpdate(startChunk);

	Chunk *chunk;
	int i;
	for (i = 0; i < startChunk->neighborCount; i++) {
		chunk = startChunk->neighbors[i];

		if (searchChunkList(chunkList, *chunkCount, chunk) == -1) {
			chunkList[*chunkCount] = chunk;
			(*chunkCount)++;
			if (*chunkCount >= CHUNKLISTLEN)
				return;

			findChunks(chunk, chunkList, chunkCount);
		}
	}
}

void drawChunk(Chunk *chunk) {
	if (chunk->blocks == NULL)
		return;

	Block **block = chunk->blocks;

	Comp x, y, z;
	int dx, dy, dz;

	dx = (int)chunk->lx - camera->ix;
	for (x = chunk->lx; x <= chunk->hx; x++) {
		dy = (int)chunk->ly - camera->iy;
		for (y = chunk->ly; y <= chunk->hy; y++) {
			dz = (int)chunk->lz - camera->iz;
			for (z = chunk->lz; z <= chunk->hz; z++) {
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
	Chunk *chunkList[CHUNKLISTLEN];
	int chunkCount = 0;

	Chunk *startChunk = chunkGet(camera->ix, camera->iy, camera->iz);
	chunkList[chunkCount] = startChunk;
	chunkCount++;

	findChunks(startChunk, chunkList, &chunkCount);

	int i;
	for (i = 0; i < chunkCount; i++) {
		drawChunk(chunkList[i]);
	}
}

int main(int argc, char *argv[]) {
	render = renderInit(argc, argv);
	camera = &(render->camera);

	generatorInit();

	renderHookDraw(&worldDraw, NULL);
	renderHookDraw(&worldDrawChunked, NULL);
	renderRun();
	return EXIT_SUCCESS;
}
