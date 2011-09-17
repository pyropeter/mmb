#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h>
#define PI 3.14159
#define RAD(DEGVAL) (DEGVAL / 180.0 * PI)

#include <GL/gl.h>
#include <GL/freeglut.h>
#if !defined(GLUT_WHEEL_UP)
#  define GLUT_WHEEL_UP   3
#  define GLUT_WHEEL_DOWN 4
#endif

typedef struct {
	float x, y, z;
	float ax, ay; // angle relative to the x- or y-axis
} Camera;

typedef char Block;
typedef struct {
	int sizeX, sizeY, sizeZ;
	Block ***data;
	int foo;
} World;

static int window;
static Camera camera;
static World world;

void blockDraw(Block block, int x, int y, int z) {
	if (block == ' ')
		return;
	
	glPushMatrix();
	
	glTranslatef((float)x, (float)y, (float)z);
	glutSolidCube(1);
	
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
			
			height = (int)(sin(x/6.0)*cos(z/8.0)*(sizeY-2)+sizeY)/2;
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
	int x, y, z;
	for (x = 0; x < world->sizeX; x++) {
		for (y = 0; y < world->sizeY; y++) {
			for (z = 0; z < world->sizeZ; z++) {
				blockDraw(world->data[x][z][y], x, y, z);
			}
		}
	}
}

void draw() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glTranslatef(camera.x, camera.y, camera.z);
	glRotatef(-camera.ax, 0, 1, 0);
	glRotatef(-camera.ay, cosf(RAD(camera.ax)), 0, -sinf(RAD(camera.ax)));

	GLfloat ambientColor[] = {1, 1, 1, 1};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	GLfloat lightColor[] = {1, 1, 1, 1};
	GLfloat lightPosition[] = {50, 40, 20, 1};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	//glutSolidTeapot(1);
	worldDraw(&world);

	glutSwapBuffers();
}

void onkeydown(unsigned char key, int x, int y) {
	if (key == 'q') {
		glutLeaveMainLoop();
	} else if (key == 'w') {
		camera.y -= 0.1;
	} else if (key == 's') {
		camera.y += 0.1;
	} else if (key == 'a') {
		camera.x += 0.1;
	} else if (key == 'd') {
		camera.x -= 0.1;
	} else if (key == 'i') {
		printf("Camera: %1.1f/%1.1f/%1.1f %5.3f/%5.3f\n",
				camera.x, camera.y, camera.z,
				camera.ax, camera.ay);
		printf("World: %ix%ix%i\n",
				world.sizeX, world.sizeY, world.sizeZ);
	}
	glutPostRedisplay();
}

void onmouse(int button, int state, int x, int y) {
	if (state == GLUT_UP && button == GLUT_WHEEL_UP) {
		camera.z += 0.1;
		glutPostRedisplay();
	} else if (state == GLUT_UP && button == GLUT_WHEEL_DOWN) {
		camera.z -= 0.1;
		glutPostRedisplay();
	}
}

void onmotion(int x, int y) {
	//paused = 1;
	camera.ax = (int)((double)-x / glutGet(GLUT_WINDOW_WIDTH) * 360 - 180);
	camera.ay = (int)((double)-y / glutGet(GLUT_WINDOW_HEIGHT) * 360 - 180);
	glutPostRedisplay();
}

void onreshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,		  //The camera angle
		   (double)w / (double)h, //The width-to-height ratio
		   0.01,		   //The near z clipping coordinate
		   200.0);		//The far z clipping coordinate
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
			GLUT_ACTION_CONTINUE_EXECUTION);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	window = glutCreateWindow("Mmb 0.0.1 [Burma Remix]");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	/*glCullFace (GL_BACK);
	glEnable (GL_CULL_FACE);
	glEnable (GL_BLEND);
	glEnable (GL_POLYGON_SMOOTH);
	glBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);*/

	memset(&camera, 0, sizeof(Camera));
	camera.x = -1;
	camera.y = -11;
	camera.z = -16;
	camera.ax = -490;
	camera.ay = -375;
	
	if (worldInit(&world, 50, 10, 50) != 0)
		return EXIT_FAILURE;

	glutDisplayFunc(&draw);
	glutKeyboardFunc(&onkeydown);
	glutMouseFunc(&onmouse);
	glutMotionFunc(&onmotion);
	glutReshapeFunc(&onreshape);

	glutMainLoop();
	//worldFree(&world);
	return EXIT_SUCCESS;
}
