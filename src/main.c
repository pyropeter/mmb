#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/freeglut.h>

#include "render.h"
#include "generator.h"

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

void blockDrawDist(Block *block, Comp x_, Comp y_, Comp z_,
		int distX, int distY, int distZ) {
	if (*block == ' ')
		return;
	
	glPushMatrix();
	
	int x = (long long int)x_ - HALFCOMP;
	int y = (long long int)y_ - HALFCOMP;
	int z = (long long int)z_ - HALFCOMP;
	
	//glTranslated((double)x, (double)y, (double)z);

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

void blockDrawFromList(Drawlistitem *item) {
	blockDrawDist(item->block, item->x, item->y, item->z,
			item->distX, item->distY, item->distZ);
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

void worldDraw2(void *foo) {
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
	
	//if (worldInit(&world, 40, 3, 40) != 0)
	//	return EXIT_FAILURE;
	generatorInit();
	
	//renderHookDraw((int (*)(void *))&worldDraw, &world);
	renderHookDraw((int (*)(void *))&worldDraw2, NULL);
	renderRun();
	return EXIT_SUCCESS;
}
