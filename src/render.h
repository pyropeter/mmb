
typedef struct {
	float x, y, z;
	int ix, iy, iz;
	float dx, dy, dz;
} Camera;

typedef struct {
	int window;
	int mouseX;
	int mouseY;
	int mouseSens;
	Camera camera;
	int (*onDraw)(void *data);
	void *onDrawData;
	int vertices;
} Render;

extern Render *renderInit(int argc, char *argv[]);
extern void renderRun();
extern void renderHookDraw(int (*func)(void *data), void *data);
