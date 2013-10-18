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

int spheresPerModel = 24;

CollisionSystem::CollisionSystem(SculptObject** models) {
	num_NonSunObjects = 18;
	int i;

	worldObjects = (Object*) malloc(sizeof(Object) * (num_NonSunObjects + 1));

	Model* sphere = loadModel("sculpt_sphere.obj");

	// Make collision models
	collisionModels = new CollisionModel*[8];
	collisionModels[0] = simpleSphereModel(sphere, 1.22698890045f);
	for (i = 1; i < 8; i++) {
		float scale = 1;
		if (i >= 2 && i <= 4) {
			scale = 0.25; // asteroid
		} else if (i >= 5 && i <= 7) {
			scale = 0.5; // planet
		}
		collisionModels[i] = multiSphereModel(models[i - 1], spheresPerModel,
				scale);
	}

	// default values
	for (i = 0; i <= num_NonSunObjects; i++) {
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
		worldObjects[i].collisionModel = NULL;

		worldObjects[i].radius = 0;
	}

	// the sun
	worldObjects[0].weight = 100000;
	worldObjects[0].collisionModel = collisionModels[0]; // basic sphere not used anywhere else

	// planets
	worldObjects[1].position.x = -3.5;
	worldObjects[1].position.y = -3.5;
	worldObjects[1].displayModel = models[5];
	worldObjects[1].collisionModel = collisionModels[6];
	worldObjects[1].direction.x = -5;
	worldObjects[1].direction.y = 5;
	worldObjects[1].direction.z = 0;
	normalize(&worldObjects[1].direction);
	worldObjects[1].speed = 0.036;
	worldObjects[1].weight = 50;

	worldObjects[2].position.x = 5;
	worldObjects[2].position.y = -5;
	worldObjects[2].displayModel = models[4];
	worldObjects[2].collisionModel = collisionModels[5];
	worldObjects[2].direction.x = -5;
	worldObjects[2].direction.y = -5;
	worldObjects[2].direction.z = 0;
	normalize(&worldObjects[2].direction);
	worldObjects[2].speed = 0.0295;
	worldObjects[2].weight = 100;

	worldObjects[3].position.x = 6.5;
	worldObjects[3].position.y = 6.5;
	worldObjects[3].displayModel = models[6];
	worldObjects[3].collisionModel = collisionModels[7];
	worldObjects[3].direction.x = 5;
	worldObjects[3].direction.y = -5;
	worldObjects[3].direction.z = 0;
	normalize(&worldObjects[3].direction);
	worldObjects[3].speed = 0.0273;
	worldObjects[3].weight = 200;

	// static meteors (collision system)
	worldObjects[4].position.x = -8;
	worldObjects[4].position.y = 8;
	worldObjects[4].position.z = 8;
	worldObjects[4].displayModel = models[1];
	worldObjects[4].collisionModel = collisionModels[2];
	worldObjects[4].direction.x = 1;
	worldObjects[4].direction.y = -1;
	normalize(&worldObjects[4].direction);
	worldObjects[4].speed = 0.06;
	worldObjects[4].weight = 2;

	worldObjects[5].position.x = 8;
	worldObjects[5].position.y = 0;
	worldObjects[5].position.z = 8;
	worldObjects[5].displayModel = models[2];
	worldObjects[5].collisionModel = collisionModels[3];
	worldObjects[5].direction.x = -1;
	worldObjects[5].speed = 0.035;
	worldObjects[5].weight = 2;

	// meteor belt
	int numInBelt = num_NonSunObjects - 5;
	for (i = 6; i <= num_NonSunObjects; i++) {
		worldObjects[i].position.x = cos(
				(360.0 / numInBelt) * PI / 180 * (i - 6)) * 12;
		worldObjects[i].position.y = sin(
				(360.0 / numInBelt) * PI / 180 * (i - 6)) * 12;
		worldObjects[i].position.z = 0;
		worldObjects[i].weight = 0.01;

		worldObjects[i].direction.x = 1;
		if (worldObjects[i].position.y == 0) {
			worldObjects[i].direction.x = 0;
			worldObjects[i].direction.y = 1;
		} else {
			worldObjects[i].direction.y = -worldObjects[i].position.x
					/ worldObjects[i].position.y;
		}
		worldObjects[i].direction.z = 0;
		normalize(&(worldObjects[i].direction));
		worldObjects[i].speed = 0.0265;

		int astModel = rand() % 3;
		worldObjects[i].displayModel = models[astModel + 1];
		worldObjects[i].collisionModel = collisionModels[astModel + 2];
	}

// bounding spheres
	for (i = 0; i <= num_NonSunObjects; i++) {
		if (i == 0) {
			worldObjects[i].radius = 2;
		} else {
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
			worldObjects[i].radius = radius;
		}
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
		SculptObject* base, int sphereCount, float scale) {
	// sweet, now sphere time
	CollisionModel* ret = new CollisionModel;
	ret->fullPolyModel = NULL;
	ret->fullPolyModelSculpt = base;
	ret->spheres = NULL;
	ret->scale = scale;
	updateMultiModel(ret, sphereCount);
	return ret;
}

void CollisionSystem::updateMultiModel(CollisionModel* cm, int sphereCount) {
	SculptObject* base = cm->fullPolyModelSculpt;
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
	cm->sphereCount = sphereCount;
	if (cm->spheres != NULL) {
		delete[] cm->spheres;
	}
	cm->spheres = new Sphere[sphereCount];
	for (c = 0; c < sphereCount; c++) {
		cm->spheres[c].radius = maxDists[c] * cm->scale;
		cm->spheres[c].relativePosition.x = centers[c].x * cm->scale;
		cm->spheres[c].relativePosition.y = centers[c].y * cm->scale;
		cm->spheres[c].relativePosition.z = centers[c].z * cm->scale;
	}
	delete[] maxDists;
	delete[] centers;
	delete[] clusterIndexes;
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

void CollisionSystem::updateAll() {
	for (int i = 0; i <= num_NonSunObjects; i++) {
		if (worldObjects[i].collisionModel->fullPolyModelSculpt != NULL) {
			if (worldObjects[i].collisionModel->fullPolyModelSculpt->geometry_changed) {
				worldObjects[i].collisionModel->fullPolyModelSculpt->geometry_changed =
						false;
				updateMultiModel(worldObjects[i].collisionModel,
						spheresPerModel);
			}
		}
	}
}

float CollisionSystem::floatRand(int min, int max, int precision) {
	int modulo = (max - min + 1) * precision;
	int randChoice = rand() % modulo;
	return ((float) randChoice) / ((float) precision) + min;
}

void CollisionSystem::render() {
	// temp
	//glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	for (int i = 0; i <= num_NonSunObjects; i++) {
		if (worldObjects[i].displayModel != NULL) {
			// show this one
			glPushMatrix();
			glColor3f(0.8f, 0.8f, 0.8f);
			glTranslatef(worldObjects[i].position.x, worldObjects[i].position.y,
					worldObjects[i].position.z);
			glRotatef(worldObjects[i].rotation.z, 0, 0, 1);
			glRotatef(worldObjects[i].rotation.y, 0, 1, 0);
			glRotatef(worldObjects[i].rotation.x, 1, 0, 0);
			//glutSolidSphere(worldObjects[i].radius, 50, 50);
			float scale = worldObjects[i].collisionModel->scale;
			glScalef(scale, scale, scale);
			worldObjects[i].displayModel->RenderGeometry(0);
			// draw spheres
			//for (int s = 0; s < worldObjects[i].collisionModel->sphereCount;
			//		s++) {
			//	glPushMatrix();
			//	glTranslatef(
			//			worldObjects[i].collisionModel->spheres[s].relativePosition.x,
			//			worldObjects[i].collisionModel->spheres[s].relativePosition.y,
			//			worldObjects[i].collisionModel->spheres[s].relativePosition.z);
			//	glutSolidSphere(
			//			worldObjects[i].collisionModel->spheres[s].radius, 10,
			//			10);
			//	glPopMatrix();
			//}
			glPopMatrix();
		}
	}
	//glDisable(GL_COLOR_MATERIAL);
}

void CollisionSystem::processCollisions() {
// opening round: bounding spheres
	for (int i = 0; i <= num_NonSunObjects; i++) {
		for (int j = i + 1; j <= num_NonSunObjects; j++) {
			float distance = distanceCalc(worldObjects[i].position,
					worldObjects[j].position);
			float sumRadii = worldObjects[i].radius + worldObjects[j].radius;
			// spherical collision: if (radius sum) > distance we have a winner
			if (sumRadii >= distance) {
				//printf("objects %d and %d could be colliding, testing\n", i, j);
				// check for collision
				bool collisionFound = false;
				int collisS1, collisS2;
				G308_Point colPos1, colPos2;
				CollisionModel* m1, *m2;
				m1 = worldObjects[i].collisionModel;
				m2 = worldObjects[j].collisionModel;
				// precalc relevant cosine and sine values
				float rot1vals[6];
				float rot2vals[6];
				rot1vals[0] = sin(worldObjects[i].rotation.x * PI / 180);
				rot1vals[1] = cos(worldObjects[i].rotation.x * PI / 180);
				rot1vals[2] = sin(worldObjects[i].rotation.y * PI / 180);
				rot1vals[3] = cos(worldObjects[i].rotation.y * PI / 180);
				rot1vals[4] = sin(worldObjects[i].rotation.z * PI / 180);
				rot1vals[5] = cos(worldObjects[i].rotation.z * PI / 180);
				rot2vals[0] = sin(worldObjects[j].rotation.x * PI / 180);
				rot2vals[1] = cos(worldObjects[j].rotation.x * PI / 180);
				rot2vals[2] = sin(worldObjects[j].rotation.y * PI / 180);
				rot2vals[3] = cos(worldObjects[j].rotation.y * PI / 180);
				rot2vals[4] = sin(worldObjects[j].rotation.z * PI / 180);
				rot2vals[5] = cos(worldObjects[j].rotation.z * PI / 180);
				for (int s1 = 0; s1 < m1->sphereCount; s1++) {
					for (int s2 = 0; s2 < m2->sphereCount; s2++) {
						G308_Point relPos1, relPos2;
						relPos1 = m1->spheres[s1].relativePosition;
						relPos2 = m2->spheres[s2].relativePosition;
						// rotation
						G308_Point rotRelPos1, rotRelPos2;
						rotRelPos1.x = rot1vals[5] * rot1vals[3] * relPos1.x
								+ (rot1vals[5] * rot1vals[2] * rot1vals[0]
										- rot1vals[4] * rot1vals[1]) * relPos1.y
								+ (rot1vals[5] * rot1vals[2] * rot1vals[1]
										+ rot1vals[4] * rot1vals[0])
										* relPos1.z;
						rotRelPos1.y = rot1vals[4] * rot1vals[3] * relPos1.x
								+ (rot1vals[4] * rot1vals[2] * rot1vals[0]
										+ rot1vals[5] * rot1vals[1]) * relPos1.y
								+ (rot1vals[4] * rot1vals[2] * rot1vals[1]
										- rot1vals[5] * rot1vals[0])
										* relPos1.z;
						rotRelPos1.z = -rot1vals[2] * relPos1.x
								+ rot1vals[3] * rot1vals[0] * relPos1.y
								+ rot1vals[3] * rot1vals[1] * relPos1.z;
						rotRelPos2.x = rot2vals[5] * rot2vals[3] * relPos2.x
								+ (rot2vals[5] * rot2vals[2] * rot2vals[0]
										- rot2vals[4] * rot2vals[1]) * relPos2.y
								+ (rot2vals[5] * rot2vals[2] * rot2vals[1]
										+ rot2vals[4] * rot2vals[0])
										* relPos2.z;
						rotRelPos2.y = rot2vals[4] * rot2vals[3] * relPos2.x
								+ (rot2vals[4] * rot2vals[2] * rot2vals[0]
										+ rot2vals[5] * rot2vals[1]) * relPos2.y
								+ (rot2vals[4] * rot2vals[2] * rot2vals[1]
										- rot2vals[5] * rot2vals[0])
										* relPos2.z;
						rotRelPos2.z = -rot2vals[2] * relPos2.x
								+ rot2vals[3] * rot2vals[0] * relPos2.y
								+ rot2vals[3] * rot2vals[1] * relPos2.z;
						float distance =
								sqrt(
										pow(
												(worldObjects[i].position.x
														+ rotRelPos1.x)
														- (worldObjects[j].position.x
																+ rotRelPos2.x),
												2)
												+ pow(
														(worldObjects[i].position.y
																+ rotRelPos1.y)
																- (worldObjects[j].position.y
																		+ rotRelPos2.y),
														2)
												+ pow(
														(worldObjects[i].position.z
																+ rotRelPos1.z)
																- (worldObjects[j].position.z
																		+ rotRelPos2.z),
														2));
						if (distance
								<= m1->spheres[s1].radius
										+ m2->spheres[s2].radius) {
							//printf(
							//		"found collision objects %d %d inner spheres %d %d \n",
							//		i, j, s1, s2);
							colPos1 = rotRelPos1;
							colPos2 = rotRelPos2;
							collisS1 = s1;
							collisS2 = s2;
							collisionFound = true;
							break;
						}
					}
					if (collisionFound) {
						break;
					}
				}
				if (collisionFound) {
					reactCollision(i, j, colPos1, colPos2);
				}
			}
		}
	}
}

bool CollisionSystem::detectCollision(int obj1, int obj2) {
	return false;
}

void CollisionSystem::reactCollision(int obj1, int obj2, G308_Point relPos1,
		G308_Point relPos2) {
	if (obj1 == 0) {
		// lol this is the sun
		// "destroy" the other object
		worldObjects[obj2].position.x = 99999 + obj2 * 1000;
		worldObjects[obj2].speed = 0;
	} else {
		// approximate a collision between obj1 and obj2
		// give them a very strong push away from each other in the vector of the collision
		G308_Vector diffvector;
		diffvector.x = (relPos2.x + worldObjects[obj2].position.x)
				- (relPos1.x + worldObjects[obj1].position.x);
		diffvector.y = (relPos2.y + worldObjects[obj2].position.y)
				- (relPos1.y + worldObjects[obj1].position.y);
		diffvector.z = (relPos2.z + worldObjects[obj2].position.z)
				- (relPos1.z + worldObjects[obj1].position.z);
		normalize(&diffvector);
		// Diffvector now points from obj1 to obj2
		// So push obj2 further in this direction
		// and pull obj1 in the other direction
		G308_Vector newMovement;
		float repelFactor = 0.25
				* (worldObjects[obj1].weight + worldObjects[obj2].weight) / 2;
		newMovement.x = diffvector.x * repelFactor / worldObjects[obj2].weight
				+ worldObjects[obj2].direction.x * worldObjects[obj2].speed;
		newMovement.y = diffvector.y * repelFactor / worldObjects[obj2].weight
				+ worldObjects[obj2].direction.y * worldObjects[obj2].speed;
		newMovement.z = diffvector.z * repelFactor / worldObjects[obj2].weight
				+ worldObjects[obj2].direction.z * worldObjects[obj2].speed;
		worldObjects[obj2].speed = magnitude(&newMovement);
		normalize(&newMovement);
		worldObjects[obj2].direction = newMovement;
		// and the pull
		newMovement.x = -diffvector.x * repelFactor / worldObjects[obj1].weight
				+ worldObjects[obj1].direction.x * worldObjects[obj1].speed;
		newMovement.y = -diffvector.y * repelFactor / worldObjects[obj1].weight
				+ worldObjects[obj1].direction.y * worldObjects[obj1].speed;
		newMovement.z = -diffvector.z * repelFactor / worldObjects[obj1].weight
				+ worldObjects[obj1].direction.z * worldObjects[obj1].speed;
		worldObjects[obj1].speed = magnitude(&newMovement);
		normalize(&newMovement);
		worldObjects[obj1].direction = newMovement;
	}
}

void CollisionSystem::processPhysics() {
// update positions
	for (int i = 0; i <= num_NonSunObjects; i++) {
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
	for (int i = 0; i <= num_NonSunObjects; i++) {
		for (int j = 0; j <= num_NonSunObjects; j++) {
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
	//printf("Distances: %f/%f/%f\n",
	//		distanceCalcP(worldObjects[0].position, worldObjects[1].position),
	//		distanceCalcP(worldObjects[0].position, worldObjects[2].position),
	//		distanceCalcP(worldObjects[0].position, worldObjects[3].position));
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
