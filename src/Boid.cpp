/**
 * Author:	Manorie Vachon
 * Course:	CPSC 587 Fundamentals of Computer Animation
 */

#include "Boid.h"

// ======================== CONSTRUCTORS ============================//
Boid::Boid(Vec3f pos) {
  position = pos;
  mass = 1.f;
  velocity = Vec3f(0,0,0);
  force = Vec3f(0,0,0);
  orientation = IdentityMatrix();
}
// ==========================================================================//

// ========================= OPERATORS ======================================//
Vec3f Boid::getPos() {
  return position;
}

void Boid::setPos(Vec3f newPos) {
  position = newPos;
}

Vec3f Boid::getForce() {
  return force;
}

void Boid::setForce(Vec3f newForce) {
  force = newForce;
}

float Boid::getMass() {
  return mass;
}

void Boid::setMass(float newMass) {
  mass = newMass;
}

Vec3f Boid::getVelocity() {
  return velocity;
}

void Boid::setVelocity(Vec3f newVel) {
  velocity = newVel;
}

void Boid::resetForce() {
  force = Vec3f(0,0,0);
}

Mat4f Boid::getOrientation() {
  return orientation;
}

void Boid::setOrientation(Mat4f newOrien) {
  orientation = newOrien;
}

// ==========================================================================//
