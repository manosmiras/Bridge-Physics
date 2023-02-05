#pragma once
#include "game.h"
// Physics component
class cPhysics : public Component {
public:
  cPhysics();
  ~cPhysics();
  glm::dvec3 position; // Current position
  glm::dvec3 prev_position; // Previous position
  glm::dvec3 forces; // Total forces
  glm::dvec3 velocity; // Velocity
  float fake; // If fake, no physics are applied
  double mass;
  virtual void Update(double delta);
  virtual void SetParent(Entity *p);
  // Adds an impulse
  virtual void AddImpulse(const glm::vec3 &i);

private:
};
// Plank class, has a reference to neighbouring particles
class cPlank : public cPhysics {
public:
	cPhysics *leftParticle;
	cPhysics *rightParticle;
	cPlank();
	~cPlank();
	void Update(double delta);

private:
};

class cParticle : public cPhysics {
public:
  cParticle();
  ~cParticle();
  void Update(double delta);

private:
};

class cRigidBody : public cPhysics {
public:
  cRigidBody();
  ~cRigidBody();
  void Update(double delta);

private:
};
// Collider
class cCollider : public Component {
public:
  cCollider(const std::string &tag);
  ~cCollider();
  void Update(double delta);

private:
};
// Sphere collider
class cSphereCollider : public cCollider {

public:
  double radius;
  cSphereCollider();
  ~cSphereCollider();

private:
};
// Plane collider
class cPlaneCollider : public cCollider {
public:
  glm::dvec3 normal;
  cPlaneCollider();
  ~cPlaneCollider();

private:
};
// AABB collider
class cAABB : public cCollider {
public:
	glm::dvec3 centerPoint; // Center point of AABB
	double radius[3]; // Radius - Halfwidth extents (dx, dy, dz)
	cAABB();
	cAABB(glm::dvec3 centerPoint, double radius[3]);
	~cAABB();
private:
};

struct collisionInfo {
  const cCollider *c1;
  const cCollider *c2;
  const glm::dvec3 position;
  const glm::dvec3 normal;
  const double depth;
};
// A spring connection
class ParticleSpring
{
	/** The particle at the other end of the spring. */
	cPhysics *other;

	/** Holds the sprint constant. */
	float springConstant;

	/** Holds the rest length of the spring. */
	float restLength;

public:

	/** Creates a new spring with the given parameters. */
	ParticleSpring(cPhysics *other, float springConstant, float restLength);
	ParticleSpring();

	/** Applies the spring force to the given particle. */
	virtual void updateForce(cPhysics *particle, float duration);
};
// Used to apply damping force to particles
class ParticleDrag
{
	/** Holds the velocity drag coeffificent. */
	float k1;

	/** Holds the velocity squared drag coeffificent. */
	float k2;

public:

	ParticleDrag();

	/** Creates the generator with the given coefficients. */
	ParticleDrag(float k1, float k2);

	/** Applies the drag force to the given particle. */
	virtual void updateForce(cPhysics *particle, float duration);
};


void InitPhysics();
void ShutdownPhysics();
void UpdatePhysics(const double t, const double dt, bool collisionsEnabled);
void UpdatePhysicsRK4(const double t, const double dt);
