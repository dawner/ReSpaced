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
#include <cfloat>
#include "CollisionSystem.h"
#include "SculptObject.h"

CollisionSystem::CollisionSystem(SculptObject** models) {
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
	Model* asteroid1 = loadModel("asteroid1.obj");

	// the sun
	worldObjects[0].weight = 100000;
	worldObjects[0].collisionModel = simpleSphereModel(sphere, 1.22698890045f); // todo: sphere
	worldObjects[0].collisionModelScale = 1.22698890045f; // the sculpt sphere has radius 1.63

	printf("Constructor called\n");

	// meteors
	for (i = 1; i <= num_meteors; i++) {
		worldObjects[i].position.x = floatRand(-10, 10, 1000);
		worldObjects[i].position.y = floatRand(-10, 10, 1000);
		worldObjects[i].position.z = floatRand(-10, 10, 1000);
		worldObjects[i].position.x = i * 0.5;
		worldObjects[i].weight = 100;

		worldObjects[i].direction.x = floatRand(-10, 10, 1000);
		worldObjects[i].direction.y = floatRand(-10, 10, 1000);
		worldObjects[i].direction.z = floatRand(-10, 10, 1000);
		normalize(&(worldObjects[i].direction));
		worldObjects[i].speed = 0.01 * floatRand(0, 3, 1000);

		worldObjects[i].displayModel = models[2];
		worldObjects[i].displayModelScale = 1.0f;
		worldObjects[i].collisionModel = multiSphereModel(models[2], 1.0f, 64);
		worldObjects[i].collisionModelScale = 1.0f;
	}

	// bounding spheres
	for (i = 0; i <= num_meteors; i++) {
		float radius = 0;
		if (worldObjects[i].collisionModel->fullPolyModel != NULL) {
			for (int j = 0;
					j
							< worldObjects[i].collisionModel->fullPolyModel->m_nNumPoint;
					j++) {
				if (magnitude(
						&(worldObjects[i].collisionModel->fullPolyModel->m_pVertexArray[j]))
						> radius) {
					radius =
							magnitude(
									&(worldObjects[i].collisionModel->fullPolyModel->m_pVertexArray[j]));
				}
			}
		} else {
			for (int j = 0;
					j
							< worldObjects[i].collisionModel->fullPolyModelSculpt->m_nNumPoint;
					j++) {
				if (magnitude(
						&(worldObjects[i].collisionModel->fullPolyModelSculpt->m_pVertexArray[j]))
						> radius) {
					radius =
							magnitude(
									&(worldObjects[i].collisionModel->fullPolyModelSculpt->m_pVertexArray[j]));
				}
			}
		}
		worldObjects[i].radius = radius * worldObjects[i].collisionModelScale;
		printf("object %d radius is %f\n", i, worldObjects[i].radius);
	}
}

CollisionSystem::CollisionModel* CollisionSystem::simpleSphereModel(
		CollisionSystem::Model* base, float scale) {
	// determine full radius
	float radius = 0;
	for (int j = 0; j < base->m_nNumPoint; j++) {
		if (magnitude(&(base->m_pVertexArray[j])) > radius) {
			radius = magnitude(&(base->m_pVertexArray[j]));
		}
	}
	CollisionModel* ret = new CollisionModel;
	ret->fullPolyModel = base;
	ret->fullPolyModelSculpt = NULL;
	ret->sphereCount = 1;
	ret->spheres = new Sphere[1];
	ret->spheres[0].radius = radius * scale;
	ret->spheres[0].relativePosition.x = 0;
	ret->spheres[0].relativePosition.y = 0;
	ret->spheres[0].relativePosition.z = 0;
	return ret;
}

CollisionSystem::CollisionModel* CollisionSystem::multiSphereModel(
		SculptObject* base, float scale, int sphereCount) {
	// pick cluster indexes
	int* clusterIndexes = new int[base->m_nNumPoint];
	int c, i;
	// pick random centers
	G308_Point* centers = pickRandomPoints(base->m_pVertexArray,
			base->m_nNumPoint, sphereCount);
	int* pointsInCluster = new int[sphereCount];
	float prevVariance = FLT_MAX;
	int pass = 0;
	while (true) {
		pass++;
		printf("Pass %d...\n", pass);
		for (c = 0; c < sphereCount; c++) {
			pointsInCluster[c] = 0;
		}
		float newVariance = 0;
		for (i = 0; i < base->m_nNumPoint; i++) {
			// pick cluster for this point
			int picked = -1;
			float dist = FLT_MAX;
			for (int cluster = 0; cluster < sphereCount; cluster++) {
				float thisDist = distanceCalcP(base->m_pVertexArray[i],
						centers[cluster]);
				if (thisDist < dist) {
					picked = cluster;
					dist = thisDist;
				}
			}
			clusterIndexes[i] = picked;
			pointsInCluster[picked]++;
			newVariance += pow(dist, 2);
		}
		// is variance the same?
		if (prevVariance == newVariance) {
			// done
			break;
		}

		// recalculate centroids
		for (c = 0; c < sphereCount; c++) {
			centers[c].x = 0;
			centers[c].y = 0;
			centers[c].z = 0;
		}

		for (i = 0; i < base->m_nNumPoint; i++) {
			centers[clusterIndexes[i]].x += base->m_pVertexArray[i].x;
			centers[clusterIndexes[i]].y += base->m_pVertexArray[i].y;
			centers[clusterIndexes[i]].z += base->m_pVertexArray[i].z;
		}

		// now average
		for (c = 0; c < sphereCount; c++) {
			centers[c].x /= pointsInCluster[c];
			centers[c].y /= pointsInCluster[c];
			centers[c].z /= pointsInCluster[c];
		}

		// and set new variance
		prevVariance = newVariance;
	}
	// now we make spheres
	// check each cluster center against the points in it
	float* maxDists = new float[sphereCount];
	for (c = 0; c < sphereCount; c++) {
		maxDists[c] = 0;
	}
	for (i = 0; i < base->m_nNumPoint; i++) {
		float dist = distanceCalcP(base->m_pVertexArray[i],
				centers[clusterIndexes[i]]);
		if (dist > maxDists[clusterIndexes[i]]) {
			maxDists[clusterIndexes[i]] = dist;
		}
	}
	// sweet, now sphere time
	CollisionModel* ret = new CollisionModel;
	ret->fullPolyModel = NULL;
	ret->fullPolyModelSculpt = base;
	ret->sphereCount = sphereCount;
	ret->spheres = new Sphere[sphereCount];
	for (c = 0; c < sphereCount; c++) {
		ret->spheres[c].radius = maxDists[c] * scale;
		ret->spheres[c].relativePosition.x = centers[c].x * scale;
		ret->spheres[c].relativePosition.y = centers[c].y * scale;
		ret->spheres[c].relativePosition.z = centers[c].z * scale;
	}
	delete[] maxDists;
	delete[] centers;
	delete[] clusterIndexes;
	return ret;
}

G308_Point* CollisionSystem::pickRandomPoints(G308_Point* vertices, int count,
		int howMany) {
	int* choices = new int[howMany];
	int i;
	for (i = 0; i < howMany; i++) {
		choices[i] = -1;
	}
	int current = 0;
	while (current < howMany) {
		int randomIndex = rand() % count;
		bool use = true;
		for (int j = 0; j < current; j++) {
			if (choices[j] == randomIndex) {
				use = false;
				break;
			}
		}
		if (use) {
			choices[current++] = randomIndex;
		}
	}
	G308_Point* picks = new G308_Point[howMany];
	for (i = 0; i < howMany; i++) {
		picks[i] = vertices[choices[i]];
	}
	return picks;
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
			//glutSolidSphere(worldObjects[i].radius, 50, 50);
			//worldObjects[i].displayModel->RenderGeometry(0);
			// draw spheres
			for (int s = 0; s < worldObjects[i].collisionModel->sphereCount;
					s++) {
				glPushMatrix();
				glTranslatef(
						worldObjects[i].collisionModel->spheres[s].relativePosition.x,
						worldObjects[i].collisionModel->spheres[s].relativePosition.y,
						worldObjects[i].collisionModel->spheres[s].relativePosition.z);
				glutSolidSphere(
						worldObjects[i].collisionModel->spheres[s].radius, 10,
						10);
				glPopMatrix();
			}
			glPopMatrix();
		}
	}
	glDisable(GL_COLOR_MATERIAL);
}

void CollisionSystem::processCollisions() {
	// opening round: bounding spheres
	for (int i = 0; i <= num_meteors; i++) {
		for (int j = i + 1; j <= num_meteors; j++) {
			float distance = distanceCalc(worldObjects[i].position,
					worldObjects[j].position);
			float sumRadii = worldObjects[i].radius + worldObjects[j].radius;
			// spherical collision: if (radius sum) > distance we have a winner
			if (sumRadii >= distance) {
				printf("objects %d and %d could be colliding, testing\n", i, j);
				// check for collision
				bool collisionFound = false;
				CollisionModel* m1, *m2;
				m1 = worldObjects[i].collisionModel;
				m2 = worldObjects[j].collisionModel;
				for (int s1 = 0; s1 < m1->sphereCount; s1++) {
					for (int s2 = 0; s2 < m2->sphereCount; s2++) {
						// todo: support rotation
						float distance =
								sqrt(
										pow(
												(worldObjects[i].position.x
														+ m1->spheres[s1].relativePosition.x)
														- (worldObjects[j].position.x
																+ m2->spheres[s2].relativePosition.x),
												2)
												+ pow(
														(worldObjects[i].position.y
																+ m1->spheres[s1].relativePosition.y)
																- (worldObjects[j].position.y
																		+ m2->spheres[s2].relativePosition.y),
														2)
												+ pow(
														(worldObjects[i].position.z
																+ m1->spheres[s1].relativePosition.z)
																- (worldObjects[j].position.z
																		+ m2->spheres[s2].relativePosition.z),
														2));
						if (distance
								<= m1->spheres[s1].radius
										+ m2->spheres[s2].radius) {
							printf(
									"found collision objects %d %d inner spheres %d %d \n",
									i, j, s1, s2);
							collisionFound = true;
							break;
						}
					}
					if (collisionFound) {
						break;
					}
				}
			}
		}
	}
}

bool CollisionSystem::detectCollision(int obj1, int obj2) {
	return false;
}

void CollisionSystem::reactCollision(int obj1, int obj2) {
	printf("objects %d and %d are colliding\n", obj1, obj2);
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

float CollisionSystem::dotProduct(G308_Vector* from, G308_Vector* to) {
	return from->x * to->x + from->y * to->y + from->z * to->z;
}

float CollisionSystem::distanceCalcP(G308_Point from, G308_Point to) {
	return (float) sqrt(
			pow(to.x - from.x, 2) + pow(to.y - from.y, 2)
					+ pow(to.z - from.z, 2));
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
