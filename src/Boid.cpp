/**
 * Author:	Manorie Vachon
 * Course:	CPSC 587 Fundamentals of Computer Animation
 */

#include "Boid.h"

// ======================== CONSTRUCTORS ============================//
Boid::Boid(float m, Vec3f pos) {
  Boid = m;
  position = pos;
  velocity = Vec3f(0,0,0);
  force = Vec3f(0,0,0);
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

Vec3f Boid::getVelocity() {
  return velocity;
}

void Boid::setVelocity(Vec3f newVel) {
  velocity = newVel;
}

float Boid::getBoid() {
  return Boid;
}

void Boid::resetForce() {
  force = Vec3f(0,0,0);
}

// ==========================================================================//
