/*******************************************************************
		   Multi-Part Model Construction and Manipulation
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <stdlib.h>
#include <malloc.h>
#include "Vector3D.h"
#include "QuadMesh.h"
#include <time.h>		  //
const int meshSize = 16;  // Default Mesh Size
const int vWidth = 2560;  // Viewport width in pixels
const int vHeight = 1375; // Viewport height in pixels

static int currentButton;
static unsigned char currentKey;

// Lighting/shading and material properties for drone - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = {-6.0F, 12.0F, 0.0F, 1.0F};
static GLfloat light_position1[] = {6.0F, 12.0F, 0.0F, 1.0F};
static GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
static GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
static GLfloat light_ambient[] = {0.2F, 0.2F, 0.2F, 1.0F};

// Material properties
static GLfloat drone_mat_ambient[] = {0.4F, 0.2F, 0.0F, 1.0F};
static GLfloat drone_mat_specular[] = {0.1F, 0.1F, 0.0F, 1.0F};
static GLfloat drone_mat_diffuse[] = {0.9F, 0.5F, 0.0F, 1.0F};
static GLfloat drone_mat_shininess[] = {0.0F};

static GLfloat PAVEMENT_ambient[] = {0.5F, 0.5F, 0.5F, 1.0F};
static GLfloat PAVEMENT_specular[] = {0.0f, 0.0f, 0.0f, 1.0f};
static GLfloat PAVEMENT_diffuse[] = {0.0f, 0.0f, 0.0f, 0.0f};
static GLfloat PAVEMENT_shininess[] = {0.0F};

static GLfloat PARKINGLINES_ambient[] = {1.0F, 1.0F, 0.0F, 1.0F};
static GLfloat PARKINGLINES_specular[] = {0.0F, 0.0F, 0.0F, 1.0F};
static GLfloat PARKINGLINES_diffuse[] = {0.0F, 0.0F, 0.0F, 0.0F};
static GLfloat PARKINGLINES_shininess[] = {1.0F};

static GLfloat CARS_ambient[] = {0.0F, 0.0F, 1.0F, 1.0F};
static GLfloat CARS_specular[] = {0.1F, 0.1F, 0.1F, 0.1F};
static GLfloat CARS_diffuse[] = {0.2F, 0.2F, 0.2F, 1.0F};
static GLfloat CARS_shininess[] = {0.0F};

// A quad mesh representing the ground
static QuadMesh groundMesh;

int xOrigin = -1;
int yOrigin = -1;

double theta = 0.0f;
double thetay = 0.0f;

float deltaAngle = 0.0f;
float deltaAngley = 0.0f;

float eyex = 0.0f;
double eyey = 25.0f;
float eyez = 5.0f;

float temp = 0.0;
float tempy = 0.0;

double d = 8;
float acc = -9.8;
//float vel = 0.0f;
//float last_vel = 0.0f;

typedef struct obj
{
	float x;
	float y;
	float z;
	float rad;
	float sl;
	float st;
	float mass;
	bool airborn;
	float vel;
	float last_vel;
	clock_t start_t;
	clock_t end_t;
	clock_t total_t;
	float angleX;
	float angleZ;
};

void jump(struct obj *planet);
// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboardd(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);

void keyOperations(void);
void keyPressed(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void drawEarth(struct obj *planet);
void drawMoon(struct obj *planet);

Vector3D location;
Vector3D ScreenToWorld(int x, int y);

struct obj Earth = {0, 0, 0, 5, 50, 50, 5, false, 0};
struct obj Moon = {10, 0, 0, 3, 50, 50, 2, false, 0};

//bool* keyStates = new bool[256]; // Create an array of boolean values of length 256 (0-255)
bool keyStates[256] = {false};
bool modes[12] = {false};

void spinner(void)
{
	keyOperations();
}
int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Physics engine");

	////

	//Earth.x = 0;
	//Earth.y = 0;
	//Earth.z = 0;
	//Earth.rad = 5;
	//Earth.sl = 50;
	//Earth.st = 50;
	//Earth.mass = 5;

	///Moon.x = 10;
	//Moon.y = 0;
	//Moon.z = 0;
	////Moon.rad = 3;
	///Moon.sl = 50;
	//Moon.st = 50;
	//Moon.mass = 2;

	////
	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyPressed);
	//glutKeyboardFunc(keyboardd);// Tell GLUT to use the method "keyPressed" for key presses
	glutKeyboardUpFunc(keyUp); // Tell GLUT to use the method "keyUp" for key up events
	glutSpecialFunc(functionKeys);
	glutIdleFunc(spinner);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}

// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);			  // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);			  // Use smooth shading, makes boundaries between polygons harder to see
	glClearColor(0.6F, 0.6F, 0.6F, 0.0F); // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);							   // Renormalize normal vectors
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // Nicer perspective

	// Set up ground quad mesh
	Vector3D origin = NewVector3D(-60.0f, 0.0f, 60.0f);
	Vector3D dir1v = NewVector3D(8.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -8.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh, meshSize, origin, 16.0, 16.0, dir1v, dir2v);

	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	keyOperations();
	glLoadIdentity();

	gluLookAt(d * eyex, eyey, d * eyez, cos(theta), 6, -sin(theta), 0, 1, 0);
	// Set drone material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);

	drawEarth(&Earth);
	drawMoon(&Moon);

	//jump(&Earth);
	//jump(&Moon);

	//glRotatef(-yAngle, 0.0f, 1.0, 0.0f);
	//glRotatef(-xAngle, 1.0f, 0.0f, 0.0f);
	//glTranslatef(-position.x, -position.y, -position.z);

	DrawMeshQM(&groundMesh, meshSize);

	glutSwapBuffers(); // Double buffering, swap buffers
}

void drawEarth(struct obj *planet)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);

	glPushMatrix();
	glTranslatef(planet->x, planet->y, planet->z);
	glTranslatef(planet->x, planet->rad, planet->z);
	glRotatef(Earth.angleX, 1, 0, 0);
	glRotatef(Earth.angleZ, 0, 0, 1);

	glutWireSphere(planet->rad, planet->sl, planet->st);
	glPopMatrix();
	jump(&Earth);
}

void drawMoon(struct obj *planet)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);

	glPushMatrix();
	glTranslatef(planet->x, planet->y, planet->z);
	glTranslatef(planet->x, planet->rad, planet->z);
	glutWireSphere(planet->rad, planet->sl, planet->st);
	glPopMatrix();
	jump(&Moon);
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode -
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (GLdouble)w / h, 0.3, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboardd(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	{
		break;
	}
	case 's':
	{

		break;
	}
	case 'a':
	{
		break;
	}
	case 'd':
	{
		break;
	}
	case 't':
	{
		//start_t = clock();

		//vel = 500.0f;
		//last_vel = vel;
		//jump( obj Earth);
		//jump(Moon);

		break;
	}
		if (key == 27)
		{
			exit(0);
		}
	}
	glutPostRedisplay(); // Trigger a window redisplay
}

void jump(struct obj *planet)
{
	//float future =
	planet->end_t = clock();

	//printf("%f \n", planet->y);

	planet->total_t = planet->end_t - planet->start_t;
	if (planet->y < 0 && planet->vel < 0)
	{

		printf("%f \n", planet->last_vel);
		//printf("%f \n", Moon.last_vel);
		printf("\n");

		planet->vel = planet->last_vel;

		//planet->last_vel -= ((double)planet->last_vel + (double)planet->mass)  * 0.25;
		planet->last_vel -= ((double)planet->last_vel + (double)planet->mass) * 0.15;
	}

	if (planet->y < 0)
	{

		planet->y = 0;
		//vel *= planet->mass;
		planet->start_t = planet->end_t;

		planet->airborn = false;
	}
	if (planet->airborn || planet->last_vel > 0)
	{
		planet->vel += acc * planet->total_t / 1000 * 0.01;

		planet->y += planet->vel * planet->total_t / 1000 * 0.01 * (1 / planet->mass);
	}
}
void keyOperations(void)
{
	if (keyStates['q'])
	{
		exit(0);
	}

	if (keyStates['w'])
	{
		//Earth.x += 0.001;
		Earth.angleX -= 0.01;
		Earth.x -= sin(3.14f * Earth.angleZ / 180.0f) * 0.001f;
		Earth.z -= cos(3.14f * Earth.angleZ / 180.0f) * 0.001f;
	}
	if (keyStates['s'])
	{
		//Earth.x += 0.001;
		Earth.angleX += 0.01;
		Earth.x += sin(3.14f * Earth.angleZ / 180.0f) * 0.001f;
		Earth.z += cos(3.14f * Earth.angleZ / 180.0f) * 0.001f;
	}
	if (keyStates['a'])
	{
		//Earth.x += 0.001;
		Earth.angleZ -= 0.01;
		//Earth.x -= sin(3.14f * 1 / 180.0f) * 0.001f;
		//Earth.z += cos(3.14f * 1 / 180.0f) * 0.001f;
	}
	if (keyStates['d'])
	{
		//Earth.x += 0.001;
		Earth.angleZ += 0.01;
		//Earth.x += sin(3.14f * 1 / 180.0f) * 0.001f;
		//Earth.z -= cos(3.14f * 1 / 180.0f) * 0.001f;
	}
	if (keyStates[' '] && !Earth.airborn && !Moon.airborn)
	{
		Earth.start_t = clock();
		Moon.start_t = clock();

		Earth.airborn = true;
		Moon.airborn = true;

		Earth.vel = 20.0f;
		Earth.last_vel = Earth.vel;

		Moon.vel = 30.0f;
		Moon.last_vel = Moon.vel;

		//jump(&Moon);
	}
	glutPostRedisplay();
}

void keyPressed(unsigned char key, int x, int y)
{
	keyStates[key] = true; // Set the state of the current key to pressed
}

void keyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false; // Set the state of the current key to not pressed
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_UP)
	{
	}
	if (key == GLUT_KEY_DOWN)
	{
	}
	if (key == GLUT_KEY_RIGHT)
	{
	}
	if (key == GLUT_KEY_LEFT)
	{
	}
	// Do transformations with arrow keys
	//else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	//{
	//}

	glutPostRedisplay(); // Trigger a window redisplay
}

// Mouse button callback - use only if you want to
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{

		// when the button is released
		if (state == GLUT_UP)
		{
			theta += deltaAngle;
			xOrigin = -1;
		}
		else
		{ // state = GLUT_DOWN
			xOrigin = x;
			//yOrigin = y;
		}
	}
	if (button == GLUT_RIGHT_BUTTON)
	{

		// when the button is released
		if (state == GLUT_UP)
		{
			thetay += deltaAngley;
			yOrigin = -1;
		}
		else
		{
			yOrigin = y;
		}
	}
	//zoom out
	if (button == 3)
	{
		if (d > 0)
		{
			d -= 0.1;
			reshape(vWidth, vHeight);
		}
	}
	//zoom in
	if (button == 4)
	{
		if (d < 15)
		{
			d += 0.1;
			reshape(vWidth, vHeight);
		}
	}

	glutPostRedisplay(); // Trigger a window redisplay
}

void mode(int selector)
{
}
// Mouse motion callback - use only if you want to
void mouseMotionHandler(int xMouse, int yMouse)
{
	Vector3D location = ScreenToWorld(xMouse, yMouse);
	//changeY(&groundMesh, location.x, location.z, 1.0, 1.0);

	// this will only be true when the left button is down
	if (xOrigin >= 0)
	{

		// update deltaAngle
		temp = deltaAngle;
		deltaAngle = (xMouse - xOrigin) * 0.0001f;

		if ((temp - deltaAngle) < 0)
		{
			theta -= 0.028;
		}
		else
		{
			theta += 0.028;
		}
		eyex = sin(theta + deltaAngle) * 5;
		eyez = cos(theta + deltaAngle) * 5;
	}
	if (yOrigin >= 0)
	{

		tempy = deltaAngley;
		deltaAngley = (yMouse - yOrigin) * 0.00001f;
		if ((tempy - deltaAngley) > 0 && eyey > -1)
		{
			eyey -= 0.5;
		}
		if ((tempy - deltaAngley) < 0 && eyey < 25.0)
		{
			eyey += 0.5;
		}
	}
	glutPostRedisplay(); // Trigger a window redisplay
}

Vector3D ScreenToWorld(int x, int y)
{

	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = x;
	winY = (float)viewport[3] - y;
	glReadPixels(x, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return NewVector3D(posX, posY, posZ);
}
