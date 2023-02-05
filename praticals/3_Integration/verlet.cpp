#include <glm/glm.hpp>
#include "main.h"
using namespace std;
using namespace glm;

static dvec3 gravity = dvec3(0, -10.0, 0);

void UpdatePhysics_Verlet(const double t, const double dt) {
  for (size_t i = 0; i < balls3.size(); i++)
  {
    // *********************************
    // calculate velocity from current and previous position
	  balls3[i].velocity = balls3[i].position - balls3[i].position_prev;
    // set previous position to current position
	  balls3[i].position_prev = balls3[i].position;
    // position += v + a * (dt^2)
	  balls3[i].position += balls3[i].velocity + gravity * dt * dt;
    // *********************************

    if (balls3[i].position.y <= 0.0f) {
      //Note: We can't use: 
      // balls[i].velocity.y = -balls[i].velocity.y;
      // As verlet calculates it's own velocity, instead we must use impulses.
      // but our simple model can't support that yet, so let's just hack it
      balls3[i].position_prev = balls3[i].position + (balls3[i].position - balls3[i].position_prev);
    }
  }
}