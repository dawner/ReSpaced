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


ParticleSystem::ParticleSystem(int num_particles, float star_dis) {
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
	emitter_position.x=0;
	emitter_position.y=0;
	emitter_position.z=0;
	global_force.x=0;
	// global_force.y=0;
	global_force.y=-1.0/1000.0;
	global_force.z=0;
	speed=1.0/100.0;
	speed_variation=0.01;
	life=30; 
	life_variation=20;
	colour.r=0.9;
	colour.g=0.4;
	colour.b=0;
	colour_variation.r=0.1;
	colour_variation.g=0.2;
	colour_variation.b=0;
	dir_variation.x=0.5;
	dir_variation.y=0; 
	dir_variation.z=0.5; 
	rot_variation.x=180;
	rot_variation.y=0;
	rot_variation.z=90;

	flare_n = 100;
	flare_tail=40;
	flare_colour.r=0.95;
	flare_colour.g=0.15;
	flare_colour.b=0;
	flare_colour_variation.r=0.1;
	flare_colour_variation.g=0.1;
	flare_colour_variation.b=0;
	int max_num_flares = 1+((total_particles-1)/flare_n); //ceiling
	tails = (G308_Point**) malloc(sizeof(G308_Point*)*max_num_flares);
	for (int i=0;i<max_num_flares;++i)
		tails[i] = (G308_Point*) malloc(sizeof(G308_Point)*flare_tail);
	tail_pointer=0;

	srand((unsigned)time(0)); 

	//set up stars
	num_stars = 2000;
	star_distance=star_dis;
	stars = (G308_Point*) malloc(sizeof(G308_Point)*num_stars);
	for (int i=0;i<num_stars;++i){
		stars[i].x=180.0*random(); //only allow negative x rotation, to face camera
		stars[i].y=0;
		stars[i].z=180.0*random();
	}
}

ParticleSystem::~ParticleSystem(void) {
	delete[] particles;
}

/* If space, creates a new particle, with random variables. 
/ Every flare_n-th particle is marked as a flare */
void ParticleSystem::CreateParticle(){
	if (particles==NULL || particle_count==total_particles || (particle_count>0 && last_particle==NULL)){
		return; //particle list has no space
	}

	Particle* p = particles; //next particle to be assigned 
	particles=particles->next;

	last_particle=p;

	p->position.x=0;
	p->position.y=0;
	p->position.z=0;
	p->rotation.x=rot_variation.x*((rand()%100)/100.0); //only allow positive x rotation, to face camera
	p->rotation.z=rot_variation.z*random();

	float base_dir[] = {0,1,0}; //directly up
	p->direction.x=base_dir[0]+dir_variation.x*random();
	p->direction.y=base_dir[1]+dir_variation.y*random();
	p->direction.z=base_dir[2]+dir_variation.z*random();


	if (particle_count%flare_n==0){ //should be made as flare!

		float p_speed=(speed + speed_variation*random())*1.6;
		p->direction.x=p->direction.x*p_speed;
		p->direction.y=p->direction.y*p_speed;
		p->direction.z=p->direction.z*p_speed;

		p->colour.r=flare_colour.r+flare_colour_variation.r*random();
		p->colour.g=flare_colour.g+flare_colour_variation.g*random();
		p->colour.b=flare_colour.b+flare_colour_variation.b*random();

		p->life = (life*2.8 + life_variation*random());

		p->is_flare=true;
		p->past_positions=tails[tail_pointer];
		tail_pointer+=1;
		int max_num_flares = 1+((total_particles-1)/flare_n); //ceiling
		if (tail_pointer==max_num_flares)
			tail_pointer=0;
		for(int i=0;i<flare_tail;++i){
			p->past_positions[i].x=0;
			p->past_positions[i].y=0;
			p->past_positions[i].z=0;
		}

	}else{ //normal plasma particle

		float p_speed=speed + speed_variation*random();
		p->direction.x=p->direction.x*p_speed;
		p->direction.y=p->direction.y*p_speed;
		p->direction.z=p->direction.z*p_speed;

		p->colour.r=colour.r+colour_variation.r*random();
		p->colour.g=colour.g+colour_variation.g*random();
		p->colour.b=colour.b+colour_variation.b*random();

		p->life = life + life_variation*random();
		p->is_flare=false;
		p->past_positions=NULL;
	}
	++particle_count;

}


//Render all particles and then update their positions
void ParticleSystem::display(float rot_x, float rot_y){
	glDisable(GL_LIGHTING);

	//draw sun itself
	glTranslatef(emitter_position.x, emitter_position.y, emitter_position.z);
	glColor3f(0.8,0.45,0);
	glutSolidSphere(sun_radius,50, 50);
	glTranslatef(-emitter_position.x, -emitter_position.y, -emitter_position.z);

	Particle* p = last_particle;
	while(p!=NULL){
		glPushMatrix();
			drawParticle(p,rot_x, rot_y);
		glPopMatrix();

		updateParticle(p);
		p = p->prev;
	}
	p = last_particle;
	while(p!=NULL){
		p=removeDead(p);	
	}
	drawStars(rot_x,rot_y);
	glEnable(GL_LIGHTING);

}

/* Resets particle list to blank */
void ParticleSystem::killAll() {
	while(last_particle!=NULL){
		last_particle->life=0;
		removeDead(last_particle);
	}
}


/* ------- PRIVATE METHODS -------- */

/* Retuns a random number between -1 and 1 */
float ParticleSystem::random(){
	return ((rand()%200)-100)/100.0;
}

/*Performs all necessary transformations to get the particle into position 
/ and facing the correct direction, then draws the actual particle shape  */
void ParticleSystem::drawParticle(Particle* p,float rot_x, float rot_y){
	glTranslatef(emitter_position.x,emitter_position.y,emitter_position.z);
			
	//reverse camera rotations, to keep particles on viewing side
    glRotatef(-rot_y, 0.0, 1.0, 0.0);
    glRotatef(-rot_x, 1.0, 0.0, 0.0);		

	glRotatef(p->rotation.x,1,0,0);
	glRotatef(p->rotation.z,0,0,1);
	glTranslatef(0,sun_radius,0);
	glTranslatef(p->position.x, p->position.y, p->position.z);

	//reverse rotations so billboards face camera
	glRotatef(-p->rotation.z,0,0,1);
	glRotatef(-p->rotation.x,1,0,0);

	glColor3f(p->colour.r,p->colour.g,p->colour.b);

	if (p->is_flare){
		glutSolidSphere(particle_size*2.0,10, 10);

		glRotatef(p->rotation.x,1,0,0);
		glRotatef(p->rotation.z,0,0,1);
		glTranslatef(-p->position.x, -p->position.y, -p->position.z);

		drawFlare(p,rot_x, rot_y);
			
	}else{
		//draw square as billboard
		glBegin(GL_QUADS);
			glVertex3f(particle_size, particle_size, 0);
			glVertex3f(-particle_size, particle_size, 0); 
			glVertex3f(-particle_size, -particle_size, 0); 
			glVertex3f(particle_size, -particle_size, 0); 
		glEnd();
	}


}

/* Draw solar flare's tail                     */
void ParticleSystem::drawFlare(Particle* p,float rot_x, float rot_y){
	float alpha_delta = 0.6/flare_tail;
	float cur_alpha = 0.6;
	G308_Vector dir;
	dir.x =p->direction.x-global_force.x;
	dir.y = p->direction.y-global_force.y;
	dir.z = p->direction.z-global_force.z;
	G308_Point pos;
	pos.x = p->position.x-dir.x;
	pos.y = p->position.y-dir.y;
	pos.z = p->position.z-dir.z;
	for (int i=0;i<flare_tail;++i){
		dir.x = dir.x-global_force.x;
		dir.y = dir.y-global_force.y;
		dir.z = dir.z-global_force.z;
		pos.x = pos.x-dir.x;
		pos.y = pos.y-dir.y;
		pos.z = pos.z-dir.z;
		glColor4f(p->colour.r,p->colour.g,p->colour.b,cur_alpha);
		glTranslatef(pos.x, pos.y, pos.z);
		glutSolidSphere(particle_size*2.0,5, 5);

		glTranslatef(-pos.x, -pos.y, -pos.z);
		cur_alpha-=alpha_delta;
	}
}

/* Updates a particles position, direction and life */
void ParticleSystem::updateParticle(Particle* p) {

	if (p->is_flare){
		//update old positions
		for(int i=0;i<flare_tail-1;++i){
			p->past_positions[i] = p->past_positions[i+1];
		}
		p->past_positions[flare_tail-1].x = p->position.x;
		p->past_positions[flare_tail-1].y = p->position.y;
		p->past_positions[flare_tail-1].z = p->position.z;
	}

	p->position.x+=p->direction.x;
	p->position.y+=p->direction.y;
	p->position.z+=p->direction.z;

	if(p->is_flare){
		p->direction.x+=global_force.x;
		p->direction.y+=global_force.y;
		p->direction.z+=global_force.z;
	}

	p->life-=1;

}

/* Deletes an active particle, resetting prev/next pointers & global list pointers accordingly */
ParticleSystem::Particle* ParticleSystem::removeDead(Particle* p) {
	Particle* prev = p->prev;
	if (p->life<=0){ 

		if (&(*last_particle)==&(*p)){ 
			//removing last particle, just update last_particle pointer
			last_particle=p->prev;
		}else{	
			//remove p from current position
			if (p->prev!=NULL)
				p->prev->next=p->next;
			if (p->next!=NULL)
				p->next->prev=p->prev;

			//move p into position before particles
			p->next=particles;
			p->prev=last_particle;
			if (last_particle!=NULL)	
				last_particle->next=p;
			if (particles!=NULL)
				particles->prev=p;
		}
		particles=p;
		particle_count--;
	}
	return prev;
}

/* Draw the background             */
void ParticleSystem::drawStars(float rot_x, float rot_y){
	for (int i=0;i<num_stars;++i){
		glPushMatrix();
			glRotatef(stars[i].x,1,0,0);
			glRotatef(stars[i].z,0,0,1);
			glTranslatef(0,star_distance,0);
			glColor3f(1,1,1);
			//reverse particle rotations so billboards face camera
			glRotatef(-stars[i].z,0,0,1);
			glRotatef(-stars[i].x,1,0,0);

			glBegin(GL_QUADS);
				glVertex3f(particle_size, particle_size, 0);
				glVertex3f(-particle_size, particle_size, 0); 
				glVertex3f(-particle_size, -particle_size, 0); 
				glVertex3f(particle_size, -particle_size, 0); 
			glEnd();
		glPopMatrix();

	}
}