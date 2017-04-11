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
#include "Mat4f.h"
#include "OpenGLMatrixTools.h"

using namespace std;

// Defines the properties of a Boid

class Boid {
public:
  vector<Boid*> Boids;
  Boid() {};
  Boid(Vec3f pos);
  Vec3f getPos();
  void setPos(Vec3f newPos);
  Vec3f getForce();
  void setForce(Vec3f newForce);
  float getMass();
  void setMass(float newMass);
  Vec3f getVelocity();
  void setVelocity(Vec3f newVel);
  void resetForce();
  Mat4f getOrientation();
  void setOrientation(Mat4f newOrien);

private:
  Vec3f position;
  float mass;
  Vec3f velocity;
  Vec3f force;
  Mat4f orientation;
};

#endif // BOID_H
