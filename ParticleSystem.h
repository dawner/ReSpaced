//---------------------------------------------------------------------------
//
// Copyright (c) 2013 Dawn Richardson 
//
//----------------------------------------------------------------------------

#pragma once

#include "define.h"
#include <GL/glut.h>

class ParticleSystem
{
private:

	//Holds properties of a single particle in the system
	typedef struct Particle {

		G308_Point position;
		G308_Vector direction;
		int life;
		G308_RGBA colour;

		//store links to particles on either side
		Particle* next; 
		Particle* prev;

	} Particle;

	//emitter variables
	G308_Point emitter_position;
	G308_Vector global_force;
	float speed;
	float speed_variation;
	int life;
	int life_variation;
	G308_RGBA colour;
	G308_RGBA colour_variation;

	int total_particles; //maximum particle number from space allocated
	int particle_count; //current number emitted

	Particle* particles; //list of all particles
	Particle* last_particle;


public:
	ParticleSystem(int);
	~ParticleSystem(void);

	void CreateParticle();
	void display();
	void updateParticle(Particle*);


};
