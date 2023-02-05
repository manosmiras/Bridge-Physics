#include "ik.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include <phys_utils.h>
#include <vector>
using namespace std;
using namespace glm;
static unsigned int numLinks = 0;

static void Reach(int i, const vec3 &target, std::vector<Link> &const links, bool isSolved) {

	float previousAngle = links[i].m_angle;
	// our current orientation
	dquat qCur = angleAxis(links[i].m_angle, links[i].m_axis);
	// current position of this effector
	vec3 vlinkBasePos = (links[i].m_base)[3];
	// current position of the effector at the end of the chain
	vec3 vEndEffPos = links[links.size() - 1].m_end[3];
	// convert our axis to world space
	vec3 vLinkAxis = links[i].m_worldaxis;
	// project target onto axis plane
	vec3 vplanetarget = projectOntoPlane(target, vLinkAxis, vlinkBasePos);
	// project vEndEffPos onto axis plane
	vec3 vplaneEndEff = projectOntoPlane(vEndEffPos, vLinkAxis, vlinkBasePos);

	// These are the two vectors we want to converge.
	vec3 vLinkBaseToEndEffDirection = normalize(vplaneEndEff - vlinkBasePos);
	vec3 vLinkBaseToTargetDirection = normalize(vplanetarget - vlinkBasePos);

	// Get Dot of the two vectors
	float cosAngle = dot(vLinkBaseToTargetDirection, vLinkBaseToEndEffDirection);
	if (abs(cosAngle) < 1.0f) {
		// *********************************
		// Get the Angle between the two vectors
		float angle = glm::angle(vLinkBaseToTargetDirection, vLinkBaseToEndEffDirection);
		// Turn into a Quat with our axis
		dquat q1 = normalize(glm::angleAxis(angle, vLinkAxis));

		// Multply our current Quat with it
		dquat q2 = normalize(q1 * qCur);

		//cout << "mixed: " << mixed << endl;

		// Mix/Lerp
		if (isSolved)
		{
			
			for (size_t j = 100; j > 0; j--)
			{
				
				float mixVal = j * 0.01f;
				float mixed = mix(previousAngle, (float)glm::angle(q2), mixVal);
				links[i].m_angle = glm::angle(q2);
				cout << mixVal << endl;
			}

		}
		// Don't
		else
		{
			// Pull out the angle component, set the link params
			links[i].m_angle = glm::angle(q2);
		}
		
		
		//cout << "angle" << links[i].m_angle << endl;
		// *********************************
	}
}

void ik_1dof_Update(const vec3 &const target, std::vector<Link> &const links, const float linkLength, bool isSolved) {
	numLinks = links.size();
	for (size_t i = links.size(); i >= 1; --i) {
		//for (size_t i = 0; i < links.size() - 1; ++i) {
		UpdateHierarchy();

		Reach(i - 1, target, links, isSolved);

		const float distance = length(vec3(links[links.size() - 1].m_end[3]) - target);
		if (distance < 0.5f) {
			return;
		}
	}
}