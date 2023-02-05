#include "ik.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <graphics_framework.h>
#include <phys_utils.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Simulation constants
int numLinks = 5;
float linkLength = 2.0f; // Length of each link
std::vector<Link> links;
vec3 target = vec3(6.0f, 4.0f, 0);

vec3 vbase;
vec3 vend;
mat4 cubeBase;
mat4 cubeEnd;
vec3 planeAxis;

void MoveTarget() {
	target = glm::ballRand((static_cast<float>(numLinks) * linkLength) * 0.6f);
	target.y = abs(target.y);
	// target.x = abs(target.x);
	cout << "boop" << endl;
}

void UpdateHierarchy() {
	for (int i = 0; i < (int)links.size(); ++i) {
		mat4 R1 = mat4_cast(angleAxis(links[i].m_angle, links[i].m_axis));
		mat4 T1 = translate(mat4(1.0f), vec3(linkLength, 0, 0));
		links[i].m_base = mat4(1.0) * R1;
		links[i].m_end = links[i].m_base * T1;
		links[i].m_worldaxis = links[i].m_axis;
		if (i > 0) {
			// Don't move the root link.
			links[i].m_base = links[i - 1].m_end * links[i].m_base;
			links[i].m_end = links[i].m_base * links[i].m_end;
			links[i].m_worldaxis = normalize(mat3(links[i - 1].m_end) * links[i].m_axis);
		}
	}
}

bool load_content() {
	phys::Init();
	for (int i = 0; i < numLinks; ++i) {
		int ax = (i + 0) % 3 == 0;
		int ay = (i + 1) % 3 == 0;
		int az = (i + 2) % 3 == 0;
		links.push_back(Link(vec3(ax, ay, az), 0.0 + (float)i * 0.0f));
		cout << vec3(ax, ay, az) << endl;
	}
	UpdateHierarchy();
	return true;
}

void UpdateIK() {

	bool notSolved = true; // Solution found?
	int timeout = 0; // Used to not stay in the loop for too long

	while (notSolved && timeout < 100)
	{
		UpdateHierarchy();

		const float distance = length(vec3(links[links.size() - 1].m_end[3]) - target);
		if (distance < 0.5f)
		{
			ik_1dof_Update(target, links, linkLength, true);
		}
		else
			ik_1dof_Update(target, links, linkLength, false);

		//ik_3dof_Update(target, links, linkLength);

		

		// Solved!
		if (distance < 0.5f) {
			notSolved = false;
			const float distance = length(vec3(links[links.size() - 1].m_end[3]) - target);
			//cout << "Moving target, distance is: " << distance << endl;
			
		}
		timeout++;
	}
	//if (glfwGetKey(renderer::get_window(), GLFW_KEY_W))
		MoveTarget();

	
}

void RenderIK() {
	phys::DrawSphere(target, 0.2f, RED);
	
	// Check to move only when solution is found

	for (int i = 0; i < (int)links.size(); ++i)
	{
		
		//if (distance < 0.5f)
		//{
			vbase = links[i].m_base[3];
			vend = links[i].m_end[3];
			cubeBase = links[i].m_base;
			cubeEnd = links[i].m_end;
			planeAxis = links[i].m_worldaxis;
			//cout << i << endl;	
		//}
		
		phys::DrawCube(cubeBase * glm::scale(mat4(1.0f), vec3(0.5f)), GREEN);
		phys::DrawCube(cubeEnd * glm::scale(mat4(1.0f), vec3(0.5f)), ORANGE);
		phys::DrawLine(vbase, vend);
		phys::DrawPlane(vbase, planeAxis, vec3(0.01f));

		//basePrev = base;
		//endPrev = end;

	}
}


bool update(float delta_time) {
	static float rot = 0.0f;
	rot += 0.2f * delta_time;
	phys::SetCameraPos(rotate(vec3(15.0f, 12.0f, 15.0f), rot, vec3(0, 1.0f, 0)));
	UpdateIK();
	phys::Update(delta_time);
	return true;
}

bool render() {
	RenderIK();
	phys::DrawScene();
	return true;
}

void main() {
	// Create application
	app application;
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}