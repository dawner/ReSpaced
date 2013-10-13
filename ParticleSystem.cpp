//---------------------------------------------------------------------------
//
// Simple particle system, 
// stored as linked list of Particle Struct objects for efficiency.
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
	sun_radius = 2;
	particle_size=0.03;
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
	emitter_position.x=-6;
	emitter_position.y=0;
	emitter_position.z=0;
	global_force.x=0;
	global_force.y=0;
	global_force.z=0;
	speed=1.0/100.0;
	speed_variation=0.01;
	life=30; 
	life_variation=20;
	colour.r=0.8;
	colour.g=0.35;
	colour.b=0;
	colour_variation.r=0.2;
	colour_variation.g=0.15;
	colour_variation.b=0;
	dir_variation.x=0.5;
	dir_variation.y=0; 
	dir_variation.z=0.5; 
	rot_variation.x=180;
	rot_variation.y=0;
	rot_variation.z=90;

	srand((unsigned)time(0)); 
}

ParticleSystem::~ParticleSystem(void) {
	delete[] particles;
}

void ParticleSystem::CreateParticle(){
	if (particles==NULL || particle_count==total_particles || (particle_count>0 && last_particle==NULL)){
		return; //particle list has no space
	}

	Particle* p = particles; //next particle to be assigned particle 
	particles=particles->next;

	if (last_particle!=NULL)
		last_particle->prev=p;
	p->next=last_particle;
	last_particle=p;

	p->position.x=0;
	p->position.y=0;
	p->position.z=0;
	p->rotation.x=rot_variation.x*((rand()%100)/100.0); //only allow positive x rotation, to face camera
	p->rotation.z=rot_variation.z*random();

	//direct straight up
	float base_dir[] = {0,1,0}; //directly up
	p->direction.x=base_dir[0]+dir_variation.x*random();
	p->direction.y=base_dir[1]+dir_variation.y*random();
	p->direction.z=base_dir[2]+dir_variation.z*random();
	//add a multiple of speed and random number between -1 and 1
	float p_speed=speed + speed_variation*random();
	p->direction.x=p->direction.x*p_speed;
	p->direction.y=p->direction.y*p_speed;
	p->direction.z=p->direction.z*p_speed;

	p->colour.r=colour.r+colour_variation.r*random();
	p->colour.g=colour.g+colour_variation.g*random();
	p->colour.b=colour.b+colour_variation.b*random();

	p->life = life + life_variation*random();
	++particle_count;
}

//Render all particles and then update their positions
void ParticleSystem::display(float rot_x, float rot_y){
	glDisable(GL_LIGHTING);

	Particle* p = last_particle;
	while(p!=NULL){
		glPushMatrix();
			glTranslatef(emitter_position.x,emitter_position.y,emitter_position.z);
			
			//reverse camera rotations, to keep particles on viewing side
			glRotatef(-rot_y, 0.0, 1.0, 0.0);
			glRotatef(-rot_x, 1.0, 0.0, 0.0);

			glRotatef(p->rotation.x,1,0,0);
			glRotatef(p->rotation.z,0,0,1);
			glTranslatef(0,sun_radius,0);
			glTranslatef(p->position.x, p->position.y, p->position.z);
			glColor3f(p->colour.r,p->colour.g,p->colour.b);
			
			//reverse particle rotations so billboards face camera
			glRotatef(-p->rotation.z,0,0,1);
			glRotatef(-p->rotation.x,1,0,0);

			//draw square as billboard
			glBegin(GL_QUADS);
				glVertex3f(particle_size, particle_size, 0);
				glVertex3f(-particle_size, particle_size, 0); 
				glVertex3f(-particle_size, -particle_size, 0); 
				glVertex3f(particle_size, -particle_size, 0); 
			glEnd();
		glPopMatrix();

		updateParticle(p);
		p = p->next;
	}
	p = last_particle;
	while(p!=NULL){
		p=removeDead(p);	
	}
	//draw sun itself
	glTranslatef(-6,0,0);
	glColor3f(0.8,0.4,0);
	glutSolidSphere(sun_radius,50, 50);
	glEnable(GL_LIGHTING);

}

/* Resets particle list to blank */
void ParticleSystem::killAll() {
	particles[0].prev=NULL;
	for (int i=1;i<total_particles;++i){
		particles[i-1].next = &particles[i];
		particles[i].prev = &particles[i-1];
	}
	particles[total_particles-1].next=NULL;
	last_particle=NULL;
	particle_count=0;
}

/* ------- PRIVATE METHODS -------- */

/* Retuns a random number between -1 and 1 */
float ParticleSystem::random(){
	return ((rand()%200)-100)/100.0;
}

/* Updates a particles position, direction and life */
void ParticleSystem::updateParticle(Particle* p) {
	p->position.x+=p->direction.x;
	p->position.y+=p->direction.y;
	p->position.z+=p->direction.z;

	p->direction.x+=global_force.x;
	p->direction.y+=global_force.y;
	p->direction.z+=global_force.z;

	p->life-=1;
}

/* Deletes an active particle, resetting prev/next pointers & global list pointers accordingly */
ParticleSystem::Particle* ParticleSystem::removeDead(Particle* p) {
	Particle* next = p->next;
	if (p->life<=0){ //kill this particle
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
	return next;
}

