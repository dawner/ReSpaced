//---------------------------------------------------------------------------
//
// Copyright (c) 2012 Taehyun Rhee
//
// Edited by Daniel Atkins
//
// This software is provided 'as-is' for assignment of COMP308 
// in ECS, Victoria University of Wellington, 
// without any express or implied warranty. 
// In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#pragma once


#define PI 3.14159265
// Default Window 
#define G308_WIN_WIDTH	640
#define G308_WIN_HEIGHT	480

// Projection parameters
#define G308_FOVY		20.0
#define G308_ZNEAR_3D	1
#define G308_ZFAR_3D	1000.0
#define G308_ZNEAR_2D	-50.0
#define G308_ZFAR_2D	50.0

// Shading mode : 0 Polygon, 1 Wireframe
#define G308_SHADE_POLYGON 0		
#define G308_SHADE_WIREFRAME 1

// Define number of vertex 
#define G308_NUM_VERTEX_PER_FACE 3 // Triangle = 3, Quad = 4 

//axis
typedef int AXIS;
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

// Define Basic Structures
struct G308_Point {
	float x;
	float y;
	float z;
};

struct G308_RGBA {
	float r;
	float g;
	float b;
	float a;
};

struct G308_Rotation {
	float x;
	float y;
	float z;
	float angle;
};

struct G308_Object_Properties {
	G308_Point* translation;
	G308_Point* rotation;
	G308_Point* scale;
	G308_RGBA* colour;

	float* ambient;
	float* diffuse;
	float* specular;
	float* shininess;
	bool is_metallic;

	char* texture_filename;
	unsigned int texture;
	float texture_multiplier;
	bool has_alpha;
};

typedef G308_Point G308_Normal;

typedef G308_Point G308_Vector;

struct G308_UVcoord {

	float u;
	float v;
};

struct G308_Spotlight {
	G308_Vector direction;
	G308_Point position;
	float cutoff;
	float exponent;
	G308_RGBA colour;
};

/*
 * Normals and textures need to be defined by face, not by vertex.
 * Reminder: these are just indicies into the normal and texture arrays.
 * n1 and t1 are the normals and texture co-ordinates for vertex 1 of this face.
 * Same goes for (v2,n2,t2) etc.
 */
struct G308_Triangle {

	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
	unsigned int n1;
	unsigned int n2;
	unsigned int n3;
	unsigned int t1;
	unsigned int t2;
	unsigned int t3;
};

struct G308_Quad {

	unsigned int v1;
	unsigned int v2;
	unsigned int v3;
	unsigned int v4;
	unsigned int n1;
	unsigned int n2;
	unsigned int n3;
	unsigned int n4;
	unsigned int t1;
	unsigned int t2;
	unsigned int t3;
	unsigned int t4;
};

