/* 
 * 
 * 
 * gcc -Wall -g -c render.c -o render
 * 
 * */

#include "render.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/freeglut.h>
#if !defined(GLUT_WHEEL_UP)
#  define GLUT_WHEEL_UP   3
#  define GLUT_WHEEL_DOWN 4
#endif

static Render render;

// =====================================================================

void moveCamera(double dx, double dy, double dz) {
	render.camera.x += dx;
	render.camera.y += dy;
	render.camera.z += dz;
	render.camera.ix = (Comp)floor(render.camera.x);
	render.camera.iy = (Comp)floor(render.camera.y);
	render.camera.iz = (Comp)floor(render.camera.z);
	render.camera.low.x = render.camera.ix - render.camera.range;
	render.camera.low.y = render.camera.iy - render.camera.range;
	render.camera.low.z = render.camera.iz - render.camera.range;
	render.camera.high.x = render.camera.ix + render.camera.range;
	render.camera.high.y = render.camera.iy + render.camera.range;
	render.camera.high.z = render.camera.iz + render.camera.range;
}

void rotateCamera(float dax, float day) {
	/* 
	 * Adds dax and day to the current camera rotation.
	 * 
	 * Attention: The rotation relative to the X-axis cannot be raised over
	 * PI and not be lowered below zero.
	 * 
	 * */

	render.camera.ax += dax;
	if (render.camera.ax < 0.1) render.camera.ax = 0.1;
	if (render.camera.ax > 3.1) render.camera.ax = 3.1;
	render.camera.ay = fmodf(render.camera.ay + day, M_PI * 2);

	render.camera.dx = sinf(render.camera.ax) * sinf(render.camera.ay);
	render.camera.dy = cosf(render.camera.ax);
	render.camera.dz = sinf(render.camera.ax) * cosf(render.camera.ay);
}

void renderDebug() {
	printf("===== RENDER DEBUG =====\n");
	printf("Camera: %1.1f/%1.1f/%1.1f %1.1f/%1.1f/%1.1f %li/%li/%li\n",
			render.camera.x - HALFCOMP,
			render.camera.y - HALFCOMP,
			render.camera.z - HALFCOMP,
			render.camera.dx, render.camera.dy, render.camera.dz,
			render.camera.ix - HALFCOMP,
			render.camera.iy - HALFCOMP,
			render.camera.iz - HALFCOMP);
	printf("Vertices drawn: %i\n", render.vertices);
	printf("===== END =====\n");
}

void onPassiveMotion(int x, int y) {
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	int dx = x - cx;
	int dy = y - cy;
	if (dx == 0 && dy == 0)
		return;
	
	float ax = dy * render.mouseSens * M_PI;
	float ay = dx * render.mouseSens * M_PI * 2;
	rotateCamera(ax, -ay);

	glutWarpPointer(cx, cy);
	glutPostRedisplay();
}

void catchPointer() {
	glutSetCursor(GLUT_CURSOR_NONE);
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	glutWarpPointer(cx, cy);
	glutPassiveMotionFunc(&onPassiveMotion);
}

void freePointer() {
	glutSetCursor(GLUT_CURSOR_INHERIT);
	glutPassiveMotionFunc(NULL);
}

void onReshape(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,			//The camera angle
			(double)w / (double)h,	//The width-to-height ratio
			1.0,			//The near z clipping coordinate
			200.0);			//The far z clipping coordinate
}

void onKeyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'w':
			moveCamera(render.camera.dx, 0, render.camera.dz);
			break;
		case 's':
			moveCamera(-render.camera.dx, 0, -render.camera.dz);
			break;
		case 'a':
			moveCamera(render.camera.dz, 0, -render.camera.dx);
			break;
		case 'd':
			moveCamera(-render.camera.dz, 0, render.camera.dx);
			break;
		case ' ':
			moveCamera(0, 0.5, 0);
			break;
		case '<':
			moveCamera(0, -0.5, 0);
			break;

		case 27: // ESC
			freePointer();
			break;

		case 'q':
			glutLeaveMainLoop();
			break;
		case 'i':
			renderDebug();
			break;
		default:
			printf("No mapping for key %i\n", key);
			break;
	}
	glutPostRedisplay();
}

void onMouse(int button, int state, int x, int y) {
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		catchPointer();
	}
}

void onDisplay() {
	//if (++frameno == 1000)
	//	glutLeaveMainLoop();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	double x = render.camera.x - HALFCOMP;
	double y = render.camera.y - HALFCOMP;
	double z = render.camera.z - HALFCOMP;

	gluLookAt(x, y, z,
			x + render.camera.dx,
			y + render.camera.dy,
			z + render.camera.dz,
			0, 1, 0);

	GLfloat ambientColor[] = {1, 1, 1, 1};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	GLfloat lightColor[] = {0.5, 0.5, 1, 1};
	GLfloat lightPosition[] = {0, 1, 0, 0};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	GLfloat lightColor1[] = {1, 0, 0, 1};
	GLfloat lightPosition1[] = {-1, 1, 0, 0};
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);

	render.vertices = 0;
	if (render.onDraw)
		render.onDraw(render.onDrawData);

	glutSwapBuffers();
}

void onIdle() {
	glutPostRedisplay();
}

Render *renderInit(int argc, char *argv[]) {
	render.onDraw = NULL;
	render.onDrawData = NULL;

	render.mouseSens = 1.0/1000;

	render.camera.range = 100;
	moveCamera(HALFCOMP, HALFCOMP + 1, HALFCOMP);
	rotateCamera(2.5, 1.0);


	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
			GLUT_ACTION_CONTINUE_EXECUTION);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	render.window = glutCreateWindow("Mmb " VERSION);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glutReshapeFunc(&onReshape);
	glutKeyboardFunc(&onKeyboard);
	glutMouseFunc(&onMouse);
	glutDisplayFunc(&onDisplay);
	glutIdleFunc(&onIdle);

	return &render;
}

void renderRun() {
	glutMainLoop();
}

void renderHookDraw(void (*func)(void *data), void *data) {
	render.onDraw = func;
	render.onDrawData = data;
}
