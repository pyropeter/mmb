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
#include "raytrace.h"
#include "chunksplit.h"

static Render *render;
static Camera *camera;
static World *world;
static Ray ray;

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

//! draw chunk's bounding rectangle as wireframe
void drawChunkBorder(Chunk *chunk) {
	int x1 = (long long int)chunk->low.x;
	int y1 = (long long int)chunk->low.y;
	int z1 = (long long int)chunk->low.z;
	int x2 = (long long int)chunk->high.x + 1;
	int y2 = (long long int)chunk->high.y + 1;
	int z2 = (long long int)chunk->high.z + 1;

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
}

//! draw block's bounding rectangle as wireframe
void drawBlockBorder(Vector3i pos) {
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

	drawChunkBorder(chunk);

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

	// we will abuse chunkGet(), so backup it's state
	Chunk *lastChunk = world->lastChunk;
	Vector3i lastPos = world->lastPos;

	ray.posi = camera->pos;
	ray.posf.x = camera->x - camera->pos.x;
	ray.posf.y = camera->y - camera->pos.y;
	ray.posf.z = camera->z - camera->pos.z;
	ray.dir = (Vector3f){camera->dx, camera->dy, camera->dz};
/*
	ray.posi = (Vector3i){-5,1,-7};
	ray.posf = (Vector3f){0.8,0.5,0.6};
	ray.dir  = (Vector3f){-0.5,-0.01,-0.9};

	Vector3f start = VEC3FOP(ray.posf, +, ray.posi);
	Vector3f len  = (Vector3f){100,100,100};
	Vector3f rayvec = VEC3FOP(ray.dir, *, len);
	Vector3f end = VEC3FOP(start, +, rayvec);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES); {
		glVertex3f(start.x, start.y, start.z);
		glVertex3f(end.x, end.y, end.z);
	}; glEnd();
	glEnable(GL_LIGHTING);
*/
	ray.chunk = worldGetChunk(world, ray.posi);
	ray.world = world;
	ray.factor = 0;

/*	printf("BEGIN ---------------------\n");
	VECPRINT(ray.posi, " (posi)\n");
	VECFPRINT(ray.posf, " (posf)\n");
	VECFPRINT(ray.dir, " (dir)\n");
	renderDebug();
*/
	for (i = 0; i < 20 && ray.chunk; i++) {
		if (ray.chunk->blocks) {
			// found a solid chunk
//			printf("FOUND\n");
			drawBlockBorder(ray.first);
			break;
		}

		raytraceNext(&ray);
//		drawBlockBorder(ray.first);
//		drawChunkBorder(ray.chunk);
//		VECPRINT(ray.chunk->low, " (low) \t\t");
//		VECPRINT(ray.chunk->high, " (high)\n");
//		VECPRINT(ray.first, " (first)\n");
	}
//	printf("END -----------------------\n");

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

void setBlock(World *world, Vector3i pos, Block *block)
{
	Chunk *chunk = chunksplitSplit(world, pos);

	if (*block == ' ') {
		free(chunk->blocks);
		chunk->blocks = NULL;
	} else {
		if (chunk->blocks == NULL)
			chunk->blocks = knalloc(sizeof(Block*));
		chunk->blocks[0] = block;
	}
}

void onMouse(int button, int state, void *data)
{
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
//		chunksplitSplit(world, ray.chunk, PLANE_YZ, ray.first.x);
//		chunksplitSplit(world, ray.chunk, PLANE_XZ, ray.first.y);
		setBlock(world, ray.first, " ");
	} else if (state == GLUT_UP && button == GLUT_RIGHT_BUTTON) {
		setBlock(world, ray.last, "s");
	}
}

int main(int argc, char *argv[]) {
	printf("MMB version %s, \"White Cubes\"\n\n", VERSION);

	render = renderInit(argc, argv);
	camera = &(render->camera);

	generatorInit();

	world = worldInit(generatorGetBlock);

	renderHookDraw(&worldDrawChunked, NULL);
	renderHookMouse(&onMouse, NULL);
	renderRun();
	return EXIT_SUCCESS;
}
