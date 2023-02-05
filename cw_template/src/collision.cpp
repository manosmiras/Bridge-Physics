#include "collision.h"
#include <glm/glm.hpp>

using namespace std;
using namespace glm;
namespace collision {

// Sphere against sphere collision
bool IsColliding(const cSphereCollider &c1, const cSphereCollider &c2, dvec3 &pos, dvec3 &norm, double &depth) {
  const dvec3 p1 = c1.GetParent()->GetPosition();
  const dvec3 p2 = c2.GetParent()->GetPosition();
  const dvec3 d = p2 - p1;
  const double distance = glm::length(d);
  const double sumRadius = c1.radius + c2.radius;
  if (distance < sumRadius && c1.GetParent()->GetName() != "Player" && c2.GetParent()->GetName() != "Player") {
	  //cout << "sphere to sphere collision" << endl;
    depth = sumRadius - distance;
    norm = -glm::normalize(d);
    pos = p1 - norm * (c1.radius - depth * 0.5f);
    return true;
  }
  return false;
}
// Sphere against sphere collision
bool IsColliding(const cSphereCollider &s, const cPlaneCollider &p, dvec3 &pos, dvec3 &norm, double &depth) {
	const dvec3 sp = s.GetParent()->GetPosition();
	const dvec3 pp = p.GetParent()->GetPosition();

	// Calculate a vector from a point on the plane to the center of the sphere
	const dvec3 vecTemp(sp - pp);

	// Calculate the distance: dot product of the new vector with the plane's normal
	double distance = dot(vecTemp, p.normal);

	if (distance <= s.radius) {
		norm = p.normal;
		pos = sp - norm * distance;
		depth = s.radius - distance;
		return true;
	}

	return false;
}
// AABB against AABB collision
bool IsColliding(const cAABB &a, const cAABB &b, dvec3 &pos, dvec3 &norm, double &depth)
{
	if (abs(a.centerPoint.x - b.centerPoint.x) > a.radius[0] + b.radius[0])
		return false;
	if (abs(a.centerPoint.y - b.centerPoint.y) > a.radius[1] + b.radius[1])
		return false;
	if (abs(a.centerPoint.z - b.centerPoint.z) > a.radius[2] + b.radius[2])
		return false;

	//cout << "aabb colliding with aabb" << endl;

	return true;
}

// Computes the squared distance between a point and an AABB
float SqDistPointAABB(const dvec3 point, cAABB b)
{
	float sqDist = 0.0f;

	// For each axis, count any excess distance outside box extents
	for (int i = 0; i < 3; i++)
	{
		// Get min point
		float min = b.centerPoint[i] - b.radius[i];
		// Get max point
		float max = b.centerPoint[i] + b.radius[i];

		float v = point[i];

		if (v < min) // v < b.min[i]
			sqDist += (min - v) * (min - v);
		if (v > max) // v > b.max[i]
			sqDist += (v - max) * (v - max);
	}

	return sqDist;
}
// Sphera against AABB
bool IsColliding(const cSphereCollider &s, const cAABB &b, dvec3 &pos, dvec3 &norm, double &depth)
{
	const dvec3 sp = s.GetParent()->GetPosition();
	const dvec3 bp = b.GetParent()->GetPosition();
	const dvec3 d = bp - sp;
	const double distance = length(d);
	float sqDist = SqDistPointAABB(sp, b);
	if (sqDist <= s.radius * s.radius && s.GetParent()->GetName() == "Player")
	{
		//cout << "player with aabb collision, norm: " << norm << endl;
		depth = sqDist - distance; //s.radius * s.radius - distance;
		norm = -glm::normalize(d);
		//cout << norm << endl;
		//pos = sp - norm * (s.radius - depth * 0.5f);
		////cout << distance << endl;
		//// temp code
		//auto physComp = b.GetParent()->GetComponents("Physics");
		//auto aabbPhysics = static_cast<cPlank *>(physComp[0]);
		//////aabbPhysics->AddImpulse(vec3(0, -200, 0)); //position.y -= 1;
		//
		//
		auto physComp2 = s.GetParent()->GetComponents("Physics");
		auto spherePhysics = static_cast<cPhysics *>(physComp2[0]);
		spherePhysics->AddImpulse(dvec3(300) * spherePhysics->mass * norm);
		//
		//aabbPhysics->rightParticle->AddImpulse(dvec3(0, 100, 0) * spherePhysics->mass * -norm);
		//aabbPhysics->leftParticle->AddImpulse(dvec3(0, 100, 0) * spherePhysics->mass * -norm);

		return true;
		
	}
	return false;

	//return sqDist <= s.radius * s.radius;
}

// Not used
bool IsCollidingCustom(const cSphereCollider &s, const cPlaneCollider &p, dvec3 &pos, dvec3 &norm, double &depth) {
  const dvec3 sp = s.GetParent()->GetPosition();
  const dvec3 pp = p.GetParent()->GetPosition();

  // Calculate a vector from a point on the plane to the center of the sphere
  const dvec3 vecTemp(sp - pp);

  // Calculate the distance: dot product of the new vector with the plane's normal
  double distance = dot(vecTemp, p.normal);
  

  if (distance <= s.radius) 
  {
	dvec3 vecTemp2 = (normalize(vecTemp));
	
	if (vecTemp2.x < 0 && vecTemp2.z < 0 || vecTemp2.x < 0 && vecTemp2.z > 0 || vecTemp2.x > 0 && vecTemp2.z < 0)
	{
		norm = dvec3(vecTemp2.x, p.normal.y, vecTemp2.z);
	}
	else if (vecTemp2.x < 0 || vecTemp2.z < 0)
	{
		norm = dvec3(-vecTemp2.x, p.normal.y, vecTemp2.z);
	}
	else
	{
		norm = dvec3(vecTemp2.x, p.normal.y, vecTemp2.z);
	}

    pos = sp - norm * distance;
    depth = s.radius - distance;
    return true;
  }

  return false;
}

// Plane against Plane
bool IsColliding(const cPlaneCollider &c1, const cPlaneCollider &c2, dvec3 &pos, dvec3 &norm, double &depth) {
  cout << "PLANE PLANE" << endl;
  return false;
}

// Calls appropriate collision methods
bool IsColliding(const cCollider &c1, const cCollider &c2, glm::dvec3 &pos, glm::dvec3 &norm, double &depth) {
  enum shape {
    UNKOWN = 0,
    PLANE,
    SPHERE,
	AABB
  };
  shape s1 = UNKOWN;
  shape s2 = UNKOWN;
  if (dynamic_cast<const cSphereCollider *>(&c1)) {
    s1 = SPHERE;
  } else if (dynamic_cast<const cPlaneCollider *>(&c1)) {
    s1 = PLANE;
  }
  else if (dynamic_cast<const cAABB *>(&c1)){
	  s1 = AABB;
  }

  if (dynamic_cast<const cSphereCollider *>(&c2)) {
    s2 = SPHERE;
  } else if (dynamic_cast<const cPlaneCollider *>(&c2)) {
    s2 = PLANE;
  }
  else if (dynamic_cast<const cAABB *>(&c2)){
	  s2 = AABB;
  }

  if (!s1 || !s2) {
    cout << "Routing Error" << endl;
    return false;
  }
  if (s1 == PLANE) 
  {
	// PLANE WITH PLANE
    if (s2 == PLANE) 
	{
      return IsColliding(dynamic_cast<const cPlaneCollider &>(c1), dynamic_cast<const cPlaneCollider &>(c2), pos, norm,
                         depth);
    } 
	// PLANE WITH SPHERE
	else if (s2 == SPHERE)
	{
      return IsColliding(dynamic_cast<const cPlaneCollider &>(c1), dynamic_cast<const cSphereCollider &>(c2), pos, norm,
                         depth);
    }
	// PLANE WITH AABB
	else
	{
		
		return false;
	}
  } 
  else if (s1 == SPHERE)
  {
	// SPHERE WITH PLANE
    if (s2 == PLANE) {
      return IsColliding(dynamic_cast<const cSphereCollider &>(c1), dynamic_cast<const cPlaneCollider &>(c2), pos, norm,
                         depth);
    } 
	// SPHERE WITH SPHERE
	else if (s2 == SPHERE)
	{
      return IsColliding(dynamic_cast<const cSphereCollider &>(c1), dynamic_cast<const cSphereCollider &>(c2), pos,
                         norm, depth);
    }
	else
	{
		// SPHERE WITH AABB
		return IsColliding(dynamic_cast<const cSphereCollider &>(c1), dynamic_cast<const cAABB &>(c2), pos,
			norm, depth);
	}
  }
  else
  {
	 // AABB with AABB
	 if (s2 == AABB)
		  return IsColliding(dynamic_cast<const cAABB &>(c1), dynamic_cast<const cAABB &>(c2), pos, norm,
                        depth);
	 // AABB with SPHERE
	 else if (s2 == SPHERE)
		  return IsColliding(dynamic_cast<const cAABB &>(c1), dynamic_cast<const cSphereCollider &>(c2), pos, norm,
		  depth);
	  
	  return false;
  }
  
}
}