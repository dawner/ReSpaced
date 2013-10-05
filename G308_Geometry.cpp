//---------------------------------------------------------------------------
//
// Copyright (c) 2012 Taehyun Rhee
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

#include "G308_Geometry.h"
#include "G308_ImageLoader.h"
#include <stdio.h>
#include <math.h>

G308_Geometry::G308_Geometry(int objectCount) {
	// m_pVertexArray = (G308_Point**) malloc(sizeof(G308_Point*) * numObjects);
	// m_pNormalArray = (G308_Normal**) malloc(sizeof(G308_Normal*) * numObjects);
	// m_pUVArray = (G308_UVcoord**) malloc(sizeof(G308_UVcoord*) * numObjects);
	// m_pTriangles = (G308_Triangle**) malloc(sizeof(G308_Triangle*) * numObjects);

	mode = G308_SHADE_POLYGON;

	numObjects=objectCount;

	// m_nNumPoint = m_nNumUV = m_nNumPolygon = 0;
	// m_glGeomListPoly =  (int*) malloc(sizeof(int) * numObjects); 
	// m_glGeomListWire = (int*) malloc(sizeof(int) * numObjects); 
	objects = (figure*) malloc(sizeof(figure) * objectCount);

	for (int i=0; i<objectCount; ++i) {
		objects[i].filename = NULL;

		objects[i].m_pVertexArray = NULL;
		objects[i].m_pNormalArray = NULL;
		objects[i].m_pUVArray = NULL;
		objects[i].m_pTriangles = NULL;

		objects[i].m_nNumPoint = objects[i].m_nNumUV = objects[i].m_nNumPolygon = 0;
		objects[i].m_glGeomListPoly = objects[i].m_glGeomListWire = 0;
	}

}

G308_Geometry::~G308_Geometry(void) {
	if (objects != NULL){
		for(int i = 0; i < numObjects; ++i){
			if (objects[i].m_pNormalArray != NULL){
				free((char*)objects[i].filename);

				delete[] objects[i].m_pVertexArray;
				delete[] objects[i].m_pNormalArray;
				delete[] objects[i].m_pUVArray;
				delete[] objects[i].m_pTriangles;
			}
		}
	}
	free(objects);
}

//-------------------------------------------------------
// Read in the OBJ (Note: fails quietly, so take care)
//--------------------------------------------------------
void G308_Geometry::ReadOBJ(int objNum, const char *filename, G308_Object_Properties properties) {
	FILE* fp;
	char mode, vmode;
	char str[200];
	int v1, v2, v3, n1, n2, n3, t1, t2, t3;
	int numVert, numNorm, numUV, numFace;
	float x, y, z;
	float u, v;

	numVert = numNorm = numUV = numFace = 0;

	objects[objNum].filename = filename;
	objects[objNum].properties = properties;
	
	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Could not find file: %s\n", filename);
		exit(EXIT_FAILURE);
	}else
		printf("Reading %s file\n", filename);

	// Check number of vertex, normal, uvCoord, and Face
	while (fgets(str, 200, fp) != NULL) {
		sscanf(str, "%c%c", &mode, &vmode);
		switch (mode) {
		case 'v': /* vertex, uv, normal */
			if (vmode == 't') // uv coordinate
				numUV++;
			else if (vmode == 'n') // normal
				numNorm++;
			else if (vmode == ' ') // vertex
				numVert++;
			break;
		case 'f': /* faces */
			numFace++;
			break;
		}
	}

	objects[objNum].m_nNumPoint = numVert;
	objects[objNum].m_nNumUV = numUV;
	objects[objNum].m_nNumPolygon = numFace;
	objects[objNum].m_nNumNormal = numNorm;

	printf("Number of Point %d, UV %d, Normal %d, Face %d\n", numVert, numUV,
			numNorm, numFace);
	//-------------------------------------------------------------
	//	Allocate memory for array
	//-------------------------------------------------------------
	if (objects[objNum].m_pVertexArray!= NULL)
		delete[] objects[objNum].m_pVertexArray;
	objects[objNum].m_pVertexArray = new G308_Point[objects[objNum].m_nNumPoint];

	if (objects[objNum].m_pNormalArray != NULL)
		delete[] objects[objNum].m_pNormalArray;
	objects[objNum].m_pNormalArray = new G308_Normal[objects[objNum].m_nNumNormal];

	if (objects[objNum].m_pUVArray != NULL)
		delete[] objects[objNum].m_pUVArray;
	objects[objNum].m_pUVArray = new G308_UVcoord[objects[objNum].m_nNumUV];

	if (objects[objNum].m_pTriangles != NULL)
		delete[] objects[objNum].m_pTriangles;
	objects[objNum].m_pTriangles = new G308_Triangle[objects[objNum].m_nNumPolygon];

	//-----------------------------------------------------------
	//	Read obj file
	//-----------------------------------------------------------
	numVert = numNorm = numUV = numFace = 0;

	fseek(fp, 0L, SEEK_SET);
	while (fgets(str, 200, fp) != NULL) {
		sscanf(str, "%c%c", &mode, &vmode);
		switch (mode) {
		case 'v': /* vertex, uv, normal */
			if (vmode == 't') // uv coordinate
					{
				sscanf(str, "vt %f %f", &u, &v);
				objects[objNum].m_pUVArray[numUV].u = u;
				objects[objNum].m_pUVArray[numUV].v = v;
				numUV++;
			} else if (vmode == 'n') // normal
					{
				sscanf(str, "vn %f %f %f", &x, &y, &z);
				objects[objNum].m_pNormalArray[numNorm].x = x;
				objects[objNum].m_pNormalArray[numNorm].y = y;
				objects[objNum].m_pNormalArray[numNorm].z = z;
				numNorm++;
			} else if (vmode == ' ') // vertex
					{
				sscanf(str, "v %f %f %f", &x, &y, &z);
				objects[objNum].m_pVertexArray[numVert].x = x;
				objects[objNum].m_pVertexArray[numVert].y = y;
				objects[objNum].m_pVertexArray[numVert].z = z;
				numVert++;
			}
			break;
		case 'f': /* faces : stored value is index - 1 since our index starts from 0, but obj starts from 1 */
			if (numNorm > 0 && numUV > 0) {
				sscanf(str, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
			} else if(numNorm > 0 && numUV ==0){
				sscanf(str, "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3);
			} else if(numUV > 0 && numNorm==0){
				sscanf(str, "f %d/%d %d/%d %d/%d", &v1, &t1, &v2, &t2, &v3, &t3);
			} else if(numUV==0 && numNorm==0){
				sscanf(str, "f %d %d %d", &v1, &v2, &v3);
			}
			// Vertex indicies for triangle:
			if (numVert != 0) {
				objects[objNum].m_pTriangles[numFace].v1 = v1 - 1;
				objects[objNum].m_pTriangles[numFace].v2 = v2 - 1;
				objects[objNum].m_pTriangles[numFace].v3 = v3 - 1;
			}

			// Normal indicies for triangle
			if (numNorm != 0) {
				objects[objNum].m_pTriangles[numFace].n1 = n1 - 1;
				objects[objNum].m_pTriangles[numFace].n2 = n2 - 1;
				objects[objNum].m_pTriangles[numFace].n3 = n3 - 1;
			}

			// UV indicies for triangle
			if (numUV != 0) {
				objects[objNum].m_pTriangles[numFace].t1 = t1 - 1;
				objects[objNum].m_pTriangles[numFace].t2 = t2 - 1;
				objects[objNum].m_pTriangles[numFace].t3 = t3 - 1;
			}

			numFace++;
			break;
		default:
			break;
		}
	}
	fclose(fp);
	printf("Reading OBJ file is DONE.\n");
}

//--------------------------------------------------------------
// [Assignment4]
// Create a 2D GL texture from the file given
//--------------------------------------------------------------
void G308_Geometry::ReadTexture(const char* filename) {
	//Your code here
}

void G308_Geometry::SetEnvironment(GLuint texture){
	 environment=texture;
}

//--------------------------------------------------------------
// [Assignment4]
// Fill the following function to create display list
// of the obj file to show it as polygon, using texture and normal information (if any)
//--------------------------------------------------------------
void G308_Geometry::CreateGLPolyGeometry(int objectNum) {
	figure curObj=objects[objectNum];
	G308_Point* curTrans=curObj.properties.translation;
	G308_Point* curScale=curObj.properties.scale;

	if (objects[objectNum].m_glGeomListPoly != 0)
		glDeleteLists(objects[objectNum].m_glGeomListPoly, 1);

	// Assign a display list; return 0 if err
	objects[objectNum].m_glGeomListPoly= glGenLists(1);
	glNewList(objects[objectNum].m_glGeomListPoly, GL_COMPILE);

	//Your code here

	//Draw filled polygons from vertices, normals and (if any) texture coordinates
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();
	glTranslatef(curTrans->x, curTrans->y, curTrans->z);
	glScalef(curScale->x, curScale->y, curScale->z);

	//texture setup
	bool texturing = false;
	if (objects[objectNum].properties.texture>0){
		GLuint tex = objects[objectNum].properties.texture;
		texturing=true;
		if (objects[objectNum].properties.has_alpha) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_ALPHA);
		}
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);

		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	else if (objects[objectNum].properties.is_metallic){
		glEnable( GL_TEXTURE_CUBE_MAP );
    	glBindTexture( GL_TEXTURE_CUBE_MAP, environment);
    	// Use texgen to apply cube map
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);

    	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	}

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < curObj.m_nNumPolygon; i++) {
		unsigned int vertices[] = { curObj.m_pTriangles[i].v1, curObj.m_pTriangles[i].v2,curObj.m_pTriangles[i].v3 };
		unsigned int textures[3];
		if (curObj.m_nNumUV != 0 && texturing) {
			textures[0] = curObj.m_pTriangles[i].t1;
			textures[1] = curObj.m_pTriangles[i].t2;
			textures[2] = curObj.m_pTriangles[i].t3;
		}
		unsigned int normals[] = { curObj.m_pTriangles[i].v1, curObj.m_pTriangles[i].v2,curObj.m_pTriangles[i].v3 };
		if (curObj.m_nNumNormal != 0){ //use stored normal values
			normals[0] = curObj.m_pTriangles[i].n1;
			normals[1] = curObj.m_pTriangles[i].n2;
			normals[2] = curObj.m_pTriangles[i].n3;
		}
		for (int v = 0; v < 3; v++) { //for each vertice on the face
			glNormal3f(curObj.m_pNormalArray[normals[v]].x,curObj.m_pNormalArray[normals[v]].y, curObj.m_pNormalArray[normals[v]].z);
			if (curObj.m_nNumUV != 0 && texturing){
				int mult = objects[objectNum].properties.texture_multiplier;
				glTexCoord2f(curObj.m_pUVArray[textures[v]].u*mult,curObj.m_pUVArray[textures[v]].v*mult);
			}
			glVertex3f(curObj.m_pVertexArray[vertices[v]].x,curObj.m_pVertexArray[vertices[v]].y,curObj.m_pVertexArray[vertices[v]].z);
		}
	}
	glEnd();
	glPopMatrix();
	glEndList();

	if (texturing){
		if (objects[objectNum].properties.has_alpha) {
			glDisable(GL_BLEND);
			glDisable(GL_ALPHA);
		}
		glDisable(GL_TEXTURE_2D);
	}
	else if(objects[objectNum].properties.is_metallic){
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
	}
}

void G308_Geometry::CreateGLWireGeometry(int objectNum) {
	figure curObj=objects[objectNum];
	G308_Point* curTrans=curObj.properties.translation;
	G308_Point* curRot=curObj.properties.rotation;
	G308_Point* curScale=curObj.properties.scale;

	if (objects[objectNum].m_glGeomListWire != 0)
		glDeleteLists(objects[objectNum].m_glGeomListWire, 1);

	// Assign a display list; return 0 if err
	objects[objectNum].m_glGeomListWire = glGenLists(1);
	glNewList(objects[objectNum].m_glGeomListWire, GL_COMPILE);

	glPushMatrix();
	glTranslatef(curTrans->x, curTrans->y, curTrans->z);
	glScalef(curScale->x, curScale->y, curScale->z);

	for (int i = 0; i < curObj.m_nNumPolygon; i++) {
		glBegin(GL_LINE_LOOP);
		glNormal3f(curObj.m_pNormalArray[curObj.m_pTriangles[i].n1].x,
				curObj.m_pNormalArray[curObj.m_pTriangles[i].n1].y,
				curObj.m_pNormalArray[curObj.m_pTriangles[i].n1].z);
		glVertex3f(curObj.m_pVertexArray[curObj.m_pTriangles[i].v1].x,
				curObj.m_pVertexArray[curObj.m_pTriangles[i].v1].y,
				curObj.m_pVertexArray[curObj.m_pTriangles[i].v1].z);
		glNormal3f(curObj.m_pNormalArray[curObj.m_pTriangles[i].n2].x,
				curObj.m_pNormalArray[curObj.m_pTriangles[i].n2].y,
				curObj.m_pNormalArray[curObj.m_pTriangles[i].n2].z);
		glVertex3f(curObj.m_pVertexArray[curObj.m_pTriangles[i].v2].x,
				curObj.m_pVertexArray[curObj.m_pTriangles[i].v2].y,
				curObj.m_pVertexArray[curObj.m_pTriangles[i].v2].z);
		glNormal3f(curObj.m_pNormalArray[curObj.m_pTriangles[i].n3].x,
				curObj.m_pNormalArray[curObj.m_pTriangles[i].n3].y,
				curObj.m_pNormalArray[curObj.m_pTriangles[i].n3].z);
		glVertex3f(curObj.m_pVertexArray[curObj.m_pTriangles[i].v3].x,
				curObj.m_pVertexArray[curObj.m_pTriangles[i].v3].y,
				curObj.m_pVertexArray[curObj.m_pTriangles[i].v3].z);
		glEnd();
	}
	glPopMatrix();
	glEndList();

}
void G308_Geometry::toggleMode() {
	if (mode == G308_SHADE_POLYGON) {
		mode = G308_SHADE_WIREFRAME;
	} else {
		mode = G308_SHADE_POLYGON;
	}
}

void G308_Geometry::SetupSkyBox() {


	// Assign a display list; return 0 if err
	sky_box = glGenLists(1);
	glNewList(sky_box, GL_COMPILE);

	//Draw filled polygons from vertices, normals and (if any) texture coordinates
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();

	float len = 100;

	glEnable(GL_TEXTURE_CUBE_MAP);
 	glBindTexture(GL_TEXTURE_CUBE_MAP, environment);

 	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	//Top face
	glBegin(GL_QUADS);
		glTexCoord2d(1.0f, 0.0f); glVertex3f( len, len, -len); 
		glTexCoord2d(1.0f, 1.0f); glVertex3f(-len, len, -len); 
		glTexCoord2d(0.0, 1.0f); glVertex3f(-len, len, len); 
		glTexCoord2d(0.0, 0.0f); glVertex3f( len, len, len); 
	glEnd();
	//bottom face
	glBegin(GL_QUADS);
		glTexCoord2d(1.0f, 0.0f); glVertex3f(-len, -len, len); 
		glTexCoord2d(1.0f, 1.0f); glVertex3f(len, -len, len); 
		glTexCoord2d(0.0, 1.0f); glVertex3f(len, -len, -len); 
		glTexCoord2d(0.0, 0.0f); glVertex3f(-len, -len, -len); 
	glEnd(); 
	//front face
	glBegin(GL_QUADS);
		glTexCoord2d(0.0f, 0.0f); glVertex3f( len, len, len);
		glTexCoord2d(1.0f, 0.0f); glVertex3f(-len, len, len); 
		glTexCoord2d(1.0f, 1.0f); glVertex3f(-len, -len, len); 
		glTexCoord2d(0.0f, 1.0f); glVertex3f( len, -len, len); 
	glEnd();
	//back face
	glBegin(GL_QUADS);
		glTexCoord2d(0.0f, 1.0f); glVertex3f( len, -len, -len); 
		glTexCoord2d(1.0f, 1.0f); glVertex3f(-len, -len, -len); 
		glTexCoord2d(1.0f, 0.0f); glVertex3f(-len, len, -len); 
		glTexCoord2d(0.0f, 0.0f); glVertex3f( len, len, -len); 
	glEnd();
 	//Left face
	glBegin(GL_QUADS);
		glTexCoord2d(1.0f, 0.0f); glVertex3f(-len, len, -len); 
		glTexCoord2d(1.0f, 1.0f); glVertex3f(-len, -len, -len); 
		glTexCoord2d(0.0f, 1.0f); glVertex3f(-len, -len, len); 
		glTexCoord2d(0.0f, 0.0f); glVertex3f(-len, len, len); 
	glEnd();
	//Right Face
	glBegin(GL_QUADS);
		glTexCoord2d(0.0f, 0.0f); glVertex3f( len, len, -len); 
		glTexCoord2d(1.0f, 0.0f); glVertex3f( len, len, len); 
		glTexCoord2d(1.0f, 1.0f); glVertex3f( len, -len, len); 
		glTexCoord2d(0.0f, 1.0f); glVertex3f( len, -len, -len);
	glEnd(); 

	glPopMatrix();
	glEndList();

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
}

void G308_Geometry::SkyBox() {
	glColor3f(1,1,1);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment);
	glCallList(sky_box);
	glDisable(GL_TEXTURE_CUBE_MAP);
}
	
void G308_Geometry::RenderGeometry(bool bunny) {

	if (mode == G308_SHADE_POLYGON) {
		for (int i = 0; i < numObjects;++i){
			G308_Object_Properties curProp = objects[i].properties;
			G308_RGBA* curCol = curProp.colour;
			glColor4f(curCol->r,curCol->g,curCol->b,curCol->a);
			if (curProp.ambient!=NULL)
				glMaterialfv(GL_FRONT,GL_AMBIENT,curProp.ambient);
			if (curProp.diffuse!=NULL)
				glMaterialfv(GL_FRONT,GL_DIFFUSE,curProp.diffuse);
			if (curProp.specular!=NULL)
				glMaterialfv(GL_FRONT,GL_SPECULAR,curProp.specular);
			if (curProp.shininess!=NULL)
				glMaterialfv(GL_FRONT,GL_SHININESS,curProp.shininess);

			glCallList(objects[i].m_glGeomListPoly);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_CUBE_MAP);
		}
	} else if (mode == G308_SHADE_WIREFRAME) {
		for (int i = 0; i < numObjects;++i){
			G308_Object_Properties curProp = objects[i].properties;
			G308_RGBA* curCol = curProp.colour;
			glColor3f(curCol->r,curCol->g,curCol->b);

			if (curProp.ambient!=NULL)
				glMaterialfv(GL_FRONT,GL_AMBIENT,curProp.ambient);
			if (curProp.diffuse!=NULL)
				glMaterialfv(GL_FRONT,GL_DIFFUSE,curProp.diffuse);
			if (curProp.specular!=NULL)
				glMaterialfv(GL_FRONT,GL_SPECULAR,curProp.specular);
			if (curProp.shininess!=NULL)
				glMaterialfv(GL_FRONT,GL_SHININESS,curProp.shininess);
			glCallList(objects[i].m_glGeomListWire);
		}
	} else {
		printf("Warning: Wrong Shading Mode. \n");
	}

}
