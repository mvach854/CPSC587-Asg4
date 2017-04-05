/**
 * Author:	Manorie Vachon
 * Course:	CPSC 587 Fundamentals of Computer Animation
 *
 * Definition for a boid particle
 */

#ifndef BOID_H
#define BOID_H

#include <iostream>
#include "Vec3f.h"
#include <vector>

using namespace std;

// Defines the properties of a Boid

class Boid {
public:
  vector<Boid*> Boids;
  Boid() {};
  Boid(float m, Vec3f pos);
  Vec3f getPos();
  void setPos(Vec3f newPos);
  Vec3f getForce();
  void setForce(Vec3f newForce);
  Vec3f getVelocity();
  void setVelocity(Vec3f newVel);
  float getBoid();
  void resetForce();

private:
  float Boid;
  Vec3f position;
  Vec3f velocity;
  Vec3f force;
};

#endif // BOID_H
