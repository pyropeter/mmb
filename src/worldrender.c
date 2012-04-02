//! @file

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <GL/glew.h>
#include "GL/freeglut.h"
#include "SOIL.h"

#include "defs.h"
#include "vector.h"
#include "worldrender.h"

static long timer = 0;
static int vertices;

static void *versionFont = GLUT_BITMAP_HELVETICA_18;
static Vector2f versionPos;
static char *versionStr;

static Chunk *debugChunk;

static Ray ray;

#define FRAMETIME (1000000/600 * 15)

struct vertexData {
	GLfloat x, y, z;   // 12
	GLfloat s, t;      // 20
	GLbyte nx, ny, nz; // 23
};

typedef struct VisRegion {
	Chunk *someChunk;
	ChunkGroup *group;
	List /* VisRegion */ *adjacent;

	int lastRender;
	int indexStart;
	int indexLen;
} VisRegion;

static int vboUpdate = 1;
static struct vertexData *vertexMem;
static struct vertexData *vertexMemNext;
static unsigned int *indexMem;
static int vboEntryCount;
#define VBO_MAX_ENTRIES 2000000
static GLuint vertexVboId;
static GLuint indexVboId;

static GLuint terrainId;

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

Vector4f convertTexCoords(Vector2i coord)
{
	return (Vector4f){0.0625 * coord.x, 0.0625 * coord.y,
			0.0625 * (coord.x + 1), 0.0625 * (coord.y + 1)};
}

void blockDrawFace(Block *block, int x, int y, int z, int dir) {
	if (block->solid == 0)
		return;

	if (vboEntryCount + 24 > VBO_MAX_ENTRIES)
		return;

	Vector4f t;

	if (dir == DIR_XS) {
		t = convertTexCoords(block->texXS);
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,0+z,t.x,t.w,-127,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,1+z,t.z,t.w,-127,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,1+z,t.z,t.y,-127,0,0};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,0+z,t.x,t.y,-127,0,0};
	} else if (dir == DIR_YS) {
		t = convertTexCoords(block->texYS);
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,0+z,t.x,t.y,0,-127,0};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,0+z,t.z,t.y,0,-127,0};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,1+z,t.z,t.w,0,-127,0};
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,1+z,t.x,t.w,0,-127,0};
	} else if (dir == DIR_ZS) {
		t = convertTexCoords(block->texZS);
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,0+z,t.x,t.w,0,0,-127};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,0+z,t.x,t.y,0,0,-127};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,0+z,t.z,t.y,0,0,-127};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,0+z,t.z,t.w,0,0,-127};
	} else if (dir == DIR_XG) {
		t = convertTexCoords(block->texXG);
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,0+z,t.x,t.w,127,0,0};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,0+z,t.x,t.y,127,0,0};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,1+z,t.z,t.y,127,0,0};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,1+z,t.z,t.w,127,0,0};
	} else if (dir == DIR_YG) {
		t = convertTexCoords(block->texYG);
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,0+z,t.x,t.y,0,127,0};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,1+z,t.z,t.y,0,127,0};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,1+z,t.z,t.w,0,127,0};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,0+z,t.x,t.w,0,127,0};
	} else if (dir == DIR_ZG) {
		t = convertTexCoords(block->texZG);
		*(vertexMemNext++) = (struct vertexData){0+x,0+y,1+z,t.x,t.w,0,0,127};
		*(vertexMemNext++) = (struct vertexData){1+x,0+y,1+z,t.z,t.w,0,0,127};
		*(vertexMemNext++) = (struct vertexData){1+x,1+y,1+z,t.z,t.y,0,0,127};
		*(vertexMemNext++) = (struct vertexData){0+x,1+y,1+z,t.x,t.y,0,0,127};
	}

	vertices += 4;
	vboEntryCount += 4;
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

void drawAdjacentChunk(Camera *camera, Chunk *chunk, Chunk *other) {
	Block **block = other->blocks;
	int x, y, z;

#define LOOP(cond, a, b, dir) \
	for (x = other->low.x; x <= other->high.x; x++) { \
	for (y = other->low.y; y <= other->high.y; y++) { \
	for (z = other->low.z; z <= other->high.z; z++) { \
		if (cond \
		&&  a >= chunk->low.a \
		&&  b >= chunk->low.b \
		&&  a <= chunk->high.a \
		&&  b <= chunk->high.b) \
			blockDrawFace(*block, x,y,z, dir); \
		block++; \
	}}}

	if (chunk->low.x == other->high.x + 1) {
		LOOP(x == other->high.x, y, z, DIR_XG)
	} else if (chunk->low.y == other->high.y + 1) {
		LOOP(y == other->high.y, x, z, DIR_YG)
	} else if (chunk->low.z == other->high.z + 1) {
		LOOP(z == other->high.z, x, y, DIR_ZG)
	} else if (chunk->high.x == other->low.x - 1) {
		LOOP(x == other->low.x, y, z, DIR_XS)
	} else if (chunk->high.y == other->low.y - 1) {
		LOOP(y == other->low.y, x, z, DIR_YS)
	} else if (chunk->high.z == other->low.z - 1) {
		LOOP(z == other->low.z, x, y, DIR_ZS)
	} else {
		assert(0);
	}
#undef LOOP
}

int isBubbleVisible(Camera *camera, Bubble *bubble) {
	if(bubble->chunk->low.x > camera->high.x
	|| bubble->chunk->low.y > camera->high.y
	|| bubble->chunk->low.z > camera->high.z
	|| bubble->chunk->high.x < camera->low.x
	|| bubble->chunk->high.y < camera->low.y
	|| bubble->chunk->high.z < camera->low.z)
		return 0;
	return 1;
}

void renderBubbleChunk(World *world, Camera *camera,
		Bubble *bubble, Chunk *chunk)
{
	chunk->cookie = world->cookie;

	Chunk **other;
	LISTITER(chunk->adjacent, other, Chunk**) {
		if ((*other)->blocks) {
			// other is solid
			drawAdjacentChunk(camera, chunk, *other);
		} else {
			// other is transparent
			if ((*other)->cookie != world->cookie
					&& (*other)->bubble == bubble)
				renderBubbleChunk(world, camera,
						bubble, *other);
		}
	}
}

void findBubbles(World *world, Camera *camera, Bubble *bubble)
{
	bubble->cookie = world->cookie;
	worldUpdateBubble(world, bubble);
	if (bubble->status == 0)
		return;

	if (!isBubbleVisible(camera, bubble))
		return;

	renderBubbleChunk(world, camera, bubble, bubble->chunk);

	Bubble **other;
	LISTITER(bubble->adjacent, other, Bubble**) {
		if ((*other)->cookie != world->cookie)
			findBubbles(world, camera, *other);
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

void debugSomeStuff(World *world, Camera *camera) {
	Chunk *chunk = debugChunk;
	Chunk **other;
	LISTITER(chunk->adjacent, other, Chunk**) {
		drawChunkBorder(*other);
	}
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

	glEnable(GL_TEXTURE_2D);

	if (vboUpdate || world->bubblesUpdated > 0) {
		vertices = 0;
		// update vertexMem and indexMem
		vertexMem = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		vertexMemNext = vertexMem;
		vboEntryCount = 0;

		Bubble *startBubble = worldGetBubble(world, camera->pos);
		findBubbles(world, camera, startBubble);

		glUnmapBuffer(GL_ARRAY_BUFFER);

		vboUpdate = 0;
	}

	// draw the scene
	glDrawElements(GL_QUADS, vboEntryCount, GL_UNSIGNED_INT, (GLvoid*) 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	hilightSelection(world, camera);

//	debugSomeStuff(world, camera);

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

	glRasterPos2f(versionPos.x, versionPos.y);
	glutBitmapString(versionFont, (unsigned char*)versionStr);

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

	if (asprintf(&versionStr, "MMB \"%s\" %s", CODENAME, VERSION) == -1) {
		fprintf(stderr, "Out of memory.\n");
		exit(EXIT_FAILURE);
	}

	debugChunk = worldGetChunk(world, (Vector3i){-4,-2,0});

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	terrainId = SOIL_load_OGL_texture("gmcraft_terrain.png", SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID, 0);
	if (terrainId == 0) {
		fprintf(stderr, "SOIL Error.\n");
		exit(EXIT_FAILURE);
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, terrainId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat[]){1, 1, 1, 1});

	// create ambient light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (GLfloat[]){0.8, 0.8, 0.8, 1});

	// create light 0 (sun)
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (GLfloat[]){1, 1, 1, 1});

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(0,1,0,1);

	glMatrixMode(GL_PROJECTION);

	glGenBuffers(1, &vertexVboId);
	glGenBuffers(1, &indexVboId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVboId);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

//	vertexMem = knalloc(VBO_MAX_ENTRIES * sizeof(struct vertexData));
	indexMem = knalloc(VBO_MAX_ENTRIES * sizeof(unsigned int));

	int i;
	for (i = 0; i < VBO_MAX_ENTRIES; i++) {
		indexMem[i] = i;
	}

	glBufferData(GL_ARRAY_BUFFER, VBO_MAX_ENTRIES
			* sizeof(struct vertexData), NULL,
			GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, VBO_MAX_ENTRIES
			* sizeof(unsigned int), indexMem,
			GL_STATIC_DRAW);

	glVertexPointer(3, GL_FLOAT, sizeof(struct vertexData), (GLvoid*) 0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertexData), (GLvoid*) 12);
	glNormalPointer(GL_BYTE, sizeof(struct vertexData), (GLvoid*) 20);

	timer = startTimer();
}

void worldrenderReshape(World *world, Camera *camera, int w, int h)
{
	glViewport(0, 0, w, h);

	// on projection matrix:
	glLoadIdentity();
	gluPerspective(45, (double)w/h, 1, 200);

	// calculate position for version string
	versionPos = (Vector2f){
		(float)6/w,
		(float)(h - glutBitmapHeight(versionFont))/h};
}

void worldrenderDraw(World *world, Camera *camera)
{
	long pre, scene, gui, draw, update, sleep;
//	vertices = 0;
	pre = stopTimer(timer);

	worldrenderDrawSzene(world, camera);
//	glFinish();
	scene = stopTimer(timer);

	worldrenderDrawGui(world, camera);
//	glFinish();
	gui = stopTimer(timer);

//	glutSwapBuffers();
//	glFinish();
	draw = stopTimer(timer);

	worldAfterFrame(world);
//	glFinish();
	update = stopTimer(timer);

	// hold constant framerate
	long timeleft = FRAMETIME - update;
	if (timeleft > 0) {
//		usleep(timeleft);
	}
	glutSwapBuffers();
	glFinish();
	sleep = stopTimer(timer);

	// start timer for next frame
	timer = startTimer();

	// print statistics
//	printf("frame: %i %5li %5li %5li %5li %5li %5li\n", vertices,
//			pre, scene, gui, draw, update, sleep);
	fflush(stdout);
}

Ray *worldrenderGetRay(World *world, Camera *camera)
{
	return &ray;
}

void worldrenderRefresh() {
	vboUpdate = 1;
}

