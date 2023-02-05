#pragma once
#include "physics.h"
#include <glm/glm.hpp>


using namespace std;
using namespace glm;

class rope
{
public:
	vector<ParticleSpring> springs;

	// Creates particles, in a straight line
	static unique_ptr<Entity> CreateParticle(int x, int y, int z, bool horizontal, int fixedPoints);

	// Update the spring forces of a bridge (Not used, done in main)
	static void updateBridgeSpringForces(vector<ParticleSpring> springs1, vector<ParticleSpring> springs2, vector<ParticleSpring> springs3,
		vector<unique_ptr<Entity>> particlePositions1, vector<unique_ptr<Entity>> particlePositions2, bool cut);

	// Creates planks inbetween ropes
	static unique_ptr<Entity> CreatePlanks();

};

