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
	float dx, dy, dz;
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
static int mouseX, mouseY, mouseSens;

void blockDraw(Block block, int x, int y, int z) {
	if (block == ' ')
		return;
	
	glPushMatrix();
	
	glTranslatef((float)x, (float)y, (float)z);
	//glutSolidCube(1);

	glBegin(GL_QUADS); {
		glNormal3f(-1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(0,0,1);
		glVertex3f(0,1,1);
		glVertex3f(0,1,0);

		glNormal3f(0,-1,0);
		glVertex3f(0,0,0);
		glVertex3f(0,0,1);
		glVertex3f(1,0,1);
		glVertex3f(1,0,0);

		glNormal3f(0,0,-1);
		glVertex3f(0,0,0);
		glVertex3f(0,1,0);
		glVertex3f(1,1,0);
		glVertex3f(1,0,0);

		glNormal3f(0,1,0);
		glVertex3f(0,1,0);
		glVertex3f(0,1,1);
		glVertex3f(1,1,1);
		glVertex3f(1,1,0);

		glNormal3f(1,0,0);
		glVertex3f(1,0,0);
		glVertex3f(1,0,1);
		glVertex3f(1,1,1);
		glVertex3f(1,1,0);

		glNormal3f(0,1,1);
		glVertex3f(0,0,1);
		glVertex3f(0,1,1);
		glVertex3f(1,1,1);
		glVertex3f(1,0,1);

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

	//glTranslatef(camera.x, camera.y, camera.z);
	//glRotatef(-camera.ax, 0, 1, 0);
	//glRotatef(-camera.ay, cosf(RAD(camera.ax)), 0, -sinf(RAD(camera.ax)));

	//gluLookAt(camera.x, camera.y, camera.z,  0,0,0,  0,1,0);
	//gluLookAt(camera.x, camera.y, camera.z,
	//		camera.x + sinf(camera.ay) * sinf(camera.ax),
	//		camera.y + cosf(camera.ay) * sinf(camera.ax),
	//		camera.z + cosf(camera.ax),
	//		0,1,0);
	gluLookAt(camera.x, camera.y, camera.z,
			camera.x + camera.dx,
			camera.y + camera.dy,
			camera.z + camera.dz,
			0,1,0);

	GLfloat ambientColor[] = {1, 1, 1, 1};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	GLfloat lightColor[] = {1, 1, 1, 1};
	GLfloat lightPosition[] = {-1.5, -1, -0.5, 0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glBegin(GL_LINES); {
		glVertex3f(lightPosition[0], lightPosition[1],
				lightPosition[2]);
		glVertex3f(0,0,0);
	}; glEnd();

	//glutSolidTeapot(1);
	worldDraw(&world);

	glutSwapBuffers();
}

void onkeydown(unsigned char key, int x, int y) {
	switch (key) {
		case 'w':
			camera.x += camera.dx;
			camera.y += camera.dy;
			camera.z += camera.dz;
			break;
		case 's':
			camera.x -= camera.dx;
			camera.y -= camera.dy;
			camera.z -= camera.dz;
			break;
		case 'a':
			camera.x += camera.dz;
			camera.z -= camera.dx;
			break;
		case 'd':
			camera.x -= camera.dz;
			camera.z += camera.dx;
			break;

		case 'q':
			glutLeaveMainLoop();
			break;
		case 'i':
			printf("Camera: %1.1f/%1.1f/%1.1f %1.1f/%1.1f/%1.1f\n",
					camera.x, camera.y, camera.z,
					camera.dx, camera.dy, camera.dz);
			printf("World: %ix%ix%i\n",
					world.sizeX, world.sizeY, world.sizeZ);
			break;
		default:
			printf("No mapping for key %i\n", key);
			break;
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
	float ax = (float) -y / glutGet(GLUT_WINDOW_WIDTH) * PI;
	float ay = (float) -x / glutGet(GLUT_WINDOW_HEIGHT) * PI * 2;
	camera.dx = sinf(ax) * sinf(ay);
	camera.dy = cosf(ax);
	camera.dz = sinf(ax) * cosf(ay);
	glutPostRedisplay();
}

void onpassivemotion(int x, int y) {
	int cx = glutGet(GLUT_WINDOW_WIDTH)/2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT)/2;
	if (x - cx == 0 && y -cy == 0)
		return;

	mouseX = ( mouseX + x - cx ) % mouseSens;
	mouseY += y - cy;
	if (mouseY > mouseSens) mouseY = mouseSens;
	if (mouseY < 0) mouseY = 0;

	float ax = (float) -mouseY / mouseSens * PI;
	float ay = (float) -mouseX / mouseSens * PI * 2;
	camera.dx = sinf(ax) * sinf(ay);
	camera.dy = cosf(ax);
	camera.dz = sinf(ax) * cosf(ay);

	glutWarpPointer(cx, cy);
	glutPostRedisplay();
}

void onreshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,		  //The camera angle
		   (double)w / (double)h, //The width-to-height ratio
		   1.0,		   //The near z clipping coordinate
		   200.0);		//The far z clipping coordinate
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
			GLUT_ACTION_CONTINUE_EXECUTION);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
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
	camera.y = 10;
	camera.dx = 0.6;
	camera.dy = -0.4;
	camera.dz = 0.7;
	mouseX = mouseY = 0;
	mouseSens = 500;
	
	if (worldInit(&world, 30, 8, 30) != 0)
		return EXIT_FAILURE;

	glutDisplayFunc(&draw);
	glutKeyboardFunc(&onkeydown);
	//glutMouseFunc(&onmouse);
	//glutMotionFunc(&onmotion);
	glutPassiveMotionFunc(&onpassivemotion);
	glutReshapeFunc(&onreshape);

	glutMainLoop();
	//worldFree(&world);
	return EXIT_SUCCESS;
}
