/**
 *	Credit for starting/sample code goes to:
 *
 * Author:	Andrew Robert Owens
 * Email:	arowens [at] ucalgary.ca
 * Date:	January, 2017
 * Course:	CPSC 587/687 Fundamental of Computer Animation
 * Organization: University of Calgary
 *
 * Copyright (c) 2017
 *
 * File:	main.cpp
 *
 * Summary:
 *
 * This is a (very) basic program to
 * 1) load shaders from external files, and make a shader program
 * 2) make Vertex Array Object and Vertex Buffer Object for the boid
 *
 * take a look at the following sites for further readings:
 * opengl-tutorial.org -> The first triangle (New OpenGL, great start)
 * antongerdelan.net -> shaders pipeline explained
 * ogldev.atspace.co.uk -> good resource
 */

 /**
 *	Changes/Additions made by:
 *
 * Author:	Manorie Vachon
 * Email:	manorie.vachon@ucalgary.ca
 * Date:		April, 2017
 * Course:	CPSC 587/687 Fundamental of Computer Animation
 * Organization: University of Calgary
 *
 * Summary:
 *
 * This is a particle system program which implements
 * Reynold's model of schools, flocks and herds through
 * behavioural animation.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <chrono>
#include <limits>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include "ShaderTools.h"
#include "Vec3f.h"
#include "Mat4f.h"
#include "OpenGLMatrixTools.h"
#include "Camera.h"
#include "Boid.h"

using namespace std;

//==================== GLOBAL VARIABLES ====================//
/*	Put here for simplicity. Feel free to restructure into
*	appropriate classes or abstractions.
*/

const Vec3f GRAVITY = Vec3f(0,-9.81,0); // gravity

// Drawing Program
GLuint basicProgramID;

// Data needed for Boid
GLuint vaoID;
GLuint vertBufferID;
Mat4f M;

// Data needed for Box
GLuint ball_vaoID;
GLuint ball_vertBufferID;
Mat4f ball_M;

// Only one camera so only one view and perspective matrix are needed.
Mat4f V;
Mat4f P;

// Only one thing is rendered at a time, so only need one MVP
// When drawing different objects, update M and MVP = M * V * P
Mat4f MVP;

// Camera and viewing Stuff
Camera camera;
int g_moveUpDown = 0;
int g_moveLeftRight = 0;
int g_moveBackForward = 0;
int g_rotateLeftRight = 0;
int g_rotateUpDown = 0;
int g_rotateRoll = 0;
float g_rotationSpeed = 0.015625;
float g_panningSpeed = 0.25;
bool g_cursorLocked;
float g_cursorX, g_cursorY;

bool g_play = false;

int WIN_WIDTH = 800, WIN_HEIGHT = 600;
int FB_WIDTH = 800, FB_HEIGHT = 600;
float WIN_FOV = 60;
float WIN_NEAR = 0.01;
float WIN_FAR = 1000;

/*** Boid variables **/
vector<Vec3f> boidGeomPoints; // Points to draw (3/boid to make a triangle)
float rA = 0.f; // radius of avoidance
float rC = 0.f; // radius of cohesion
float rG = 0.f; // radius of gathering
float wA = 0.f; // weight of avoidance
float wC = 0.f; // weight of cohesion
float wG = 0.f; // weight of gathering
float Fmax = 0.f; // max force allowed
float Vmax = 0.f; // max velocity allowed
int numBoids = 0; // number of boids to be in the simulation

// Boid object
Boid b;

// Locations of instances
//vector<Vec3f> translations;

vector<Vec3f> sphere;

// Bounding box
float edge = 0; // from centre of box

//==================== FUNCTION DECLARATIONS ====================//
void displayFunc();
void resizeFunc();
void init();
void generateIDs();
void deleteIDs();
void setupVAO();
void loadBoidGeometryToGPU();
void loadBallGeometryToGPU();
void reloadProjectionMatrix();
void loadModelViewMatrix();
void setupModelViewProjectionTransform();

void windowSetSizeFunc();
void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
void windowMouseMotionFunc(GLFWwindow *window, double x, double y);
void windowSetSizeFunc(GLFWwindow *window, int width, int height);
void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height);
void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods);
void windowMouseMotionFunc(GLFWwindow *window, double x, double y);
void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods);
void animateBoid(float t);
void moveCamera();
void reloadMVPUniform();
void reloadColorUniform(float r, float g, float b);
std::string GL_ERROR();
int main(int, char **);

Vec3f clamp(Vec3f f, float fmax);
float favoid(float distance);
float fcohesion(float distance);
float fgather(float distance);
void keepInBounds(Boid* b);
void initBoids();
void getBoidGeomPoints();
void readFile(string filename);
void readObj(string filename);

//==================== FUNCTION DEFINITIONS ====================//

void displayFunc() {
  // Make background grayish
  glClearColor(0.4f, 0.f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Use our shader
  glUseProgram(basicProgramID);

  // ===== DRAW BOID ====== //
  MVP = P * V * M;
  reloadMVPUniform();
  reloadColorUniform(0.2f, 1.f, 0.2f);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(vaoID);
  // Draw Boids, start at vertex 0, draw 3 of them (for every boid)
  // Instancing
  // glDrawArraysInstanced(GL_TRIANGLES, 0, 3, translations.size());
  glDrawArrays(GL_TRIANGLES, 0, boidGeomPoints.size());
  glBindVertexArray(0);

  // ==== DRAW ball ===== //
  glPointSize(50);
  MVP = P * V * ball_M;
  reloadMVPUniform();

  reloadColorUniform(1, 1, 1);

  // Use VAO that holds buffer bindings
  // and attribute config of buffers
  glBindVertexArray(ball_vaoID);
  // Draw boxs
  glDrawArrays(GL_TRIANGLES, 0, sphere.size());
}

void loadBoidGeometryToGPU() {
  // instancing testing with hardcoded 2D positions of a box
  // float boidVertices[] = {
  //   // positions    // Colors
  //   -0.05f, 0.05f,  1.f, 0.f, 0.f,
  //   0.05f, -0.05f,  0.f, 1.f, 0.f,
  //   -0.05f, -0.05f,  0.f, 0.f, 1.f,
  //
  //   -0.05f, 0.05f,  1.f, 0.f, 0.f,
  //   0.05f, -0.05f,  0.f, 1.f, 0.f,
  //   0.05f, 0.05f,  0.f, 1.f, 1.f,
  // };


  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
        //       sizeof(boidVertices), // byte size of Vec3f
               sizeof(Vec3f) * boidGeomPoints.size(), // byte size of Vec3f
        //       boidVertices,      // pointer (Vec3f*) to contents of verts
               boidGeomPoints.data(),      // pointer (Vec3f*) to contents of verts
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}
/*
void loadTranslationsToGPU() { // no signature currently
//  for (int i = -numBoids/2; i < numBoids/2; i++) { // y
//    for (int j = -numBoids/2; j < numBoids/2; j++) { // x
  float x = float(-numBoids);
  float y = 0.f;

  for (int i = 0; i < numBoids; i++) { // x
      translations.push_back(Vec3f(x, y, 0.f));
      x += 2.f;
  }
//  }
glm::vec2 translations2[100];
int index = 0;
GLfloat offset = 0.1f;
Boid* boid;
b.Boids.clear();

for(GLint y = -10; y < 10; y += 2)
{
    for(GLint x = -10; x < 10; x += 2)
    {
        glm::vec2 translation;
        translation.x = (GLfloat)x / 10.0f + offset;
        translation.y = (GLfloat)y / 10.0f + offset;
        translations2[index++] = translation;

        boid = new Boid(Vec3f(translation.x,translation.y,0.f));
        b.Boids.push_back(boid);
    }
}

  glGenBuffers(1, &ball_vertBufferID); // replace with instanceVBO
  glBindBuffer(GL_ARRAY_BUFFER, ball_vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(glm::vec2) * 100, // byte size of Vec3f
               &translations2[0],      // pointer (Vec3f*) to contents of verts
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  cout << "after bind: " << index << endl;

}*/

void loadBallGeometryToGPU() {
  glBindBuffer(GL_ARRAY_BUFFER, ball_vertBufferID);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Vec3f) * sphere.size(), // byte size of Vec3f, 4 of them
               sphere.data(),      // pointer (Vec3f*) to contents of verts
               GL_STATIC_DRAW);   // Usage pattern of GPU buffer
}

void setupVAO() {
  glBindVertexArray(vaoID);

  // vertices of shape
  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        0,        // stride
            //  instancing test          5*sizeof(GL_FLOAT),        // stride
                        (void *)0 // array buffer offset
                        );
/* Stuff that was used to try instancing
  glEnableVertexAttribArray(1); // match layout # in shader
  glVertexAttribPointer(1,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        5*sizeof(GL_FLOAT),        // stride
                        (void *)(2*sizeof(GL_FLOAT)) // array buffer offset
                        );

  // Set instance data
  glEnableVertexAttribArray(2); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, ball_vertBufferID);
  glVertexAttribPointer(2,        // attribute layout # above
                        2,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        2*sizeof(GL_FLOAT),        // stride
                        (void *)0 // array buffer offset
                        );

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  cout << "getting here" << endl;

 glVertexAttribDivisor(2, 1); // Tell OpenGL this is an instanced vertex attribute.
 glBindVertexArray(0); */


  glBindVertexArray(ball_vaoID);

  glEnableVertexAttribArray(0); // match layout # in shader
  glBindBuffer(GL_ARRAY_BUFFER, ball_vertBufferID);
  glVertexAttribPointer(0,        // attribute layout # above
                        3,        // # of components (ie XYZ )
                        GL_FLOAT, // type of components
                        GL_FALSE, // need to be normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
                        );

  glBindVertexArray(0); // reset to default

}

void reloadProjectionMatrix() {
  // Perspective Only

  // field of view angle 60 degrees
  // window aspect ratio
  // near Z plane > 0
  // far Z plane

  P = PerspectiveProjection(WIN_FOV, // FOV
                            static_cast<float>(WIN_WIDTH) /
                                WIN_HEIGHT, // Aspect
                            WIN_NEAR,       // near plane
                            WIN_FAR);       // far plane depth
}

void loadModelViewMatrix() {
  M = IdentityMatrix();
  ball_M = IdentityMatrix();
  // view doesn't change, but if it did you would use this
  V = camera.lookatMatrix();
}

void reloadViewMatrix() { V = camera.lookatMatrix(); }

void setupModelViewProjectionTransform() {
  MVP = P * V * M; // transforms vertices from right to left (odd huh?) :) This is funny
}

void reloadMVPUniform() {
  GLint id = glGetUniformLocation(basicProgramID, "MVP");

  glUseProgram(basicProgramID);

  // Introduce instancing somewhere here:
  // Call glUniformMatrix4fv and then perform a new translate and rotation
  // matrix on the boid and then draw
  glUniformMatrix4fv(id,        // ID
                     1,         // only 1 matrix
                     GL_TRUE,   // transpose matrix, Mat4f is row major
                     MVP.data() // pointer to data in Mat4f
                     );
}

void reloadColorUniform(float r, float g, float b) {
  GLint id = glGetUniformLocation(basicProgramID, "inputColor");

  glUseProgram(basicProgramID);
  glUniform3f(id, // ID in basic_vs.glsl
              r, g, b);
}

void generateIDs() {
  // shader ID from OpenGL
  std::string vsSource = loadShaderStringfromFile("./shaders/basic_vs.glsl");
  std::string fsSource = loadShaderStringfromFile("./shaders/basic_fs.glsl");
  basicProgramID = CreateShaderProgram(vsSource, fsSource);
  // VAO and buffer IDs given from OpenGL
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vertBufferID); // VBO
  glGenVertexArrays(1, &ball_vaoID);
  glGenBuffers(1, &ball_vertBufferID);
}

void deleteIDs() {
  glDeleteProgram(basicProgramID);

  glDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &vertBufferID);
  glDeleteVertexArrays(1, &ball_vaoID);
  glDeleteBuffers(1, &ball_vertBufferID);
}

void init() {
  glEnable(GL_DEPTH_TEST);
  glPointSize(50);

  camera = Camera(Vec3f{0, 0, 90}, Vec3f{0, 0, -1}, Vec3f{0, 1, 0});

  // SETUP SHADERS, BUFFERS, VAOs
  generateIDs();
  setupVAO();

  initBoids();
  // The translations that were for instancing
  //  loadTranslationsToGPU() ;
  getBoidGeomPoints();
  loadBoidGeometryToGPU();
  loadBallGeometryToGPU();

  loadModelViewMatrix();
  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

int main(int argc, char **argv) {
  GLFWwindow *window;

  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window =
      glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "CPSC 587/687 Boid Simulation", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(window, windowSetSizeFunc);
  glfwSetFramebufferSizeCallback(window, windowSetFramebufferSizeFunc);
  glfwSetKeyCallback(window, windowKeyFunc);
  glfwSetCursorPosCallback(window, windowMouseMotionFunc);
  glfwSetMouseButtonCallback(window, windowMouseButtonFunc);

  glfwGetFramebufferSize(window, &WIN_WIDTH, &WIN_HEIGHT);

  // Initialize glad
  if (!gladLoadGL()) {
    std::cerr << "Failed to initialise GLAD" << std::endl;
    return -1;
  }

  std::cout << "GL Version: :" << glGetString(GL_VERSION) << std::endl;
  std::cout << GL_ERROR() << std::endl;

  // Read initial states and parameters
  readFile("boids1.txt");
  readObj("pokeball.obj");
  // Initialize all the geometry, and load it once to the GPU
  init();

  // Variables needed in loop
  float deltaT = 0.09f;
  int i;
  int j;
  float dist; // used for distance between two boids
  Vec3f dir; // used to hold the direction between two boids
  Vec3f Xi; // position of boid i
  Vec3f Xj; // position of boid j
  Vec3f F = Vec3f(0,0,0); // force being accumulated
  Vec3f V = Vec3f(0,0,0);
  Vec3f vNeighbours; // velocity of all neighbours
  Vec3f averageOfNeighbours = Vec3f(0,0,0);
  Vec3f Vc = Vec3f(0,0,0); // used as a current velocity placeholder
  float r = 0.f;
  int count = 0;
  Boid* boidi;

  // Main running window loop
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         !glfwWindowShouldClose(window)) {

    if (g_play) {
        for (i = 0; i < b.Boids.size(); i++) {
          vNeighbours.zero();
          count = 0;
          // get average of boids within rC
          for (j = i + 1; j < b.Boids.size(); j++) {
            Xi = b.Boids[i]->getPos();
            Xj = b.Boids[j]->getPos();
            dist = Xi.distance(Xj); // distance between the current pair
            if (dist > rA && dist < rC) {
              vNeighbours = vNeighbours + b.Boids[j]->getVelocity();
              count = count + 1;
            }
          }
          // go through every pair and accumulate forces
          for (j = i + 1; j < b.Boids.size(); j++) {
            Xi = b.Boids[i]->getPos();
            Xj = b.Boids[j]->getPos();
            dist = Xi.distance(Xj); // distance between the current pair

            if (dist <= 0) {
              F.zero();
            }
            else {
              dir = (Xi - Xj)/dist;
              if (dist < rA) {
                r = dist / rA; // try changing this
                F = favoid(dist) * dir;
              } else if (dist < rC) {
                if (count > 0) {
                  // if there are pairs within cohesion distance, use their average
                  averageOfNeighbours = vNeighbours/count;
                  averageOfNeighbours.normalized();
                  Vc = (averageOfNeighbours - b.Boids[i]->getVelocity());
                  F = fcohesion(dist) * Vc;
                } else {
                  F.zero(); // no one to match velocities with
                }
              } else if (dist < rG) {
                F = -fgather(dist) * dir;
              } else {
                F.zero(); // the two boids ignore each other
              }
            }

            b.Boids[i]->setForce(b.Boids[i]->getForce() + F); // add the total force to one boid
            b.Boids[j]->setForce(b.Boids[j]->getForce() - F); // subtract the total force from the other
          }
        }

        // go through every boid and update velocity and position
        for (i = 0; i < b.Boids.size(); i++) {
          boidi = b.Boids[i];
          F = boidi->getForce();
          F = clamp(F, Fmax); // change to Fmax read in
          // integrate
          // below, 1 is used as the mass for this simulation
          boidi->setVelocity(boidi->getVelocity() + (F/boidi->getMass())*deltaT); // F/m*dt gives new velocity
          V = boidi->getVelocity();
          V = clamp(V, Vmax);
          boidi->setVelocity(V);
          boidi->setPos(boidi->getPos() + (V*deltaT));
          keepInBounds(boidi);
          boidi->resetForce();
          // update (Mi);
        }
    }

    // Make geometry based on the current positions of all boids
    getBoidGeomPoints();
    loadBoidGeometryToGPU();
    //loadBallGeometryToGPU - to use later if getting the sphere to move

    displayFunc();
    moveCamera();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // clean up after loop
  deleteIDs();

  return 0;
}

Vec3f clamp(Vec3f f, float fmax) {
  if (f.x() > fmax) {
    f.x() = fmax;
  } else if (f.x() < -fmax) {
    f.x() = -fmax;
  }
  if (f.y() > fmax) {
    f.y() = fmax;
  } else if (f.y() < -fmax) {
    f.y() = -fmax;
  }
  if (f.z() > fmax) {
    f.z() = fmax;
  } else if (f.z() < -fmax) {
    f.z() = -fmax;
  }
  return f;
}

float favoid(float distance) {
  float r = distance / rA;
  if (distance <= 1) {
    return wA*1.f;
  } else {
    return wA * pow((1-r), 3) * (3*r + 1);
    // A different function that was used
    //wA * (1/(pow(distance, 2))); // 1/x^2 function
  }                                // return force value of function
}

float fcohesion(float distance) {
  return wC*(distance); // x function
}              // return force value of function

float fgather(float distance) {
  if (distance <= 1) {
    return wG*1.f;
  } else {
    return wG*(1/(pow(distance, 2))); // 1/x^2 function
  }                                // return force value of function
}

void keepInBounds(Boid* b) {
  Vec3f pos = b->getPos();
  Vec3f vel = b->getVelocity();

  if (pos.x() > edge) {
    pos.x() = edge-1;
    vel.x() = -vel.x();
  } else if (pos.x() < -edge) {
    pos.x() = -(edge-1);
    vel.x() = -vel.x();
  }

  if (pos.y() > edge) {
    pos.y() = edge-1;
    vel.y() = -vel.y();
  } else if (pos.y() < -edge) {
    pos.y() = -(edge-1);
    vel.y() = -vel.y();
  }

  if (pos.z() > edge) {
    pos.z() = edge-1;
    vel.z() = -vel.z();
  } else if (pos.z() < -edge) {
    pos.z() = -(edge-1);
    vel.z() = -vel.z();
  }
  b->setPos(pos);
  b->setVelocity(vel);
}

void initBoids() {
  Boid* boid;
  float spawn = edge -2;
  float x = -spawn;
  float y = spawn;
  float z = 0.f;

  for (int i = 0; i < numBoids; i++) {
    boid = new Boid(Vec3f(x,y,z));
    b.Boids.push_back(boid);
    x = x + 5.f;
    if (x >= spawn) { // put on the next row down
      x = -spawn;
      y = y - 5.f;
    }
  }

  for (int i = 0; i < b.Boids.size(); i++) {
    // get average of boids within rC
    for (int j = i + 1; j < b.Boids.size(); j++) {
      Vec3f Xi = b.Boids[i]->getPos();
      Vec3f Xj = b.Boids[j]->getPos();
      float dist = Xi.distance(Xj); // distance between the current pair
    }
  }
}

void getBoidGeomPoints() {
  boidGeomPoints.clear();
  Vec3f boidPos;
  Vec3f vel;

  for (int i = 0; i < b.Boids.size(); i++) {
    boidPos = b.Boids[i]->getPos();
    vel = b.Boids[i]->getVelocity();

    boidGeomPoints.push_back(Vec3f(boidPos.x()-0.5f, boidPos.y(), boidPos.z()));
    boidGeomPoints.push_back(Vec3f(boidPos.x()+1.f, boidPos.y()+0.5f, boidPos.z()));
    boidGeomPoints.push_back(Vec3f(boidPos.x()+1.f, boidPos.y()-0.5f, boidPos.z()));
    // Push the nose (x position) out 0.5 to the left (x-0.5),
    // and the tail points 1 back (x+1) and 0.5 up (y+0.5) and
    // down (y-0.5), respectively
  }
}

void readFile(string filename) {
  ifstream file(filename);
  char input;

  if (file.is_open()) {
    file >> input;

    while (!file.eof()) {
      if (input == 'N') {
        file >> numBoids;
      } else if(input == 'A') {
          file >> rA;
      } else if(input == 'C') {
          file >> rC;
      } else if(input == 'G') {
          file >> rG;
      }  else if(input == 'F') {
          file >> Fmax;
      } else if(input == 'V') {
          file >> Vmax;
      } else if(input == 'D') {
          file >> wA;
      } else if(input == 'H') {
          file >> wC;
      } else if(input == 'T') {
          file >> wG;
      } else if(input == 'E') {
          file >> edge;
      }
      file >> input;
    }
    file.close();
  }
  else {
    cout << "Unable to open file!" << endl;
  }
}

void readObj(string filename) {
  ifstream file(filename);
  char input;
  float x;
  float y;
  float z;

  if (file.is_open()) {
    file >> input;

    while (!file.eof()) {
      if (input == 'v') {
        file >> x;
        file >> y;
        file >> z;
        x *= 0.1f;
        y *= 0.1f;
        z *= 0.1f;

        sphere.push_back(Vec3f(x,y,z));
      }
      file >> input;
    }
    file.close();
  }
  else {
    cout << "Unable to open file!" << endl;
  }
}

//==================== CALLBACK FUNCTIONS ====================//

void windowSetSizeFunc(GLFWwindow *window, int width, int height) {
  WIN_WIDTH = width;
  WIN_HEIGHT = height;

  reloadProjectionMatrix();
  setupModelViewProjectionTransform();
  reloadMVPUniform();
}

void windowSetFramebufferSizeFunc(GLFWwindow *window, int width, int height) {
  FB_WIDTH = width;
  FB_HEIGHT = height;

  glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
}

void windowMouseButtonFunc(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      g_cursorLocked = GL_TRUE;
    } else {
      g_cursorLocked = GL_FALSE;
    }
  }
}

void windowMouseMotionFunc(GLFWwindow *window, double x, double y) {
  if (g_cursorLocked) {
    float deltaX = (x - g_cursorX) * 0.01;
    float deltaY = (y - g_cursorY) * 0.01;
    camera.rotateAroundFocus(deltaX, deltaY);

    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }

  g_cursorX = x;
  g_cursorY = y;
}

void windowKeyFunc(GLFWwindow *window, int key, int scancode, int action,
                   int mods) {
  bool set = action != GLFW_RELEASE && GLFW_REPEAT;
  switch (key) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_W:
    g_moveBackForward = set ? 1 : 0;
    break;
  case GLFW_KEY_S:
    g_moveBackForward = set ? -1 : 0;
    break;
  case GLFW_KEY_A:
    g_moveLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_D:
    g_moveLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_Q:
    g_moveUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_E:
    g_moveUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_UP:
    g_rotateUpDown = set ? -1 : 0;
    break;
  case GLFW_KEY_DOWN:
    g_rotateUpDown = set ? 1 : 0;
    break;
  case GLFW_KEY_LEFT:
    if (mods == GLFW_MOD_SHIFT)
      g_rotateRoll = set ? -1 : 0;
    else
      g_rotateLeftRight = set ? 1 : 0;
    break;
  case GLFW_KEY_RIGHT:
    if (mods == GLFW_MOD_SHIFT)
      g_rotateRoll = set ? 1 : 0;
    else
      g_rotateLeftRight = set ? -1 : 0;
    break;
  case GLFW_KEY_SPACE:
    g_play = set ? !g_play : g_play;
    break;
  case GLFW_KEY_LEFT_BRACKET:
    if (mods == GLFW_MOD_SHIFT) {
      g_rotationSpeed *= 0.5;
    } else {
      g_panningSpeed *= 0.5;
    }
    break;
  case GLFW_KEY_RIGHT_BRACKET:
    if (mods == GLFW_MOD_SHIFT) {
      g_rotationSpeed *= 1.5;
    } else {
      g_panningSpeed *= 1.5;
    }
    break;
  default:
    break;
  }
}

//==================== OPENGL HELPER FUNCTIONS ====================//

void moveCamera() {
  Vec3f dir;

  if (g_moveBackForward) {
    dir += Vec3f(0, 0, g_moveBackForward * g_panningSpeed);
  }
  if (g_moveLeftRight) {
    dir += Vec3f(g_moveLeftRight * g_panningSpeed, 0, 0);
  }
  if (g_moveUpDown) {
    dir += Vec3f(0, g_moveUpDown * g_panningSpeed, 0);
  }

  if (g_rotateUpDown) {
    camera.rotateUpDown(g_rotateUpDown * g_rotationSpeed);
  }
  if (g_rotateLeftRight) {
    camera.rotateLeftRight(g_rotateLeftRight * g_rotationSpeed);
  }
  if (g_rotateRoll) {
    camera.rotateRoll(g_rotateRoll * g_rotationSpeed);
  }

  if (g_moveUpDown || g_moveLeftRight || g_moveBackForward ||
      g_rotateLeftRight || g_rotateUpDown || g_rotateRoll) {
    camera.move(dir);
    reloadViewMatrix();
    setupModelViewProjectionTransform();
    reloadMVPUniform();
  }
}

std::string GL_ERROR() {
  GLenum code = glGetError();

  switch (code) {
  case GL_NO_ERROR:
    return "GL_NO_ERROR";
  case GL_INVALID_ENUM:
    return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "GL_INVALID_VALUE";
  case GL_INVALID_OPERATION:
    return "GL_INVALID_OPERATION";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "GL_OUT_OF_MEMORY";
  default:
    return "Non Valid Error Code";
  }
}
