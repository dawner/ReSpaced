
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <string.h>
#include <math.h>

#include "define.h"
#include "G308_Geometry.h"
#include "G308_ImageLoader.h"
#include <iostream>

using namespace std;

// Global Variables
GLuint g_mainWnd;
GLuint g_nWinWidth  = G308_WIN_WIDTH;
GLuint g_nWinHeight = G308_WIN_HEIGHT;
G308_Geometry* g_pGeometry = NULL;
const char** object_files;
int numObjects = 6; 

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
G308_Spotlight spotlight;

void G308_Display() ;
void G308_Reshape(int w, int h) ;
void G308_SetCamera();
void G308_SetLight();
void G308_mouseListener(int button, int state, int x, int y);
void updateMouse(int, int);
void G308_keyboardListener(unsigned char, int, int);
void G308_arrow_keys(int key, int x, int y);

G308_Object_Properties G308_Object_Details(const char*);
void load_textures(G308_Object_Properties*);
int load_cubemap(char**);
G308_Point normalise(G308_Point);
G308_Point crossProduct(G308_Point p, G308_Point q);
float dotProduct(G308_Point p, G308_Point q);
void set_material(float property[], float, float, float, float);
void drawAxis(GLUquadric* q);
void change_axis();


int main(int argc, char** argv)
{
	//assign static objects to be displayed
	// object_files = (char*) malloc(sizeof(char) * 20 * numObjects; 
	object_files = (const char**) malloc(sizeof(char*)*numObjects);
	for (int i = 0; i < numObjects;++i){
		object_files[i] = (const char*) malloc(20*sizeof(char)); //assuming filename length of 20 max
	}
	object_files[0]="Sphere.obj";
	object_files[1]="Torus.obj";
	object_files[2]="Teapot.obj";
	object_files[3]="Box.obj";
	object_files[4]="Table.obj";
	object_files[5]="Bunny.obj";

	modifier_key=-1;

	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(g_nWinWidth, g_nWinHeight);
    g_mainWnd = glutCreateWindow("COMP308 Assignment4");

    glutDisplayFunc(G308_Display);
    glutReshapeFunc(G308_Reshape);
    glutMouseFunc(G308_mouseListener);
	glutMotionFunc(updateMouse);
	glutKeyboardFunc(G308_keyboardListener);
	glutSpecialFunc(G308_arrow_keys); 

	spotlight.direction.x = -0.05;
	spotlight.direction.y = 0.02;
	spotlight.direction.z = -0.27;
	spotlight.position.x = 2;
	spotlight.position.y = 0.5;
	spotlight.position.z = -20;
	spotlight.cutoff=15.0;
	spotlight.exponent=2.0;
	spotlight.colour.r=0.4;
	spotlight.colour.g=0.4;
	spotlight.colour.b=0.4;

	G308_SetLight();
	camera_initial.x = 10;
	camera_initial.y = 10;
	camera_initial.z = 32;
	camera_change.x = 0;
	camera_change.y = 0;
	camera_change.z = 0;
	G308_SetCamera();

	g_pGeometry = new G308_Geometry(numObjects);
	
	char* filenames[6] = {"right.jpg","left.jpg","bot.jpg","top.jpg","front.jpg","back.jpg"};
	int environment = load_cubemap(filenames);
	g_pGeometry->SetEnvironment(environment); 
	g_pGeometry->SetupSkyBox();
	for (int i = 0; i < numObjects;++i){
		G308_Object_Properties props = G308_Object_Details(object_files[i]);
		load_textures(&props);
		g_pGeometry->ReadOBJ(i, object_files[i],props); // 1) read OBJ function
		g_pGeometry->CreateGLPolyGeometry(i); // 2) create GL Geometry as polygon
		g_pGeometry->CreateGLWireGeometry(i); // 3) create GL Geometry as wireframe
	}
	glDisable(GL_TEXTURE_2D);

	glutMainLoop();

	if(g_pGeometry != NULL) delete g_pGeometry;

    return 0;
}

int load_cubemap(char** filenames){
	
	GLuint texName;
	TextureInfo* t = (TextureInfo*) malloc(sizeof(TextureInfo)*6);

	GLenum cube[6] = {  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	                    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

    glEnable(GL_TEXTURE_CUBE_MAP);
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texName);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
    // Load Cube Map images
    for(int j=0; j < 6; j++){
		unsigned int k;
		for (k = 0; k < strlen(filenames[j]); k++) {
			if (filenames[j][k] == '.') {
				break;
			}
		}
		char extension[5];
		strcpy(extension, &filenames[j][k + 1]);
		if (strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0)
			loadTextureFromJPEG(filenames[j], &(t[j]));
		else if (strcmp(extension, "png") == 0)
			loadTextureFromPNG(filenames[j], &(t[j]));
		else {
			printf("Invalid format. Only supports JPEG and PNG.\n");
			exit(1);
		}
    	glTexImage2D(cube[j], 0, GL_RGB, t[j].width, t[j].height, 0, GL_RGB, GL_UNSIGNED_BYTE, t[j].textureData);

		free(t[j].textureData);
    }
	return texName  ;
}

void load_textures(G308_Object_Properties* props){

	char* filename = props->texture_filename;
	if (filename==NULL)
		return; //no texture for this object
	
	GLuint texName;
	TextureInfo t;
	unsigned int i;
	for (i = 0; i < strlen(filename); i++) {
		if (filename[i] == '.') {
			break;
		}
	}
	char extension[5];
	strcpy(extension, &filename[i + 1]);

	if (strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0)
		loadTextureFromJPEG(filename, &t);
	else if (strcmp(extension, "png") == 0)
		loadTextureFromPNG(filename, &t);
	else {
		printf("Invalid format. Only supports JPEG and PNG.\n");
		exit(1);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Only useful for PNG files, since JPEG doesn't support alpha
	if (t.hasAlpha) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.width, t.height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, t.textureData);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t.width, t.height, 0, GL_RGB,
				GL_UNSIGNED_BYTE, t.textureData);
	}
	//Once the texture has been loaded by GL, we don't need this anymore.
	free(t.textureData);

	props->texture=texName;
	props->has_alpha = t.hasAlpha;

}


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

		glColor3f(0,0,0);

		glLoadIdentity();
		G308_SetLight();
		G308_SetCamera();

		glMatrixMode(GL_MODELVIEW);
		glRotatef(cur_rotation.x,0,1,0);

		g_pGeometry->RenderGeometry(false);
		glRotatef(-cur_rotation.x,0,1,0);

		glMatrixMode(GL_MODELVIEW);

		g_pGeometry->SkyBox();

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

// Set Light
void G308_SetLight()
{
	glTranslatef(-camera_change.x,-camera_change.y,-camera_change.z);

	//--Point Light--//
	float point_pos[] = {0.0f, 0.5f, 0.5f, 1.0f};
	float point_intensity[] = {0.1f, 0.1f, 0.1f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, point_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  point_intensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR,  point_intensity);

	glEnable(GL_LIGHT0);


	//--Direction Light--//
	float direction[]	  = {0.5f, 1.0f, 1.0f, 0.0f};
	float dir_intensity[] = {0.2f, 0.2f, 0.2f, 1.0f};

	glLightfv(GL_LIGHT1, GL_POSITION, direction);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  point_intensity);
	glLightfv(GL_LIGHT1, GL_SPECULAR,  dir_intensity);

	glEnable(GL_LIGHT1);


	//--Spotlight--//
	float spot_pos[] = {spotlight.position.x,spotlight.position.y,spotlight.position.z,1.0f};
	float spot_dir[] = {spotlight.direction.x,spotlight.direction.y,spotlight.direction.z, 0.0f};
	float spotlight_colour[] = {spotlight.colour.r,spotlight.colour.g, spotlight.colour.b,1.0f};

	glLightfv(GL_LIGHT2, GL_POSITION, spot_pos);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_dir);
	glLightf (GL_LIGHT2, GL_SPOT_CUTOFF,spotlight.cutoff);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, spotlight.exponent);
	glLightfv(GL_LIGHT2, GL_AMBIENT, spotlight_colour);
	glEnable(GL_LIGHT2);
	
	glPushMatrix();

	glTranslatef(spotlight.position.x,spotlight.position.y,spotlight.position.z);

	GLUquadric* q = gluNewQuadric(); //Create a new quadric to allow you to draw cylinders
	if (q == 0) {
		printf("Not enough memory to allocate space to draw\n");
		exit(EXIT_FAILURE);
	}
	if (modifier_key=='d' || modifier_key=='p')
		drawAxis(q);

	G308_Vector z_axis; //default cylinder direction
	z_axis.x = 0;
	z_axis.y = 0;
	z_axis.z = 1;

	G308_Vector axis; //cross product of dir and z = axis of rotation
	axis.x = (z_axis.y * spotlight.direction.z) - (z_axis.z * spotlight.direction.y);
	axis.y = (z_axis.z * spotlight.direction.x) - (z_axis.x * spotlight.direction.z);
	axis.z = (z_axis.x * spotlight.direction.y) - (z_axis.y * spotlight.direction.x);

	float dot_p = spotlight.direction.x*z_axis.x + spotlight.direction.y*z_axis.y + spotlight.direction.z*z_axis.z;
	float dir_magnitude = sqrt( spotlight.direction.x*spotlight.direction.x +spotlight.direction.y*spotlight.direction.y + spotlight.direction.z*spotlight.direction.z );
	float z_magnitude = sqrt( z_axis.x*z_axis.x + z_axis.y*z_axis.y + z_axis.z*z_axis.z );

	float angle = 180 / PI * acos (dot_p/(dir_magnitude*z_magnitude)); //angle in degrees
	glRotatef(angle,axis.x,axis.y,axis.z);

	glColor3f(1,1,0.8);
	glutSolidSphere(0.2,10, 10);

	//draw direction arrow
	glPushMatrix();
		gluCylinder(q,0.05,0.05,2,5,5);
		glTranslatef(0, 0, 2);
		glutSolidCone(0.2, 0.3,5, 5);
	glPopMatrix();

	int num_lines = 6;
	float angle_delta = 360.0/6;
	if (modifier_key=='c')
		glColor3f(1,1,0);
	for (int i=0;i<num_lines;++i){
		glPushMatrix();
			glRotatef(spotlight.cutoff,-1,0,0);
			gluCylinder(q,0.05,0.05,0.8,5,5);
		glPopMatrix();
		glRotatef(angle_delta,0,0,1);
	}

	glPopMatrix();

	glTranslatef(camera_change.x,camera_change.y,camera_change.z);

	//--ambient light--//
	float ambient[] = {0.05f, 0.05f, 0.05f, 1.0f};
	glLightfv(GL_LIGHT3, GL_AMBIENT,  ambient);
	glEnable(GL_LIGHT3);
}

void drawAxis(GLUquadric* q){

	//blue z axis 
	glPushMatrix();
		if (selected_axis==Z_AXIS)
			glColor3f(1,1,0); 
		else
			glColor3f(0, 0, 1); 	
		gluCylinder(q,0.05,0.05,1,5,5);
		glTranslatef(0, 0, 1);
		glutSolidCone(0.2, 0.2, 5, 5);
	glPopMatrix();
	//green y axis 
	glPushMatrix();
		if (selected_axis==Y_AXIS)
			glColor3f(1,1,0); 
		else
			glColor3f(0, 1, 0); 	
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(q,0.05,0.05,1,5,5);
		glTranslatef(0, 0, 1);
		glutSolidCone(0.2, 0.2, 5, 5);
	glPopMatrix();
	//red x axis 
	glPushMatrix();
		if (selected_axis==X_AXIS)
			glColor3f(1,1,0); 
		else
			glColor3f(1, 0, 0);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		gluCylinder(q,0.05,0.05,1,5,5);
		glTranslatef(0, 0, 1);
		glutSolidCone(0.2, 0.2, 5, 5);
	glPopMatrix();
}

//cycle through axis: X->Y->Z->X
void change_axis(){
	if (modifier_key!='d' && modifier_key!='p')
		return;
	if (selected_axis==X_AXIS)
		selected_axis=Y_AXIS;
	else if (selected_axis==Y_AXIS)
		selected_axis=Z_AXIS;
	else if (selected_axis==Z_AXIS)
		selected_axis=X_AXIS;
}

void G308_keyboardListener(unsigned char key, int x, int y) {
	//Code to respond to key events
	if (key==13){
		g_pGeometry->toggleMode();
	}
	else if (key=='t'){ //start 360degree rotation
		if (modifier_key=='t'){
			rotation360=false;
			modifier_key = -1;
		}else{
			rotation360=true;
			modifier_key = key;
		}
	}else if (key=='x'){
		change_axis();
	}else{
		if (key=='p' && modifier_key!='p'){
			selected_axis=X_AXIS;
			modifier_key = key;
		}else if (key=='d' && modifier_key!='d'){
			selected_axis=X_AXIS;
			modifier_key = key;
		}else if ((key=='c' && modifier_key!='c') || (key == 'e' && modifier_key!='e')){
			modifier_key = key;
		}else{
			modifier_key=-1;
		}
	}

	glutPostRedisplay();

}

void G308_mouseListener(int button, int state, int x, int y){

	if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN){
		if(glutGetModifiers()==GLUT_ACTIVE_SHIFT){
			clickDown.x=x;
			clickDown.y=y;
			rotating=true;
		}
	}else if(button==GLUT_LEFT_BUTTON && state==GLUT_UP){
		rotating=false;
	}
	else if(button==GLUT_RIGHT_BUTTON && state==GLUT_DOWN ){
		if(glutGetModifiers()==GLUT_ACTIVE_SHIFT){
			clickDown.x=x;
			clickDown.y=y;
			panning=true;
		}
	}else if(button==GLUT_RIGHT_BUTTON && state==GLUT_UP){
		panning=false;
	}else if(button==GLUT_MIDDLE_BUTTON && state==GLUT_DOWN){
		if(glutGetModifiers()==GLUT_ACTIVE_SHIFT){
			clickDown.x=x;
			clickDown.y=y;
			zooming=true;
		}
	}else if(button==GLUT_MIDDLE_BUTTON && state==GLUT_UP){
		clickDown.x=x;
		clickDown.y=y;
		zooming=false;
	}

	glutPostRedisplay();
}

void G308_arrow_keys(int key, int x, int y){
	if (key == GLUT_KEY_RIGHT){ //rotate right
		cur_rotation.x = cur_rotation.x-5;
	}
	if (key == GLUT_KEY_LEFT){ //rotate left
		cur_rotation.x = cur_rotation.x+5;
	}

	if (key == GLUT_KEY_UP){ //zoom in
		camera_change.z=camera_change.z-1;
	}
	if (key == GLUT_KEY_DOWN){ //zoom out
		camera_change.z=camera_change.z+1;

	}

}


void updateMouse(int x, int y){
	if (modifier_key!=-1){ //see if key effects light position
		float dir = 1;
		if (x<clickDown.x){
			dir= dir*-1;
		}
		if(modifier_key=='p'){ //position
			if (selected_axis==X_AXIS){
				spotlight.position.x = spotlight.position.x+0.1*dir;
			}else if (selected_axis==Y_AXIS){
				spotlight.position.y = spotlight.position.y+0.1*dir*-1;
			}else if (selected_axis==Z_AXIS){
				spotlight.position.z = spotlight.position.z+0.1*dir;
			}
		}else if(modifier_key=='d'){ //direction
			if (selected_axis==X_AXIS){
				spotlight.direction.x = spotlight.direction.x+0.01*dir;
				if (spotlight.direction.x<-1)
					spotlight.direction.x=-1;
				if (spotlight.direction.x>1)
					spotlight.direction.x=1;
			}else if (selected_axis==Y_AXIS){
				spotlight.direction.y = spotlight.direction.y+0.01*dir*-1;
				if (spotlight.direction.y<-1)
					spotlight.direction.y=-1;
				if (spotlight.direction.y>1)
					spotlight.direction.y=1;
			}else if (selected_axis==Z_AXIS){
				spotlight.direction.z = spotlight.direction.z+0.01*dir;
				if (spotlight.direction.z<-1)
					spotlight.direction.z=-1;
				if (spotlight.direction.z>1)
					spotlight.direction.z=1;
			}
		}else if(modifier_key=='c'){ //cutoff angle
			spotlight.cutoff = spotlight.cutoff+0.5*dir;
			if (spotlight.cutoff >90)
				spotlight.cutoff = 90;
			if (spotlight.cutoff <0)
				spotlight.cutoff = 0;
			
		}else if(modifier_key=='e'){ //exponent
			spotlight.exponent = spotlight.exponent+1*dir;
		}
		clickDown.x=x;
		clickDown.y=y;
	}
}


// Set Camera Position
void G308_SetCamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(G308_FOVY, (double) g_nWinWidth / (double) g_nWinHeight, G308_ZNEAR_3D, G308_ZFAR_3D);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(camera_initial.x+camera_change.x,camera_initial.y+camera_change.y, camera_initial.z+camera_change.z, -2.0, -1.5, 0.0, 0.0, 1.0, 0.0);
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










