/*
 * CollisionSystem.cpp
 *
 *  Created on: 15/10/2013
 *      Author: stewart
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "CollisionSystem.h"

CollisionSystem::CollisionSystem() {
	printf("Constructor called\n");
	num_meteors = 10;
	int i;

	worldObjects = (Object*) malloc(sizeof(Object) * (num_meteors + 1));

	// default values
	for (i = 0; i <= num_meteors; i++) {
		worldObjects[i].position.x = 0;
		worldObjects[i].position.y = 0;
		worldObjects[i].position.z = 0;
		worldObjects[i].rotation.x = 0;
		worldObjects[i].rotation.y = 0;
		worldObjects[i].rotation.z = 0;
		worldObjects[i].direction.x = 0;
		worldObjects[i].direction.y = 0;
		worldObjects[i].direction.z = 0;
		worldObjects[i].speed = 0;
		worldObjects[i].weight = 0;

		worldObjects[i].displayModel = NULL;
		worldObjects[i].displayModelScale = 1;
		worldObjects[i].collisionModel = NULL;
		worldObjects[i].collisionModelScale = 1;

		worldObjects[i].radius = 0;
	}

	Model* sphere = loadModel("sculpt_sphere.obj");

	// the sun
	worldObjects[0].position.x = -6;
	worldObjects[0].weight = 100000;
	worldObjects[0].collisionModel = sphere; // todo: sphere
	worldObjects[0].collisionModelScale = 1.22698890045f; // the sculpt sphere has radius 1.63
	worldObjects[0].radius = 2;

	printf("Constructor called\n");

	// meteors
	for (i = 1; i <= num_meteors; i++) {
		worldObjects[i].position.x = floatRand(-10, 10, 1000);
		worldObjects[i].position.y = floatRand(-10, 10, 1000);
		worldObjects[i].position.z = floatRand(-10, 10, 1000);
		worldObjects[i].weight = 100;

		worldObjects[i].direction.x = floatRand(-10, 10, 1000);
		worldObjects[i].direction.y = floatRand(-10, 10, 1000);
		worldObjects[i].direction.z = floatRand(-10, 10, 1000);
		normalize(&(worldObjects[i].direction));
		worldObjects[i].speed = 0.01 * floatRand(0, 3, 1000);

		worldObjects[i].displayModel = sphere;
		worldObjects[i].displayModelScale = 0.6f;
		worldObjects[i].collisionModel = sphere;
		worldObjects[i].collisionModelScale = 0.6f;

		worldObjects[i].radius = 0.978003957134f;
	}
}

CollisionSystem::Model* CollisionSystem::loadModel(const char* filename) {
	Model *model = (Model*) malloc(sizeof(Model));
	FILE* fp;
	char mode, vmode;
	char str[200];
	int v1, v2, v3, n1, n2, n3, t1, t2, t3;
	int numVert, numNorm, numUV, numFace;
	float x, y, z;
	float u, v;

	numVert = numNorm = numUV = numFace = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Could not find file: %s\n", filename);
		exit(EXIT_FAILURE);
	} else
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

	model->m_nNumPoint = numVert;
	model->m_nNumUV = numUV;
	model->m_nNumPolygon = numFace;
	model->m_nNumNormal = numNorm;

	printf("Number of Point %d, UV %d, Normal %d, Face %d\n", numVert, numUV,
			numNorm, numFace);
	//-------------------------------------------------------------
	//	Allocate memory for array
	//-------------------------------------------------------------
	model->m_pVertexArray = new G308_Point[model->m_nNumPoint];

	model->m_pNormalArray = new G308_Normal[model->m_nNumNormal];

	model->m_pUVArray = new G308_UVcoord[model->m_nNumUV];

	model->m_pTriangles = new G308_Triangle[model->m_nNumPolygon];

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
				model->m_pUVArray[numUV].u = u;
				model->m_pUVArray[numUV].v = v;
				numUV++;
			} else if (vmode == 'n') // normal
					{
				sscanf(str, "vn %f %f %f", &x, &y, &z);
				model->m_pNormalArray[numNorm].x = x;
				model->m_pNormalArray[numNorm].y = y;
				model->m_pNormalArray[numNorm].z = z;
				numNorm++;
			} else if (vmode == ' ') // vertex
					{
				sscanf(str, "v %f %f %f", &x, &y, &z);
				model->m_pVertexArray[numVert].x = x;
				model->m_pVertexArray[numVert].y = y;
				model->m_pVertexArray[numVert].z = z;
				numVert++;
			}
			break;
		case 'f': /* faces : stored value is index - 1 since our index starts from 0, but obj starts from 1 */
			if (numNorm > 0 && numUV > 0) {
				sscanf(str, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2,
						&t2, &n2, &v3, &t3, &n3);
			} else if (numNorm > 0 && numUV == 0) {
				sscanf(str, "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3,
						&n3);
			} else if (numUV > 0 && numNorm == 0) {
				sscanf(str, "f %d/%d %d/%d %d/%d", &v1, &t1, &v2, &t2, &v3,
						&t3);
			} else if (numUV == 0 && numNorm == 0) {
				sscanf(str, "f %d %d %d", &v1, &v2, &v3);
			}
			// Vertex indicies for triangle:
			if (numVert != 0) {
				model->m_pTriangles[numFace].v1 = v1 - 1;
				model->m_pTriangles[numFace].v2 = v2 - 1;
				model->m_pTriangles[numFace].v3 = v3 - 1;
			}

			// Normal indicies for triangle
			if (numNorm != 0) {
				model->m_pTriangles[numFace].n1 = n1 - 1;
				model->m_pTriangles[numFace].n2 = n2 - 1;
				model->m_pTriangles[numFace].n3 = n3 - 1;
			}

			// UV indicies for triangle
			if (numUV != 0) {
				model->m_pTriangles[numFace].t1 = t1 - 1;
				model->m_pTriangles[numFace].t2 = t2 - 1;
				model->m_pTriangles[numFace].t3 = t3 - 1;
			}

			numFace++;
			break;
		default:
			break;
		}
	}
	fclose(fp);
	printf("Reading OBJ file is DONE.\n");
	return model;
}

void CollisionSystem::step() {
	// physics
	processPhysics();

	// collisions
	processCollisions();

	// rendering
	render();
}

float CollisionSystem::floatRand(int min, int max, int precision) {
	int modulo = (max - min + 1) * precision;
	int randChoice = rand() % modulo;
	return ((float) randChoice) / ((float) precision) + min;
}

void CollisionSystem::render() {
	// temp
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	for (int i = 0; i <= num_meteors; i++) {
		if (worldObjects[i].displayModel != NULL) {
			// show this one
			glPushMatrix();
			glColor3f(0.8f, 0.8f, 0.8f);
			glTranslatef(worldObjects[i].position.x, worldObjects[i].position.y,
					worldObjects[i].position.z);
			glScalef(worldObjects[i].displayModelScale,
					worldObjects[i].displayModelScale,
					worldObjects[i].displayModelScale);
			glRotatef(worldObjects[i].rotation.z, 0, 0, 1);
			glRotatef(worldObjects[i].rotation.y, 0, 1, 0);
			glRotatef(worldObjects[i].rotation.x, 1, 0, 0);
			displayModel(worldObjects[i].displayModel);
			glPopMatrix();
		}
	}
	glDisable(GL_COLOR_MATERIAL);
}

void CollisionSystem::displayModel(CollisionSystem::Model *m) {
	//Your code here
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < m->m_nNumPolygon; i++) {
		glTexCoord2f(m->m_pUVArray[m->m_pTriangles[i].t1].u * 5,
				m->m_pUVArray[m->m_pTriangles[i].t1].v * 5);
		glNormal3f(m->m_pNormalArray[m->m_pTriangles[i].n1].x,
				m->m_pNormalArray[m->m_pTriangles[i].n1].y,
				m->m_pNormalArray[m->m_pTriangles[i].n1].z);
		glVertex3f(m->m_pVertexArray[m->m_pTriangles[i].v1].x,
				m->m_pVertexArray[m->m_pTriangles[i].v1].y,
				m->m_pVertexArray[m->m_pTriangles[i].v1].z);
		glTexCoord2f(m->m_pUVArray[m->m_pTriangles[i].t2].u * 5,
				m->m_pUVArray[m->m_pTriangles[i].t2].v * 5);
		glNormal3f(m->m_pNormalArray[m->m_pTriangles[i].n2].x,
				m->m_pNormalArray[m->m_pTriangles[i].n2].y,
				m->m_pNormalArray[m->m_pTriangles[i].n2].z);
		glVertex3f(m->m_pVertexArray[m->m_pTriangles[i].v2].x,
				m->m_pVertexArray[m->m_pTriangles[i].v2].y,
				m->m_pVertexArray[m->m_pTriangles[i].v2].z);
		glTexCoord2f(m->m_pUVArray[m->m_pTriangles[i].t3].u * 5,
				m->m_pUVArray[m->m_pTriangles[i].t3].v * 5);
		glNormal3f(m->m_pNormalArray[m->m_pTriangles[i].n3].x,
				m->m_pNormalArray[m->m_pTriangles[i].n3].y,
				m->m_pNormalArray[m->m_pTriangles[i].n3].z);
		glVertex3f(m->m_pVertexArray[m->m_pTriangles[i].v3].x,
				m->m_pVertexArray[m->m_pTriangles[i].v3].y,
				m->m_pVertexArray[m->m_pTriangles[i].v3].z);
	}
	glEnd();
}

void CollisionSystem::processCollisions() {
	// very cheap way of doing this
	for (int i = 0; i <= num_meteors; i++) {
		for (int j = i + 1; j <= num_meteors; j++) {
			float distance = distanceCalc(worldObjects[i].position,
					worldObjects[j].position);
			float sumRadii = worldObjects[i].radius + worldObjects[j].radius;
			// spherical collision: if (radius sum) > distance we have a winner
			if (sumRadii > distance) {
				// react to collision somehow
			}
		}
	}
}

void CollisionSystem::processPhysics() {
	// update positions
	for (int i = 0; i <= num_meteors; i++) {
		// This assumes direction vector is already normalized, might not be safe in practice
		worldObjects[i].position.x += worldObjects[i].direction.x
				* worldObjects[i].speed;
		worldObjects[i].position.y += worldObjects[i].direction.y
				* worldObjects[i].speed;
		worldObjects[i].position.z += worldObjects[i].direction.z
				* worldObjects[i].speed;
	}

	// basic gravity
	float gconstant = 0.000000067;
	for (int i = 0; i <= num_meteors; i++) {
		for (int j = 0; j <= num_meteors; j++) {
			if (i == j) {
				continue;
			}
			// pull j towards i
			float gravity = gconstant * worldObjects[i].weight
					/ pow(
							distanceCalc(worldObjects[i].position,
									worldObjects[j].position), 2);
			G308_Vector diffvector;
			diffvector.x = worldObjects[i].position.x
					- worldObjects[j].position.x;
			diffvector.y = worldObjects[i].position.y
					- worldObjects[j].position.y;
			diffvector.z = worldObjects[i].position.z
					- worldObjects[j].position.z;
			normalize(&diffvector);
			// calculate new
			G308_Vector newMovement;
			newMovement.x = diffvector.x * gravity
					+ worldObjects[j].direction.x * worldObjects[j].speed;
			newMovement.y = diffvector.y * gravity
					+ worldObjects[j].direction.y * worldObjects[j].speed;
			newMovement.z = diffvector.z * gravity
					+ worldObjects[j].direction.z * worldObjects[j].speed;
			worldObjects[j].speed = magnitude(&newMovement);
			normalize(&newMovement);
			worldObjects[j].direction = newMovement;
		}
	}
}

float CollisionSystem::distanceCalc(G308_Vector from, G308_Vector to) {
	return (float) sqrt(
			pow(to.x - from.x, 2) + pow(to.y - from.y, 2)
					+ pow(to.z - from.z, 2));
}

float CollisionSystem::magnitude(G308_Vector* vector) {
	return (float) sqrt(
			pow(vector->x, 2) + pow(vector->y, 2) + pow(vector->z, 2));
}

void CollisionSystem::normalize(G308_Vector* vector) {
	float len = (float) sqrt(
			pow(vector->x, 2) + pow(vector->y, 2) + pow(vector->z, 2));
	vector->x /= len;
	vector->y /= len;
	vector->z /= len;
}
