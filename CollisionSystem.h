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

class CollisionSystem {
private:
	int num_meteors;

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

	typedef struct Object {
		G308_Point position;
		G308_Point rotation;
		G308_Vector direction;
		float speed;
		float weight;

		Model* displayModel;
		float displayModelScale;
		Model* collisionModel;
		float collisionModelScale;

		// for bogus implementation
		float radius;
	} Object;

	Object* worldObjects;

	Model* loadModel(const char*);

	float floatRand(int, int, int);

	void processPhysics();
	void processCollisions();
	void render();
	void displayModel(Model*);
	void normalize(G308_Vector*);
	float magnitude(G308_Vector*);

	float distanceCalc(G308_Point, G308_Point);

public:
	CollisionSystem(void);
	void step();
};

#endif /* COLLISIONSYSTEM_H_ */
