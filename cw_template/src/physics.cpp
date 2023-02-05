#include "physics.h"
#include "collision.h"
#include <glm/glm.hpp>
using namespace std;
using namespace glm;
static vector<cPhysics *> physicsScene;
static vector<cCollider *> colliders;

static dvec3 gravity = dvec3(0, -10.0, 0);
static dvec3 initVelocity = dvec3(0.0, 0.0, 0.0);
bool first = true;

void Resolve(const collisionInfo &ci) {

	const double coef = 0.5;
	// Handle collisions for planks
	if (ci.c1->GetParent()->GetName() == "Plank")
	{
		auto a = ci.c1->GetParent()->GetComponents("Physics");
		if (a.size() == 1)
		{
			const auto p = static_cast<cPlank *>(a[0]);
			// Left particle
			p->leftParticle->AddImpulse(dvec3(300) * ci.normal * p->mass);
			//p->leftParticle->position += ci.normal * (ci.depth * 0.5);
			//const double currentSpeedLeft = glm::length(p->leftParticle->position - p->leftParticle->prev_position);
			//p->leftParticle->prev_position = p->leftParticle->position + dvec3(-ci.normal * currentSpeedLeft * coef);

			// Right particle
			p->rightParticle->AddImpulse(dvec3(300) * ci.normal * p->mass);
			//p->rightParticle->position += ci.normal * (ci.depth * 0.5);
			//const double currentSpeedRight = glm::length(p->rightParticle->position - p->rightParticle->prev_position);
			//p->rightParticle->prev_position = p->rightParticle->position + dvec3(-ci.normal * currentSpeedRight * coef);
		}
	}
	if (ci.c2->GetParent()->GetName() == "Plank")
	{
		auto b = ci.c2->GetParent()->GetComponents("Physics");
		if (b.size() == 1)
		{
			auto p = static_cast<cPlank *>(b[0]);
			// Left particle
			p->leftParticle->AddImpulse(dvec3(300) * -ci.normal);

			//p->leftParticle->position += -ci.normal * (ci.depth * 0.5 * 0.1);
			//const double currentSpeedLeft = glm::length(p->leftParticle->position - p->leftParticle->prev_position);
			//p->leftParticle->prev_position = p->leftParticle->position + dvec3(ci.normal * currentSpeedLeft * coef);
			
			// Right particle
			p->rightParticle->AddImpulse(dvec3(300) * -ci.normal);

			//p->rightParticle->position += -ci.normal * (ci.depth * 0.5 * 0.1);
			//const double currentSpeedRight = glm::length(p->rightParticle->position - p->rightParticle->prev_position);
			//p->rightParticle->prev_position = p->rightParticle->position + dvec3(ci.normal * currentSpeedRight * coef);
		}
	}
	// Handle other collisions
	if (!((ci.c1->GetParent()->GetName() == "Player" && ci.c1->GetParent()->GetName() == "Plank") || 
		(ci.c1->GetParent()->GetName() == "Plank" && ci.c1->GetParent()->GetName() == "Player")))
	{
		auto a = ci.c1->GetParent()->GetComponents("Physics");
		if (a.size() == 1)
		{
			const auto p = static_cast<cPhysics *>(a[0]);
			p->position += ci.normal * (ci.depth * 0.5);
			const double currentSpeed = glm::length(p->position - p->prev_position);
			p->prev_position = p->position + dvec3(-ci.normal * currentSpeed * coef);
		}
		auto b = ci.c2->GetParent()->GetComponents("Physics");
		if (b.size() == 1)
		{
			const auto p = static_cast<cPhysics *>(b[0]);
			p->position += -ci.normal * (ci.depth * 0.5 * 0.1);
			const double currentSpeed = glm::length(p->position - p->prev_position);
			p->prev_position = p->position + dvec3(ci.normal * currentSpeed * coef);
		}
	}
}

cPhysics::cPhysics() : forces(dvec3(0)), mass(1.0), Component("Physics") { physicsScene.push_back(this); }

cPhysics::~cPhysics() {
  auto position = std::find(physicsScene.begin(), physicsScene.end(), this);
  if (position != physicsScene.end()) {
    physicsScene.erase(position);
  }
}

void cPhysics::Update(double delta) {
  for (auto &e : physicsScene) {
    e->GetParent()->SetPosition(e->position);
  }
}

void cPhysics::SetParent(Entity *p) {
  Component::SetParent(p);
  position = Ent_->GetPosition();
  prev_position = position;
}

void cPhysics::AddImpulse(const glm::vec3 &i) 
{ 
	forces += i; 
}


void UpdatePhysics(const double t, const double dt, bool collisionsEnabled)
{
	if (collisionsEnabled)
	{
		std::vector<collisionInfo> collisions;
		// check for collisions
		{
			dvec3 pos;
			dvec3 norm;
			double depth;
			for (size_t i = 0; i < colliders.size(); ++i) {
				for (size_t j = i + 1; j < colliders.size(); ++j) {
					if (collision::IsColliding(*colliders[i], *colliders[j], pos, norm, depth)) {
						collisions.push_back({ colliders[i], colliders[j], pos, norm, depth });
					}
				}
			}
		}
		// handle collisions
		{
			for (auto &c : collisions) {
				Resolve(c);
			}
		}
	}
	// Integrate
	// TODO: work with new_pos instead of prev_pos
	for (auto &e : physicsScene) 
	{
		// Check if particle has a fixed position in space
		if (e->fake)
		{
			continue;
		}
		else
		{
			e->Render();
			// calculate velocity from current and previous position
			e->velocity = e->position - e->prev_position;
			//e->velocity = e->prev_position + e->position;
			//dvec3 wind =  -velocity * 5.0;
			// set previous position to current position
			e->prev_position = e->position;
			// position += v + a * (dt^2)
			e->position += e->velocity + (e->forces + gravity) * e->mass * pow(dt, 2.0);
			//e->position += e->velocity + (e->forces + gravity) * e->mass * pow(dt, 2.0);
			e->forces = dvec3(0);

			if (e->position.y <= 0.0f)
			{
				// e->prev_position = e->position + (e->position - e->prev_position);
			}
		}
	}
}

struct Derivative {
	dvec3 dx, dv;
};

dvec3 acceleration(cPhysics* e, dvec3 x, dvec3 v, float t) {
	// we could be summing accelerations, or doing other cool things here
	return (e->forces + gravity);// * pow(t, 2.0);
	//return dvec3(0, -10.0, 0);
}

Derivative compute(cPhysics* e, const double t, const double dt, const Derivative &d) {
	//where would we be and how fast at dt 
	dvec3 x = e->position + d.dx * dt;
	dvec3 v = e->velocity + d.dv * dt;

	Derivative output;
	output.dx = v;
	//what would the acceleration be at this point?
	// *********************************
	output.dv = acceleration(e, x, v, t);
	// *********************************
	return output;
}

void UpdatePhysicsRK4(const double t, const double dt) {
	for (auto &e : physicsScene)
	{
		if (e->fake)
		{
			continue;
		}
		Derivative a, b, c, d;

		//Incrementally compute for various dt
		a = compute(e, t, 0.0f, { dvec3(0), dvec3(0) });
		b = compute(e, t, dt * 0.5f, a);
		c = compute(e, t, dt * 0.5f, b);
		d = compute(e, t, dt, c);

		//compute the final derivitive
		// *********************************
		//apply against dt and get final outputs
		e->position += (a.dx + 2.0 * b.dx + 2.0 * c.dx + d.dx) * dt / 6.0;
		e->velocity += (a.dv + 2.0 * b.dv + 2.0 * c.dv + d.dv) * dt / 6.0;
		
		e->forces = dvec3(0);
		// *********************************
		
		//if (e->position.y <= 0.0f) {
		//	e->velocity.y = -e->velocity.y;
		//}
	}
}


void UpdatePhysicsCustom(const double t, const double dt) {
  std::vector<collisionInfo> collisions;
  // check for collisions
  {
    dvec3 pos;
    dvec3 norm;
    double depth;
    for (size_t i = 0; i < colliders.size(); ++i) {
      for (size_t j = i + 1; j < colliders.size(); ++j) {
        if (collision::IsColliding(*colliders[i], *colliders[j], pos, norm, depth)) {
          collisions.push_back({colliders[i], colliders[j], pos, norm, depth});
        }
      }
    }
  }
  // handle collisions
  {
    for (auto &c : collisions) {
      Resolve(c);
    }
  }
  // Integrate
  for (auto &e : physicsScene) {
    e->Render();


    // calcualte velocity from current and previous position
    dvec3 velocity = e->position - e->prev_position;
	//dvec3 wind = dvec3(-0.2, 0, 0);
	//if (first)
	//{
	//	velocity += initVelocity;
	//	first = false;
	//}
    // set previous position to current position
    e->prev_position = e->position;

	//if (velocity.x < 0.002f && initVelocity.x > 0.0f && e->position.y < 0.1 || velocity.z < 0.002f && initVelocity.z > 0.0f && e->position.y < 0.1)
	//{
	//	if (velocity.x < 0.002f && initVelocity.x > 0.0f)
	//	{
	//		velocity = dvec3(0.0f, velocity.y, velocity.z);
	//	}
	//	
	//	if (velocity.z < 0.002f && initVelocity.z > 0.0f)
	//	{
	//		velocity = dvec3(velocity.x, velocity.y, 0.0f);
	//	}
	//}
	//else
	//{
		// position += v + a * (dt^2)
		//e->position += velocity + (e->forces + gravity + wind) * pow(dt, 2);
		e->position += velocity + (e->forces + gravity) * pow(dt, 2);
	//}
	
	//cout << "x: " << velocity.x << ", y: " << velocity.y << ", z: " << velocity.z << endl;
    
	//cout << "x: " << e->position.x << ", y: " << e->position.y << ", z: " << e->position.z << endl;
    e->forces = dvec3(0);

    if (e->position.y <= 0.0f) {
      //  e->prev_position = e->position + (e->position - e->prev_position);
    }
  }
}

void InitPhysics() {}

void ShutdownPhysics() {}
//----------------------

cParticle::cParticle() {}

cParticle::~cParticle() {}

cPlank::cPlank() {}

cPlank::~cPlank() {}

void cPlank::Update(double delta) {}

void cParticle::Update(double delta) {}

//----------------------
cRigidBody::cRigidBody() {}

cRigidBody::~cRigidBody() {}

void cRigidBody::Update(double delta) {}

cCollider::cCollider(const std::string &tag) : Component(tag) { colliders.push_back(this); }

cCollider::~cCollider() {
  auto position = std::find(colliders.begin(), colliders.end(), this);
  if (position != colliders.end()) {
    colliders.erase(position);
  }
}

void cCollider::Update(double delta) {}

cSphereCollider::cSphereCollider() : radius(1.0), cCollider("SphereCollider") {}

cSphereCollider::~cSphereCollider() {}

cPlaneCollider::cPlaneCollider() : normal(dvec3(0, 1.0, 0)), cCollider("PlaneCollider") {}

cPlaneCollider::~cPlaneCollider() {}

// TODO: Initiliaze AABB values
cAABB::cAABB() : cCollider("AABBCollider") {}
cAABB::cAABB(dvec3 centerPoint, double radius[3]) : cCollider("AABBCollider")
{
	this->centerPoint = centerPoint;
	this->radius[0] = radius[0];
	this->radius[1] = radius[1];
	this->radius[2] = radius[2];
}

cAABB::~cAABB(){}

ParticleSpring::ParticleSpring(cPhysics *other, float sc, float rl)
: other(other), springConstant(sc), restLength(rl)
{
}

ParticleSpring::ParticleSpring()
{
}

ParticleDrag::ParticleDrag(float k1, float k2)
: k1(k1), k2(k2)
{
}

ParticleDrag::ParticleDrag()
{
}

void ParticleDrag::updateForce(cPhysics* particle, float duration)
{
	vec3 force;
	force = particle->velocity;
	
	// Calculate the total drag coefficient
	float dragCoeff = length(force);
	dragCoeff = k1 * dragCoeff + k2 * dragCoeff * dragCoeff;

	// Calculate the final force and apply it
	normalize(force);
	force *= -dragCoeff;
	particle->AddImpulse(force);
	//cout << force << endl;
}

void ParticleSpring::updateForce(cPhysics* particle, float duration)
{
	// Calculate the vector of the spring
	dvec3 force;
	force = particle->position;
	force -= other->position;

	// Calculate the magnitude of the force
	float magnitude = length(force);

	//if (magnitude <= restLength) return;

	//magnitude *= magnitude;
	magnitude = abs(magnitude - restLength);
	//cout << magnitude << endl;
	magnitude *= springConstant;

	// Calculate the final force and apply it
	force = normalize(force);
	force *= -magnitude;
	particle->AddImpulse(force);
	other->AddImpulse(-force);
}