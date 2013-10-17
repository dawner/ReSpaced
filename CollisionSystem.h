/*
 * CollisionSystem.h
 *
 *  Created on: 15/10/2013
 *      Author: stewart
 */

#ifndef COLLISIONSYSTEM_H_
#define COLLISIONSYSTEM_H_

#pragma once

#include "define.h"
#include <GL/glut.h>
#include "SculptObject.h"

class CollisionSystem {
private:
	int num_NonSunObjects;

	typedef struct Model {
		// Array for Geometry
		G308_Point* m_pVertexArray;		// Vertex Array
		G308_Normal* m_pNormalArray;	// Normal Array
		G308_Triangle* m_pTriangles;	// Triangle Array
		G308_UVcoord* m_pUVArray;	    // Texture Coordinate Array

		// Data for Geometry
		int m_nNumPoint;
		int m_nNumUV;
		int m_nNumNormal;
		int m_nNumPolygon;
	} Model;

	typedef struct Sphere {
		G308_Point relativePosition;
		float radius;
	};

	typedef struct CollisionModel {
		int sphereCount;
		Sphere* spheres;
		Model* fullPolyModel;
		SculptObject* fullPolyModelSculpt;
	};

	typedef struct Object {
		G308_Point position;
		G308_Point rotation;
		G308_Vector direction;
		float speed;
		float weight;

		SculptObject* displayModel;
		CollisionModel* collisionModel;

		// for bogus implementation
		float radius;
	} Object;

	Object* worldObjects;
	CollisionModel** collisionModels;

	Model* loadModel(const char*);

	float floatRand(int, int, int);

	void processPhysics();
	void processCollisions();
	bool detectCollision(int, int);
	void reactCollision(int, int, G308_Point, G308_Point);
	void render();
	void normalize(G308_Vector*);
	float magnitude(G308_Vector*);
	float dotProduct(G308_Vector*, G308_Vector*);

	float distanceCalcP(G308_Point, G308_Point);
	float distanceCalc(G308_Vector, G308_Vector);

	CollisionModel* simpleSphereModel(Model*, float);
	CollisionModel* multiSphereModel(SculptObject*, int);
	G308_Point* pickRandomPoints(G308_Point*, int, int);

public:
	CollisionSystem(SculptObject**);
	void step();
};

#endif /* COLLISIONSYSTEM_H_ */
