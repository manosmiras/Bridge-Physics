#include <glm/glm.hpp>
#include "main.h"
using namespace std;
using namespace glm;

static dvec3 gravity = dvec3(0, -10.0, 0);

void UpdatePhysics_Euler(const double t, const double dt) {
  for (size_t i = 0; i < balls2.size(); i++)
  {
    // *********************************
    // Apply Accleration to Velocity
	  balls2[i].velocity = gravity * dt;
    // Apply Velocity to position
	  balls2[i].position += balls[i].velocity * dt;
    // *********************************
    
    //super simple bounce function
    if (balls2[i].position.y <= 0.0f) {
      balls2[i].velocity.y = -balls2[i].velocity.y;
    }
  }
}