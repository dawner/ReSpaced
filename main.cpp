
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <string.h>
#include <math.h>

#include "define.h"
#include "G308_Geometry.h"
#include "SculptObject.h"
#include "ParticleSystem.h"
#include "CollisionSystem.h"
// #include "G308_ImageLoader.h"
#include <iostream>

using namespace std;

// Global Variables
GLuint g_mainWnd;
GLuint g_nWinWidth  = G308_WIN_WIDTH;
GLuint g_nWinHeight = G308_WIN_HEIGHT;
G308_Geometry* g_pGeometry = NULL;
const char** object_files;
int numObjects = 1;

bool rotating=false;
bool panning = false;
bool zooming = false;
bool rotation360 = false;
G308_Point clickDown;
G308_Point clickUp;
int mouseX;
int modifier_key;
AXIS selected_axis;

G308_Rotation cur_rotation;

float FPS = 60.0;
int lastUpdate = 0;

G308_Point camera_initial;
G308_Point camera_change;

//Camera variables.
G308_Point cam_position = {0.0, 0.0, 0.0};
int arc_x = 0;
int arc_y = 0;
bool rotate = false;
bool zoom = false;
bool pan = false;
int drag_x;
int drag_y;
float theta_x = 0.0;
float theta_y = 0.0;

float star_distance=30; //distance of stars from origin

void G308_Display() ;
void G308_Reshape(int w, int h) ;
void G308_SetCamera();
void G308_SetLight();
void G308_mouseListener(int button, int state, int x, int y);
void updateMouse(int, int);
void G308_keyboardListener(unsigned char, int, int);

G308_Object_Properties G308_Object_Details(const char*);
// void load_textures(G308_Object_Properties*);
int load_cubemap(char**);
void set_material(float property[], float, float, float, float);

//Sculpting variables.
bool sculpt_mode = false;
SculptObject *sculpt;
bool sculpting = false;
int tool = 1;
float sculpt_str = 0.001f;
float sculpt_dist = 5.0f;

void SculptingLight();
void colourMenu(int);

SculptObject *dirt_planet;
SculptObject *asteroid1;

// temp
SculptObject* sculptedModels[3];

//Particle System variables
ParticleSystem* particle_system = NULL;
int num_particles=3000;
int particles_per_frame=300;
bool sun_active=false;

// Collision System pointer
CollisionSystem* collision_system = NULL;

int main(int argc, char** argv)
{
	//assign static objects to be displayed
	// object_files = (char*) malloc(sizeof(char) * 20 * numObjects; 
	object_files = (const char**) malloc(sizeof(char*)*numObjects);
	for (int i = 0; i < numObjects;++i){
		object_files[i] = (const char*) malloc(20*sizeof(char)); //assuming filename length of 20 max
	}
	object_files[0]="Sphere.obj";

	modifier_key=-1;

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

	camera_initial.x = 10;
	camera_initial.y = 10;
	camera_initial.z = 32;
	camera_change.x = 0;
	camera_change.y = 0;
	camera_change.z = 0;
	

	g_pGeometry = new G308_Geometry(numObjects);

	//Sculpt stuff.
	sculpt = new SculptObject();
	sculpt->ReadOBJ("sculpt_sphere.obj");

	//Space objects.
	dirt_planet = new SculptObject();
	dirt_planet->ReadOBJ("dirt_planet.obj");
	dirt_planet->LoadTexture("dirt_planet.txt");

	asteroid1 = new SculptObject();
	asteroid1->ReadOBJ("asteroid1.obj");
	asteroid1->LoadTexture("asteroid1.txt");

	sculptedModels[0] = sculpt;
	sculptedModels[1] = dirt_planet;
	sculptedModels[2] = asteroid1;


	particle_system = new ParticleSystem(num_particles,star_distance);
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
	
	
	// char* filenames[6] = {"right.jpg","left.jpg","bot.jpg","top.jpg","front.jpg","back.jpg"};
	// int environment = load_cubemap(filenames);
	// g_pGeometry->SetEnvironment(environment); 
	// g_pGeometry->SetupSkyBox();
	for (int i = 0; i < numObjects;++i){
		G308_Object_Properties props = G308_Object_Details(object_files[i]);
		// load_textures(&props);
		g_pGeometry->ReadOBJ(i, object_files[i],props); // 1) read OBJ function
		g_pGeometry->CreateGLPolyGeometry(i); // 2) create GL Geometry as polygon
		g_pGeometry->CreateGLWireGeometry(i); // 3) create GL Geometry as wireframe
	}
	glDisable(GL_TEXTURE_2D);

	G308_SetLight();
	G308_SetCamera();
	glutMainLoop();

	if(g_pGeometry != NULL) delete g_pGeometry;

    return 0;
}

// int load_cubemap(char** filenames){
	
// 	GLuint texName;
// 	TextureInfo* t = (TextureInfo*) malloc(sizeof(TextureInfo)*6);

// 	GLenum cube[6] = {  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
// 	                    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
// 	                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
// 	                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
// 	                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
// 	                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

//     glEnable(GL_TEXTURE_CUBE_MAP);
//     glGenTextures(1, &texName);
//     glBindTexture(GL_TEXTURE_CUBE_MAP, texName);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
//     // Load Cube Map images
//     for(int j=0; j < 6; j++){
// 		unsigned int k;
// 		for (k = 0; k < strlen(filenames[j]); k++) {
// 			if (filenames[j][k] == '.') {
// 				break;
// 			}
// 		}
// 		char extension[5];
// 		strcpy(extension, &filenames[j][k + 1]);
// 		if (strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0)
// 			loadTextureFromJPEG(filenames[j], &(t[j]));
// 		else if (strcmp(extension, "png") == 0)
// 			loadTextureFromPNG(filenames[j], &(t[j]));
// 		else {
// 			printf("Invalid format. Only supports JPEG and PNG.\n");
// 			exit(1);
// 		}
//     	glTexImage2D(cube[j], 0, GL_RGB, t[j].width, t[j].height, 0, GL_RGB, GL_UNSIGNED_BYTE, t[j].textureData);

// 		free(t[j].textureData);
//     }
// 	return texName  ;
// }

// void load_textures(G308_Object_Properties* props){

// 	char* filename = props->texture_filename;
// 	if (filename==NULL)
// 		return; //no texture for this object
	
// 	GLuint texName;
// 	TextureInfo t;
// 	unsigned int i;
// 	for (i = 0; i < strlen(filename); i++) {
// 		if (filename[i] == '.') {
// 			break;
// 		}
// 	}
// 	char extension[5];
// 	strcpy(extension, &filename[i + 1]);

// 	if (strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0)
// 		loadTextureFromJPEG(filename, &t);
// 	else if (strcmp(extension, "png") == 0)
// 		loadTextureFromPNG(filename, &t);
// 	else {
// 		printf("Invalid format. Only supports JPEG and PNG.\n");
// 		exit(1);
// 	}

// 	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
// 	glGenTextures(1, &texName);
// 	glBindTexture(GL_TEXTURE_2D, texName);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

// 	//Only useful for PNG files, since JPEG doesn't support alpha
// 	if (t.hasAlpha) {
// 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.width, t.height, 0, GL_RGBA,
// 				GL_UNSIGNED_BYTE, t.textureData);
// 	} else {
// 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t.width, t.height, 0, GL_RGB,
// 				GL_UNSIGNED_BYTE, t.textureData);
// 	}
// 	//Once the texture has been loaded by GL, we don't need this anymore.
// 	free(t.textureData);

// 	props->texture=texName;
// 	props->has_alpha = t.hasAlpha;

// }


void set_material(float* property, float a, float b, float c, float d){
	property[0]=a;
	property[1]=b;
	property[2]=c;
	property[3]=d;
}

// Display function
void G308_Display()
{

	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	if ((currentTime-lastUpdate) > 1000/FPS){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
		glEnable( GL_BLEND );
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_NORMALIZE); //ensure lighting is uneffected by scaling

		glColor3f(1,1,0.8);

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

		//glRotatef(cur_rotation.x,0,1,0);

		// g_pGeometry->RenderGeometry(false);

		//Sculpting mode stuff goes in here.
		if (sculpt_mode) {
			sculpt->RenderGeometry(0);
		}
		//Everything else goes in here.
		else {

			G308_SetLight();

			//Removed predrawn sculpt objects in favor of having physics system draw them.

			//particle stuff
			glColor3f(0,0,0);
			for(int i=0;i<particles_per_frame;++i){
				particle_system->CreateParticle();
			}

			particle_system->display(theta_x,theta_y);
			collision_system->step();
		}
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);

		// g_pGeometry->SkyBox();

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_2D);

		glutSwapBuffers();
		glutPostRedisplay();
		if (rotation360){
			cur_rotation.x=cur_rotation.x+1;
			if (cur_rotation.x>360){
				cur_rotation.x=0;
				rotation360=false;
				modifier_key=-1;
			}
		}

		lastUpdate=currentTime;
	}
}

// Reshape function
void G308_Reshape(int w, int h)
{
    if (h == 0) h = 1;

	g_nWinWidth = w;
	g_nWinHeight = h;

    glViewport(0, 0, g_nWinWidth, g_nWinHeight);
}

void SculptingLight(){
	float point_pos[] = {0.0, 0.0, 1.0, 0.0f};
	float point_intensity[] = {0.8, 0.8, 0.8, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, point_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  point_intensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR,  point_intensity);

	glEnable(GL_LIGHT0);

	float ambient[] = {0.05f, 0.05f, 0.05f, 1.0f};
	glLightfv(GL_LIGHT3, GL_AMBIENT,  ambient);
	glEnable(GL_LIGHT3);
}

void colourMenu(int colour){
	if (colour == 0)
		sculpt->SetCurrentColour(0.9, 0.05, 0.05, sculpt->current_colour[3]);
	if (colour == 1)
		sculpt->SetCurrentColour(0.9, 0.5, 0.05, sculpt->current_colour[3]);
	if (colour == 2)
		sculpt->SetCurrentColour(0.9, 0.9, 0.05, sculpt->current_colour[3]);
	if (colour == 3)
		sculpt->SetCurrentColour(0.05, 0.6, 0.05, sculpt->current_colour[3]);
	if (colour == 4)
		sculpt->SetCurrentColour(0.25, 1.0, 0.25, sculpt->current_colour[3]);
	if (colour == 5)
		sculpt->SetCurrentColour(0.05, 0.05, 1.0, sculpt->current_colour[3]);
	if (colour == 6)
		sculpt->SetCurrentColour(0.1, 0.5, 1.0, sculpt->current_colour[3]);
	if (colour == 7)
		sculpt->SetCurrentColour(0.3, 0.2, 0.1, sculpt->current_colour[3]);
	if (colour == 8)
		sculpt->SetCurrentColour(0.45, 0.3, 0.15, sculpt->current_colour[3]);
	if (colour == 9)
		sculpt->SetCurrentColour(0.0, 0.0, 0.0, sculpt->current_colour[3]);
	if (colour == 10)
		sculpt->SetCurrentColour(1.0, 1.0, 1.0, sculpt->current_colour[3]);
}

// Set Light
void G308_SetLight()
{
	//--Point Light as Sun --//
	float point_pos[] = {0.0,0.0,0.0, 1.0f};
	float point_intensity[] = {1, 0.8, 0.2, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, point_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  point_intensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR,  point_intensity);

	glEnable(GL_LIGHT0);

	//--ambient light--//
	float ambient[] = {0.05f, 0.05f, 0.05f, 1.0f};
	glLightfv(GL_LIGHT3, GL_AMBIENT,  ambient);
	glEnable(GL_LIGHT3);
}


void G308_keyboardListener(unsigned char key, int x, int y) {
	//Code to respond to key events
	if (key==13){
		sculpt_mode = !sculpt_mode;
		if (sculpt_mode){
			particle_system->killAll();	
			glutAttachMenu(GLUT_RIGHT_BUTTON);
		}else	
			glutDetachMenu(GLUT_RIGHT_BUTTON);

	}

	if (sculpt_mode){
		//Toggle sculpting or painting.
		if (key == 't') {
			if (tool == 1)
				tool = 2;
			else
				tool = 1;
		}
		//Colour alpha up and down.
		else if (key == 'w'){
			sculpt->current_colour[3] = min(sculpt->current_colour[3] + 0.05f, 1.0f);
		}
		else if (key == 'q'){
			sculpt->current_colour[3] = max(sculpt->current_colour[3] - 0.05f, 0.0f);
		}
		//Fill object with current colour.
		else if (key == 'f'){
			sculpt->FillColour();
		}
		//Increase/decrease strength of geometry brush.
		else if(key == 's'){
			sculpt_str += 0.0002;
		}
		else if(key == 'a'){
			sculpt_str = max(sculpt_str - 0.0002f, 0.0f);
		}
		//Increase/decrease size of brush.
		else if(key == 'x'){
			sculpt_dist += 0.5;
		}
		else if(key == 'z'){
			sculpt_dist = max(sculpt_str - 0.5f, 0.0f);
		}
		else if(key == 'c'){
			char filename [80];
			printf("Enter obj name:\n");
			scanf("%s", filename);
			sculpt->SaveOBJ(filename);

			printf("Enter texture name:\n");
			scanf("%s", filename);
			sculpt->SaveTexture(filename);
		}
	}
}

void G308_mouseListener(int button, int state, int x, int y){

	rotating=false;
	panning=false;
	zooming=false;
	sculpting = false;
	clickDown.x=x;
	clickDown.y=y;
	if(glutGetModifiers()==GLUT_ACTIVE_SHIFT) {
		if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
			rotating=true;
		} 
		else if(button==GLUT_RIGHT_BUTTON && state==GLUT_DOWN ){
			panning=true;
		}else if(button==GLUT_MIDDLE_BUTTON && state==GLUT_DOWN){
			zooming=true;
		}
	}
	else {
		if (button==GLUT_LEFT_BUTTON && state==GLUT_DOWN && sculpt_mode) {
			sculpting = true;
		}
	}
	glutPostRedisplay();
}

void updateMouse(int x, int y){
	//Sculpting.
	if (sculpting){

		glPushMatrix();
			glTranslatef(cam_position.x, cam_position.y, cam_position.z);
			glRotatef(theta_x, 1.0, 0.0, 0.0);
			glRotatef(theta_y, 0.0, 1.0, 0.0);

			//Hold Alt to reverse direction.
			if (glutGetModifiers() == GLUT_ACTIVE_ALT)
				sculpt->MouseDrag(x, y, -sculpt_str, sculpt_dist, tool);
			else
				sculpt->MouseDrag(x, y, sculpt_str, sculpt_dist, tool);
			
		
		glPopMatrix();
	}

	//Camera control.
	if (rotating) {
		theta_y += 1.0*(x - clickDown.x)/2.0;
		theta_x += 1.0*(y - clickDown.y)/2.0;
	}
	else if (zooming) {
		cam_position.z += (float) (y - clickDown.y)/10.0;
	}
	else if (panning) {
		cam_position.x += (float) (x - clickDown.x)/20.0;
		cam_position.y -= (float) (y - clickDown.y)/20.0;
		//ensure camera doesn't pan past stars
		if (cam_position.x>(star_distance/3.0))
			cam_position.x=star_distance/3.0;
		if (cam_position.y>(star_distance/3.0))
			cam_position.y=star_distance/3.0;
		if (cam_position.x<(-1*star_distance/3.0))
			cam_position.x=-1*star_distance/3.0;
		if (cam_position.y<(-1*star_distance/3.0))
			cam_position.y=-1*star_distance/3.0;
	}
	glutPostRedisplay( );

	
	clickDown.x=x;
	clickDown.y=y;
}


// Set Camera Position
void G308_SetCamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(G308_FOVY, (double) g_nWinWidth / (double) g_nWinHeight, G308_ZNEAR_3D, G308_ZFAR_3D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	//gluLookAt(camera_initial.x+camera_change.x,camera_initial.y+camera_change.y, camera_initial.z+camera_change.z, -2.0, -1.5, 0.0, 0.0, 1.0, 0.0);
}

//----------------OBJECT PROPERITES-----------------//

// Returns the relevant translation depending on the given object filename
// to modify an objects position, the figures returned here would need to be changed
G308_Object_Properties G308_Object_Details(const char* object_name){
	G308_Object_Properties prop;
	prop.translation = (G308_Point*) malloc(sizeof(G308_Point));
	prop.rotation = (G308_Point*) malloc(sizeof(G308_Point));
	prop.scale = (G308_Point*) malloc(sizeof(G308_Point));
	prop.scale->x=1;
	prop.scale->y=1;
	prop.scale->z=1;
	//colours
	prop.colour = (G308_RGBA*) malloc(sizeof(G308_RGBA));
	prop.colour->r=1;
	prop.colour->g=1;
	prop.colour->b=1;
	prop.colour->a=1;

	prop.texture_filename=NULL;
	prop.texture=0;
	prop.texture_multiplier=1;

	if (strcmp(object_name,"Table.obj")==0){
		prop.translation->x=0;
		prop.translation->y=-2;
		prop.translation->z=5;
		prop.scale->x=1.2;
		prop.scale->z=1.2;
		prop.texture_filename ="wood.jpg";
		prop.texture_multiplier=12;
		prop.is_metallic=false;
	}else if (strcmp(object_name,"Sphere.obj")==0){
		prop.translation->x=-5;
		prop.translation->y=0;
		prop.translation->z=7;
		prop.colour->r=1.0;
		prop.colour->g=0.6; 
		prop.colour->b=0.1;
		prop.scale->x=1.2;
		prop.scale->x=1.2;
		prop.scale->z=1.2;
		prop.diffuse = (float*) malloc(sizeof(float)*4);
		prop.specular = (float*) malloc(sizeof(float)*4);
		prop.shininess = (float*) malloc(sizeof(float));
		set_material(prop.diffuse, 0.7, 0.4, 0.2, 1.0);
		set_material(prop.specular, 1,1,1,0.5);
		prop.shininess[0]= 30;
		prop.is_metallic=true;
	}else if (strcmp(object_name,"Torus.obj")==0){
		prop.translation->x=2;
		prop.translation->y=-1;
		prop.translation->z=9;
		prop.colour->r=1;
		prop.colour->g=0; 
		prop.colour->b=0;
		prop.diffuse = (float*) malloc(sizeof(float)*4);
		prop.ambient = (float*) malloc(sizeof(float)*4);
		prop.specular = (float*) malloc(sizeof(float)*4);
		prop.shininess = (float*) malloc(sizeof(float));
		set_material(prop.ambient, 0.0, 0.0, 0.0, 1.0);
		set_material(prop.diffuse, 0.5, 0.0,0.0, 1.0);
		set_material(prop.specular, 1.0,1.0,1.0 ,1.0);
		prop.shininess[0]= 128;
		prop.is_metallic=false;
	}else if (strcmp(object_name,"Bunny.obj")==0){
		prop.colour->a=0.75;
		prop.translation->x=-2;
		prop.translation->y=-1.5;
		prop.translation->z=1;
		prop.diffuse = (float*) malloc(sizeof(float)*4);
		prop.specular = (float*) malloc(sizeof(float)*4);
		prop.shininess = (float*) malloc(sizeof(float));
		set_material(prop.diffuse, 0.5,0.4,0.4, 1.0);
		set_material(prop.specular, 0.5,0.5,0.5,0.5);
		prop.shininess[0]= 20;
		prop.is_metallic=false;
	}else if (strcmp(object_name,"Teapot.obj")==0){
		prop.translation->x=-7;
		prop.translation->y=-1.5;
		prop.translation->z=-3;
		prop.colour->r=0.11;
		prop.colour->g=0.22; 
		prop.colour->b=0.72;
		prop.scale->x=0.9;
		prop.scale->y=0.9;
		prop.scale->z=0.9;
		prop.diffuse = (float*) malloc(sizeof(float)*4);
		prop.specular = (float*) malloc(sizeof(float)*4);
		prop.shininess = (float*) malloc(sizeof(float));
		set_material(prop.diffuse, 0.5, 0.5,0.5, 1.0);
		set_material(prop.specular, 1,1,1,1.0);
		prop.shininess[0]= 100;
		prop.is_metallic=true;
	}else if (strcmp(object_name,"Box.obj")==0){
		prop.translation->x=4;
		prop.translation->y=0;
		prop.translation->z=-1.5;
		prop.texture_multiplier=5;
		prop.texture_filename ="brick.jpg";
		prop.is_metallic=false;
	}
	return prop;
}










