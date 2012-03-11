#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <GL/glew.h>
#include "GL/freeglut.h"

#include "defs.h"
#include "vector.h"
#include "worldrender.h"

static long timer = 0;
static int vertices;

static Ray ray;

#define FRAMETIME (1000000/600 * 11)

struct vertexData {
	GLfloat x, y, z;
	GLbyte nx, ny, nz;
	char padding;
};

static int vboUpdate = 1;
static struct vertexData *vertexMem;
static struct vertexData *vertexMemNext;
static unsigned int *indexMem;
static int vboEntryCount;
#define VBO_MAX_ENTRIES 10000000
static int vertexVboId;
static int indexVboId;

/**
 * OpenGL default state
 * --------------------
 *
 * GL_LIGHTING = disabled
 * GL_DEPTH_TEST = disabled
 *
 * Matrix Mode: GL_PROJECTION
 * Projection Matrix: gluPerspective(45, w/h, 1, 200) (for scene rendering)
 * Modelview Matrix: gluOrtho2D(0,1,0,1) (for 2d overlay)
 *
 * Lights 0 and 1 are defined. (for scene rendering)
 *
 * */

void blockDrawDist(Block *block, int x, int y, int z,
		int distX, int distY, int distZ) {
	if (*block == ' ')
		return;

	if (vboEntryCount + 24 > VBO_MAX_ENTRIES)
		return;

	if (distX > 0) {
		//Normal3f(-1,0,0);
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,0+z,0,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,1+z,-127,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,1+z,-127,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,0+z,-127,0,0};
		vertices += 4;
		vboEntryCount += 4;
	}

	if (distY > 0) {
		//Normal3f(0,-127,0);
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,0+z,0,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,1+z,0,-127,0};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,1+z,0,-127,0};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,0+z,0,-127,0};
		vertices += 4;
		vboEntryCount += 4;
	}

	if (distZ > 0) {
		//Normal3f(0,0,-1);
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,0+z,0,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,0+z,0,0,-127};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,0+z,0,0,-127};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,0+z,0,0,-127};
		vertices += 4;
		vboEntryCount += 4;
	}

	if (distX < 0) {
		//Normal3f(1,0,0);
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,0+z,0,0,0};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,1+z,127,0,0};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,1+z,127,0,0};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,0+z,127,0,0};
		vertices += 4;
		vboEntryCount += 4;
	}

	if (distY < 0) {
		//Normal3f(0,1,0);
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,0+z,0,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,1+z,0,127,0};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,1+z,0,127,0};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,0+z,0,127,0};
		vertices += 4;
		vboEntryCount += 4;
	}

	if (distZ < 0) {
		//Normal3f(0,0,1);
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,1+z,0,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,1+z,0,0,127};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,1+z,0,0,127};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,1+z,0,0,127};
		vertices += 4;
		vboEntryCount += 4;
	}
}

//! draw chunk's bounding rectangle as wireframe
void drawChunkBorder(Chunk *chunk) {
	int x1 = (long long int)chunk->low.x;
	int y1 = (long long int)chunk->low.y;
	int z1 = (long long int)chunk->low.z;
	int x2 = (long long int)chunk->high.x + 1;
	int y2 = (long long int)chunk->high.y + 1;
	int z2 = (long long int)chunk->high.z + 1;

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
}

//! draw block's bounding rectangle as wireframe
void drawBlockBorder(Vector3i pos) {
	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glScalef(1.2, 1.2, 1.2);
	glTranslatef(-0.1, -0.1, -0.1);

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

	glPopMatrix();
}

void drawChunk(Camera *camera, Chunk *chunk) {
	if (chunk->blocks == NULL)
		return;

//	drawChunkBorder(chunk);

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

int isVisible(Camera *camera, Chunk *chunk) {
	if(chunk->low.x > camera->high.x
	|| chunk->low.y > camera->high.y
	|| chunk->low.z > camera->high.z
	|| chunk->high.x < camera->low.x
	|| chunk->high.y < camera->low.y
	|| chunk->high.z < camera->low.z)
		return 0;
	return 1;
}

void findChunks(World *world, Camera *camera, Chunk *startChunk) {
	worldUpdateChunk(world, startChunk);

	Chunk **chunk = (Chunk**)startChunk->adjacent->mem;
	for (; (void**)chunk != startChunk->adjacent->nextFree; chunk++) {
		if ((*chunk)->cookie != world->cookie) {
			(*chunk)->cookie = world->cookie;

			// out of range?
			if (!isVisible(camera, *chunk))
				continue;

			drawChunk(camera, *chunk);

			if ((*chunk)->blocks == NULL) // transparent block
				findChunks(world, camera, *chunk);
		}
	}
}

void hilightSelection(World *world, Camera *camera) {
	// we will abuse chunkGet() => save its state
	Chunk *lastChunk = world->lastChunk;
	Vector3i lastPos = world->lastPos;

	ray.posi = camera->pos;
	ray.posf.x = camera->x - camera->pos.x;
	ray.posf.y = camera->y - camera->pos.y;
	ray.posf.z = camera->z - camera->pos.z;
	ray.dir = (Vector3f){camera->dx, camera->dy, camera->dz};

	ray.chunk = worldGetChunk(world, ray.posi);
	ray.world = world;
	ray.factor = 0;

	int i;
	for (i = 0; i < 20 && ray.chunk &&
			isVisible(camera, ray.chunk); i++) {
		if (ray.chunk->blocks) {
			// found a solid chunk
			drawBlockBorder(ray.first);
			break;
		}

		raytraceNext(&ray);
	}

	// restore chunkGet()'s state
	world->lastChunk = lastChunk;
	world->lastPos = lastPos;
}

void worldrenderDrawSzene(World *world, Camera *camera)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	gluLookAt(camera->x, camera->y, camera->z,
			camera->x + camera->dx,
			camera->y + camera->dy,
			camera->z + camera->dz,
			0, 1, 0);

	world->cookie++;

	// reenable lighting:
	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat[]){0, 1, 0, 0});
	glLightfv(GL_LIGHT1, GL_POSITION, (GLfloat[]){-1, 1, 0, 0});

//	Chunk *startChunk = worldGetChunk(world, camera->pos);
//	startChunk->cookie = world->cookie;

//	glBegin(GL_QUADS);
//	findChunks(world, camera, startChunk);
//	glEnd();

	if (vboUpdate || world->chunksUpdated > 0) {
		vertices = 0;
		// update vertexMem and indexMem
		vertexMemNext = vertexMem;
		vboEntryCount = 0;
		Chunk *startChunk = worldGetChunk(world, camera->pos);
		startChunk->cookie = world->cookie;
		findChunks(world, camera, startChunk);

		// update vbo
//		glGenBuffers(1, &vertexVboId);
//		glGenBuffers(1, &indexVboId);

//		glBindBuffer(GL_ARRAY_BUFFER, vertexVboId);
		glBufferData(GL_ARRAY_BUFFER, VBO_MAX_ENTRIES
				* sizeof(struct vertexData), vertexMem,
				GL_STATIC_DRAW);

//		glBindBuffer(GL_ARRAY_BUFFER, indexVboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, VBO_MAX_ENTRIES
				* sizeof(unsigned int), indexMem,
				GL_STATIC_DRAW);

		vboUpdate = 0;
	}

	// draw the scene
//	glBindBuffer(GL_ARRAY_BUFFER, vertexVboId);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVboId);
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(struct vertexData), 0);
	glNormalPointer(GL_BYTE, sizeof(struct vertexData), 12);
	glDrawElements(GL_QUADS, vboEntryCount, GL_UNSIGNED_INT, 0);
//	glDisableClientState(GL_VERTEX_ARRAY);
//	glDisableClientState(GL_NORMAL_ARRAY);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisable(GL_LIGHTING);
	hilightSelection(world, camera);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);

	glDisable(GL_DEPTH_TEST);
}

void worldrenderDrawGui(World *world, Camera *camera)
{
	// on projection matrix:
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_LINES); {
		glVertex2f(0.5,0.45);
		glVertex2f(0.5,0.55);
		glVertex2f(0.45,0.5);
		glVertex2f(0.55,0.5);
	}; glEnd();

	glPopMatrix();
}

void worldrenderInit(World *world, Camera *camera)
{
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	printf("glew: %s\n", glewGetString(GLEW_VERSION));

	// create ambient light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (GLfloat[]){1, 1, 1, 1});

	// create light 0
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (GLfloat[]){0.5, 0.5, 1, 1});

	// create light 1
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, (GLfloat[]){1, 0, 0, 1});

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0,1,0,1);

	glMatrixMode(GL_PROJECTION);

	glGenBuffers(1, &vertexVboId);
	glGenBuffers(1, &indexVboId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVboId);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	vertexMem = knalloc(VBO_MAX_ENTRIES * sizeof(struct vertexData));
	indexMem = knalloc(VBO_MAX_ENTRIES * sizeof(unsigned int));

	int i;
	for (i = 0; i < VBO_MAX_ENTRIES; i++) {
		indexMem[i] = i;
	}

	timer = startTimer();
}

void worldrenderReshape(World *world, Camera *camera, int w, int h)
{
	glViewport(0, 0, w, h);

	// on projection matrix:
	glLoadIdentity();
	gluPerspective(45, (double)w/h, 1, 200);
}

void worldrenderDraw(World *world, Camera *camera)
{
	long pre, scene, gui, draw, update, sleep;
//	vertices = 0;
	pre = stopTimer(timer);

	worldrenderDrawSzene(world, camera);
	scene = stopTimer(timer);

	worldrenderDrawGui(world, camera);
	gui = stopTimer(timer);

//	glutSwapBuffers();
	glFinish();
	draw = stopTimer(timer);

	worldAfterFrame(world);
	update = stopTimer(timer);

	// hold constant framerate
	long timeleft = FRAMETIME - update;
	if (timeleft > 0) {
		usleep(timeleft);
	}
	glutSwapBuffers();
	sleep = stopTimer(timer);

	// start timer for next frame
	timer = startTimer();

	// print statistics
	printf("frame: %i %5li %5li %5li %5li %5li %5li\n", vertices,
			pre, scene, gui, draw, update, sleep);
	fflush(stdout);
}

Ray *worldrenderGetRay(World *world, Camera *camera)
{
	return &ray;
}

void worldrenderRefresh() {
	vboUpdate = 1;
}

