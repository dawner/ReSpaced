//---------------------------------------------------------------------------
//
// Copyright (c) 2013 Dawn Richardson 
//
//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "ParticleSystem.h"


ParticleSystem::ParticleSystem(int num_particles) {
	total_particles=num_particles;
	//set up linked list of particles
	particles = (Particle*) malloc(sizeof(Particle)*total_particles);
	particles[0].prev=NULL;
	for (int i=1;i<total_particles;++i){
		particles[i-1].next = &particles[i];
		particles[i].prev = &particles[i-1];
	}
	particles[total_particles-1].next=NULL;
	last_particle=NULL;
	particle_count=0;

	//set emitter values
	emitter_position.x=0;
	emitter_position.y=0;
	emitter_position.z=0;
	global_force.x=0;
	global_force.y=0;
	global_force.z=0;
	speed=0.2;
	speed_variation=0.1;
	life=50; //50 'ticks'
	life_variation=10;
	colour.r=255;
	colour.g=160;
	colour.b=0;
	colour_variation.r=10;
	colour_variation.g=5;
	colour_variation.b=0;

	srand((unsigned)time(0)); 
}

ParticleSystem::~ParticleSystem(void) {
	delete[] particles;
}

void ParticleSystem::CreateParticle(){
	if (particles==NULL && last_particle!=NULL && particle_count<total_particles){
		return; //particle list is full
	}
	Particle* p = particles; //next particle to be assigned particle 
	particles=particles->next;

	if (last_particle!=NULL)
		last_particle->prev=p;
	p->next=last_particle;
	p->prev=NULL;
	last_particle=p;

	p->position.x=emitter_position.x;
	p->position.y=emitter_position.y;
	p->position.z=emitter_position.z;

	//direct straight up
	//TODO make this angled randomly
	p->direction.x=0;
	p->direction.y=1;
	p->direction.z=0;
	//add a multiple of speed and random number between -1 and 1
	float p_speed=speed + speed_variation*(((rand()%200)-100)/100.0);
	p->direction.x=p->direction.x*p_speed;
	p->direction.y=p->direction.y*p_speed;
	p->direction.z=p->direction.z*p_speed;

	p->colour.r+=colour.r+colour_variation.r*(((rand()%200)-100)/100.0);
	p->colour.g+=colour.g+colour_variation.g*(((rand()%200)-100)/100.0);
	p->colour.b+=colour.b+colour_variation.b*(((rand()%200)-100)/100.0);

	p->life = life + life_variation*(((rand()%200)-100)/100.0);

	++particle_count;
}


//Render all particles and then update their positions
void ParticleSystem::display(){
	Particle* p = last_particle;
	while(p!=NULL){
		if (p->life>0){
			glPushMatrix();
				glTranslatef(p->position.x, p->position.y, p->position.z);
				glColor3f(p->colour.r,p->colour.g,p->colour.b);
				glutSolidSphere(0.05,10, 10);
			glPopMatrix();
			updateParticle(p);
		}
		p = p->next;
	}
}

void ParticleSystem::updateParticle(Particle* p) {
	p->position.x+=p->direction.x;
	p->position.y+=p->direction.y;
	p->position.z+=p->direction.z;

	p->direction.x+=global_force.x;
	p->direction.y+=global_force.y;
	p->direction.z+=global_force.z;

	p->life-=1;
	if (p->life==0){ //kill this particle
		if (p->prev!=NULL)
			p->prev->next=p->next;
		else
			last_particle=p->next;

		if (p->next!=NULL)
			p->next->prev=p->prev;

		p->next=particles;
		particles=p;

		particle_count--;
	}

}
