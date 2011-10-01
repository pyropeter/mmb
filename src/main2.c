#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "render.h"
#include <GL/gl.h>
#include <GL/freeglut.h>

typedef char Block;
typedef struct {
	int sizeX, sizeY, sizeZ;
	Block ***data;
	int foo;
} World;

struct Drawlistitem_ {
	Block *block;
	int x, y, z;
	int distX, distY, distZ;
	int distance;
	struct Drawlistitem_ *next;
};
typedef struct Drawlistitem_ Drawlistitem;

static Render *render;
static Camera *camera;
static World world;
static Drawlistitem *drawlist;

void pushDrawlist(Block *block, int x, int y, int z) {
	Drawlistitem *item = malloc(sizeof(Drawlistitem));
	item->block = block;
	item->x = x;
	item->y = y;
	item->z = z;
	item->distX = x - camera->ix;
	item->distY = y - camera->iy;
	item->distZ = z - camera->iz;
	item->distance = pow(item->distX, 2)
			+ pow(item->distY, 2) + pow(item->distZ, 2);

	Drawlistitem **prev = &drawlist;
	while (*prev) {
		if ((*prev)->distance < item->distance)
			break;
		prev = &((*prev)->next);
	}

	item->next = *prev;
	*prev = item;
}

void blockDrawFromList(Drawlistitem *item) {
	if (*(item->block) == ' ')
		return;
	
	glPushMatrix();
	
	glTranslatef((float)item->x, (float)item->y, (float)item->z);

	glBegin(GL_QUADS); {
		if (item->distX > 0) {
			glNormal3f(-1,0,0);
			glVertex3f(0,0,0);
			glVertex3f(0,0,1);
			glVertex3f(0,1,1);
			glVertex3f(0,1,0);
			render->vertices += 4;
		}

		if (item->distY > 0) {
			glNormal3f(0,-1,0);
			glVertex3f(0,0,0);
			glVertex3f(0,0,1);
			glVertex3f(1,0,1);
			glVertex3f(1,0,0);
			render->vertices += 4;
		}

		if (item->distZ > 0) {
			glNormal3f(0,0,-1);
			glVertex3f(0,0,0);
			glVertex3f(0,1,0);
			glVertex3f(1,1,0);
			glVertex3f(1,0,0);
			render->vertices += 4;
		}

		if (item->distX < 0) {
			glNormal3f(1,0,0);
			glVertex3f(1,0,0);
			glVertex3f(1,0,1);
			glVertex3f(1,1,1);
			glVertex3f(1,1,0);
			render->vertices += 4;
		}

		if (item->distY < 0) {
			glNormal3f(0,1,0);
			glVertex3f(0,1,0);
			glVertex3f(0,1,1);
			glVertex3f(1,1,1);
			glVertex3f(1,1,0);
			render->vertices += 4;
		}

		if (item->distZ < 0) {
			glNormal3f(0,0,1);
			glVertex3f(0,0,1);
			glVertex3f(0,1,1);
			glVertex3f(1,1,1);
			glVertex3f(1,0,1);
			render->vertices += 4;
		}
	}; glEnd();
	
	glPopMatrix();
}

int worldInit(World *world, int sizeX, int sizeY, int sizeZ) {
	memset(world, 0, sizeof(World));
	world->sizeX = sizeX;
	world->sizeY = sizeY;
	world->sizeZ = sizeZ;
	
	world->data = malloc(sizeX * sizeof(Block**));
	if (world->data == NULL)
		return -1;
	
	int x, z, height;
	for (x = 0; x < sizeX; x++) {
		world->data[x] = malloc(sizeZ * sizeof(Block*));
		if (world->data[x] == NULL)
			return -1;
		
		for (z = 0; z < sizeZ; z++) {
			world->data[x][z] = malloc(sizeY * sizeof(Block));
			if (world->data[x][z] == NULL)
				return -1;
			memset(world->data[x][z], ' ', sizeY * sizeof(Block));
			
			height = (int)(
					sin(x/6.0) * cos(z/8.0)
					* (sizeY-1) + sizeY ) / 2 + 1;
			memset(world->data[x][z], 's', height * sizeof(Block));
		}
	}
	return 0;
}

int worldFree(World *world) {
	int x,z;
	for (x = 0; x < world->sizeX; x++) {
		for (z = 0; z < world->sizeZ; z++) {
			free(world->data[x][z]);
		}
		free(world->data[x]);
	}
	free(world->data);
	world->data = NULL;
	return 0;
}

void worldDraw(World *world) {
	drawlist = NULL;

	int x, y, z;
	for (x = 0; x < world->sizeX; x++) {
		for (y = 0; y < world->sizeY; y++) {
			for (z = 0; z < world->sizeZ; z++) {
				pushDrawlist(&(world->data[x][z][y]), x, y, z);
			}
		}
	}

	Drawlistitem *next;
	while (drawlist) {
		blockDrawFromList(drawlist);
		next = drawlist->next;
		free(drawlist);
		drawlist = next;
	}
}

int main(int argc, char *argv[]) {
	render = renderInit(argc, argv);
	camera = &(render->camera);
	
	if (worldInit(&world, 10, 5, 10) != 0)
		return EXIT_FAILURE;
	
	renderHookDraw(&worldDraw, &world);
	renderRun();
	return EXIT_SUCCESS;
}
