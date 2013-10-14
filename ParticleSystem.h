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
		G308_Point rotation;
		G308_Vector direction;

		int life;
		G308_RGBA colour;

		bool is_flare;
		G308_Point* past_positions;

		//store links to particles on either side
		Particle* next; 
		Particle* prev;

	} Particle;

	float sun_radius;
	float particle_size;

	//emitter variables
	G308_Point emitter_position;
	G308_Vector global_force;
	float speed;
	float speed_variation;
	int life;
	int life_variation;
	G308_RGBA colour;
	G308_RGBA colour_variation;
	G308_Vector dir_variation;
	G308_Point rot_variation;

	int total_particles; //maximum particle number from space allocated
	int particle_count; //current number emitted

	int flare_n; //every n particles is a flare
	int flare_tail; //num of past positions to store for tail
	G308_RGBA flare_colour;
	G308_RGBA flare_colour_variation;
	G308_Point** tails; //pre-allocated memory for the flare tails
	int tail_pointer;

	Particle* particles; //list of all particles
	Particle* last_particle;

	float random();
	void positionParticle(Particle*, float, float);
	void drawParticle(Particle*);
	void drawFlare(Particle*);
	void updateParticle(Particle*);
	Particle* removeDead(Particle*);

public:
	ParticleSystem(int);
	~ParticleSystem(void);

	void CreateParticle();
	void display(float,float);

	void killAll();

};
