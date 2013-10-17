#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <string.h>
#include <math.h>

#include "define.h"
#include "SculptObject.h"
#include "ParticleSystem.h"
#include "CollisionSystem.h"
// #include "G308_ImageLoader.h"
#include <iostream>

using namespace std;

// Global Variables
GLuint g_mainWnd;
GLuint g_nWinWidth = G308_WIN_WIDTH;
GLuint g_nWinHeight = G308_WIN_HEIGHT;

bool rotating = false;
bool panning = false;
bool zooming = false;

G308_Point clickDown;
G308_Point clickUp;

float FPS = 60.0;
int lastUpdate = 0;

//Camera variables.
G308_Point cam_position = { 0.0, 0.0, 0.0 };
int arc_x = 0;
int arc_y = 0;
bool rotate = false;
bool zoom = false;
bool pan = false;
int drag_x;
int drag_y;
float theta_x = 0.0;
float theta_y = 0.0;

float star_distance = 900; //distance of stars from origin

void G308_Display();
void G308_Reshape(int w, int h);
void G308_SetCamera();
void G308_SetLight();
void G308_mouseListener(int button, int state, int x, int y);
void updateMouse(int, int);
void G308_keyboardListener(unsigned char, int, int);

//Sculpting variables.
bool sculpt_mode = false;
int sculpt = 0;
bool sculpting = false;
int tool = 1;
float sculpt_str = 0.001f;
float sculpt_dist = 5.0f;
bool ctrl_pressed = false;

void SculptingLight();
void colourMenu(int);

SculptObject* sculptedModels[7];

//Particle System variables
ParticleSystem* particle_system = NULL;
int num_particles = 3000;
int particles_per_frame = 300;
bool sun_active = false;

// Collision System pointer
CollisionSystem* collision_system = NULL;

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(g_nWinWidth, g_nWinHeight);
	g_mainWnd = glutCreateWindow("COMP308 Assignment4");

	glutDisplayFunc(G308_Display);
	glutIdleFunc(G308_Display);
	glutReshapeFunc(G308_Reshape);
	glutMouseFunc(G308_mouseListener);
	glutMotionFunc(updateMouse);
	glutKeyboardFunc(G308_keyboardListener);

	//Sculpt stuff.
	for (int i = 0; i < 7; i++) {
		sculptedModels[i] = new SculptObject();
	}
	sculptedModels[0]->ReadOBJ("sculpt_sphere.obj");

	sculptedModels[1]->ReadOBJ("asteroid1.obj");
	sculptedModels[1]->LoadTexture("asteroid1.txt");

	sculptedModels[2]->ReadOBJ("asteroid2.obj");
	sculptedModels[2]->LoadTexture("asteroid2.txt");

	sculptedModels[3]->ReadOBJ("asteroid3.obj");
	sculptedModels[3]->LoadTexture("asteroid3.txt");

	sculptedModels[4]->ReadOBJ("dirt_planet.obj");
	sculptedModels[4]->LoadTexture("dirt_planet.txt");

	sculptedModels[5]->ReadOBJ("blue_planet.obj");
	sculptedModels[5]->LoadTexture("blue_planet.txt");

	sculptedModels[6]->ReadOBJ("earthish.obj");
	sculptedModels[6]->LoadTexture("earthish.txt");

	particle_system = new ParticleSystem(num_particles, star_distance);
	collision_system = new CollisionSystem(sculptedModels);
	glutCreateMenu(colourMenu);
	glutAddMenuEntry("Red", 0);
	glutAddMenuEntry("Orange", 1);
	glutAddMenuEntry("Yellow", 2);
	glutAddMenuEntry("Green", 3);
	glutAddMenuEntry("Light Green", 4);
	glutAddMenuEntry("Blue", 5);
	glutAddMenuEntry("Light Blue", 6);
	glutAddMenuEntry("Dark Brown", 7);
	glutAddMenuEntry("Light Brown", 8);
	glutAddMenuEntry("Black", 9);
	glutAddMenuEntry("White", 10);

	glDisable(GL_TEXTURE_2D);

	G308_SetLight();
	G308_SetCamera();
	glutMainLoop();

	return 0;
}

// Display function
void G308_Display() {
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	if ((currentTime - lastUpdate) > 1000 / FPS) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_NORMALIZE); //ensure lighting is uneffected by scaling

		glColor3f(1, 1, 0.8);

		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		G308_SetCamera();
		glPushMatrix();

		if (sculpt_mode)
			SculptingLight();

		//Camera changes.
		glTranslatef(cam_position.x, cam_position.y, cam_position.z);
		glRotatef(theta_x, 1.0, 0.0, 0.0);
		glRotatef(theta_y, 0.0, 1.0, 0.0);

		//Sculpting mode stuff goes in here.
		if (sculpt_mode) {
			sculptedModels[sculpt]->RenderGeometry(0);
		}
		//Everything else goes in here.
		else {

			G308_SetLight();

			collision_system->step();
			//particle stuff drawn after to allow for alpha
			glColor3f(0, 0, 0);
			for (int i = 0; i < particles_per_frame; ++i) {
				particle_system->CreateParticle();
			}
			particle_system->display(theta_x, theta_y);
		}
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_2D);

		glutSwapBuffers();
		glutPostRedisplay();

		lastUpdate = currentTime;
	}
}

// Reshape function
void G308_Reshape(int w, int h) {
	if (h == 0)
		h = 1;

	g_nWinWidth = w;
	g_nWinHeight = h;

	glViewport(0, 0, g_nWinWidth, g_nWinHeight);
}

void SculptingLight() {
	float point_pos[] = { 0.0, 0.0, 1.0, 0.0f };
	float point_intensity[] = { 0.8, 0.8, 0.8, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, point_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, point_intensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR, point_intensity);

	glEnable(GL_LIGHT0);

	float ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
	glLightfv(GL_LIGHT3, GL_AMBIENT, ambient);
	glEnable(GL_LIGHT3);
}

void colourMenu(int colour) {
	if (colour == 0)
		sculptedModels[sculpt]->SetCurrentColour(0.9, 0.05, 0.05,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 1)
		sculptedModels[sculpt]->SetCurrentColour(0.9, 0.5, 0.05,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 2)
		sculptedModels[sculpt]->SetCurrentColour(0.9, 0.9, 0.05,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 3)
		sculptedModels[sculpt]->SetCurrentColour(0.05, 0.6, 0.05,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 4)
		sculptedModels[sculpt]->SetCurrentColour(0.25, 1.0, 0.25,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 5)
		sculptedModels[sculpt]->SetCurrentColour(0.05, 0.05, 1.0,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 6)
		sculptedModels[sculpt]->SetCurrentColour(0.1, 0.5, 1.0,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 7)
		sculptedModels[sculpt]->SetCurrentColour(0.3, 0.2, 0.1,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 8)
		sculptedModels[sculpt]->SetCurrentColour(0.45, 0.3, 0.15,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 9)
		sculptedModels[sculpt]->SetCurrentColour(0.0, 0.0, 0.0,
				sculptedModels[sculpt]->current_colour[3]);
	if (colour == 10)
		sculptedModels[sculpt]->SetCurrentColour(1.0, 1.0, 1.0,
				sculptedModels[sculpt]->current_colour[3]);
}

// Set Light
void G308_SetLight() {
	//--Point Light as Sun --//
	float point_pos[] = { 0.0, 0.0, 0.0, 1.0f };
	float point_intensity[] = { 1, 0.8, 0.2, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, point_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, point_intensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR, point_intensity);

	glEnable(GL_LIGHT0);

	//--ambient light--//
	float ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
	glLightfv(GL_LIGHT3, GL_AMBIENT, ambient);
	glEnable(GL_LIGHT3);
}

void G308_keyboardListener(unsigned char key, int x, int y) {
	//Code to respond to key events
	if (key == 13) {
		sculpt_mode = !sculpt_mode;
		if (sculpt_mode) {
			particle_system->killAll();
			glutAttachMenu(GLUT_RIGHT_BUTTON);
		} else {
			collision_system->updateAll();
			glutDetachMenu(GLUT_RIGHT_BUTTON);
		}

	}

	if (sculpt_mode) {
		//Toggle sculpting or painting.
		if (key == 't') {
			if (tool == 1)
				tool = 2;
			else
				tool = 1;
		}
		//Colour alpha up and down.
		else if (key == 'w') {
			sculptedModels[sculpt]->current_colour[3] = min(
					sculptedModels[sculpt]->current_colour[3] + 0.05f, 1.0f);
			printf("Alpha: %f\n", sculptedModels[sculpt]->current_colour[3]);
		} else if (key == 'q') {
			sculptedModels[sculpt]->current_colour[3] = max(
					sculptedModels[sculpt]->current_colour[3] - 0.05f, 0.0f);
			printf("Alpha: %f\n", sculptedModels[sculpt]->current_colour[3]);
		}
		//Fill object with current colour.
		else if (key == 'f') {
			sculptedModels[sculpt]->FillColour();
		}
		//Increase/decrease strength of geometry brush.
		else if (key == 's') {
			sculpt_str += 0.0002f;
			printf("Brush strength: %f\n", sculpt_str);
		} else if (key == 'a') {
			sculpt_str = max(sculpt_str - 0.0002f, 0.0f);
			printf("Brush strength: %f\n", sculpt_str);
		}
		//Increase/decrease size of brush.
		else if (key == 'x') {
			sculpt_dist += 0.5f;
			printf("Brush size: %f\n", sculpt_dist);
		} else if (key == 'z') {
			sculpt_dist = max(sculpt_dist - 0.5f, 0.0f);
			printf("Brush size: %f\n", sculpt_dist);
		} else if (key == 'c') {
			char filename[80];
			printf("Enter obj name:\n");
			scanf("%s", filename);
			sculptedModels[sculpt]->SaveOBJ(filename);

			printf("Enter texture name:\n");
			scanf("%s", filename);
			sculptedModels[sculpt]->SaveTexture(filename);
		} else if (key == 'e') {
			sculpt = (sculpt + 1) % 7;
		}
	}
}

void G308_mouseListener(int button, int state, int x, int y) {
	ctrl_pressed = (glutGetModifiers() == GLUT_ACTIVE_CTRL);
	rotating = false;
	panning = false;
	zooming = false;
	sculpting = false;
	clickDown.x = x;
	clickDown.y = y;
	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
			rotating = true;
		} else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
			panning = true;
		} else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
			zooming = true;
		}
	} else {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && sculpt_mode) {
			sculpting = true;
		}
	}
	glutPostRedisplay();
}

void updateMouse(int x, int y) {
	//Sculpting.
	if (sculpting) {

		glPushMatrix();
		glTranslatef(cam_position.x, cam_position.y, cam_position.z);
		glRotatef(theta_x, 1.0, 0.0, 0.0);
		glRotatef(theta_y, 0.0, 1.0, 0.0);

		//Hold Alt to reverse direction.
		if (ctrl_pressed)
			sculptedModels[sculpt]->MouseDrag(x, y, -sculpt_str, sculpt_dist,
					tool);
		else
			sculptedModels[sculpt]->MouseDrag(x, y, sculpt_str, sculpt_dist,
					tool);

		glPopMatrix();
	}

	//Camera control.
	if (rotating) {
		theta_y += 1.0 * (x - clickDown.x) / 2.0;
		theta_x += 1.0 * (y - clickDown.y) / 2.0;
	} else if (zooming) {
		cam_position.z += (float) (y - clickDown.y) / 10.0;
	} else if (panning) {
		cam_position.x += (float) (x - clickDown.x) / 20.0;
		cam_position.y -= (float) (y - clickDown.y) / 20.0;
		//ensure camera doesn't pan past stars
		if (cam_position.x > (star_distance / 3.0))
			cam_position.x = star_distance / 3.0;
		if (cam_position.y > (star_distance / 3.0))
			cam_position.y = star_distance / 3.0;
		if (cam_position.x < (-1 * star_distance / 3.0))
			cam_position.x = -1 * star_distance / 3.0;
		if (cam_position.y < (-1 * star_distance / 3.0))
			cam_position.y = -1 * star_distance / 3.0;
	}
	glutPostRedisplay();

	clickDown.x = x;
	clickDown.y = y;
}

// Set Camera Position
void G308_SetCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(G308_FOVY, (double) g_nWinWidth / (double) g_nWinHeight,
			G308_ZNEAR_3D, G308_ZFAR_3D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

