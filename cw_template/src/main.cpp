#define GLM_ENABLE_EXPERIMENTAL
#include "game.h"
#include "physics.h"
#include "rope.h"
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <graphics_framework.h>
#include <phys_utils.h>
#include <thread>

// ImGui includes
#include "menu_handling.h"

using namespace std;
using namespace graphics_framework;
using namespace glm;
#define physics_tick 1.0 / 60.0

// Left rope entity list
static vector<unique_ptr<Entity>> rope1;
// Right rope entity list
static vector<unique_ptr<Entity>> rope2;
// Floor (Plane)
static unique_ptr<Entity> floorEnt;
// Planks entity vector
static vector<unique_ptr<Entity>> planks;
// Player entity
static unique_ptr<Entity> player;
// Single rope entity vector
static vector<unique_ptr<Entity>> ropeSingle;
// Springs for single rope in second scene
vector<ParticleSpring> springsOfRopeSingle;
// Springs for left rope
vector<ParticleSpring> springs;
// Springs for right rope
vector<ParticleSpring> springs2;
// Springs inbetween two ropes
vector<ParticleSpring> springsInbetween;

// Used for initialization of springs
ParticleSpring ps;
// Damping force for springs
ParticleDrag pd;
// Cut rope
bool cut = false;
// Draw lines between particles
bool drawLines = true;
// Draw particles
bool drawParticles = false;
// Switch scenes
bool switchScenes = false;
// Trigger collisions
bool collisionsEnabled = true;
// Camera positions
int camPos = 0;
// Number of particles in bridge simulation
int theSize = 20;
// Frame rate
double fps = 0.0;
// Used to update frame rate every 0.25s
double fpsUpdater;

// Update content
bool update(double delta_time) 
{
	if (camPos == 0)
	{
		phys::SetCameraPos(vec3(60.0f, 80.0f, 60.0f));
		phys::SetCameraTarget(vec3(-50, 20.0f, 0));
	}
	if (camPos == 1)
	{
		phys::SetCameraPos(vec3(40.0f, 120.0f, 0.0f));
		phys::SetCameraTarget(vec3(-40, 40.0f, 0));
	}
	// Player for first scene
	auto playerPhysComp = player.get()->GetComponents("Physics");
	auto playerPhysics = static_cast<cPhysics *>(playerPhysComp[0]);

	// Rope for second scene
	auto singleRopePhysComp = ropeSingle[0].get()->GetComponents("Physics");
	auto singleRopePhysics = static_cast<cPhysics *>(singleRopePhysComp[0]);
	// Move forward
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_W) || glfwGetKey(renderer::get_window(), GLFW_KEY_UP))
	{
		if (switchScenes == false)
			playerPhysics->AddImpulse(vec3(-100, 0, 0));
		else
			singleRopePhysics->AddImpulse(vec3(-100, 0, 0));
	}
	// Move backwards
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_S) || glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN))
	{
		if (switchScenes == false)
			playerPhysics->AddImpulse(vec3(100, 0, 0));
		else
			singleRopePhysics->AddImpulse(vec3(100, 0, 0));
	}
	// Move right
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_D) || glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT))
	{
		if (switchScenes == false)
			playerPhysics->AddImpulse(vec3(0, 0, -100));
		else
			singleRopePhysics->AddImpulse(vec3(0, 0, -100));
	}
	// Move left
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_A) || glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT))
	{
		if (switchScenes == false)
			playerPhysics->AddImpulse(vec3(0, 0, 100));
		else
			singleRopePhysics->AddImpulse(vec3(0, 0, 100));
	}
	// "Jump"
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_SPACE))
	{
		if (switchScenes == false)
			playerPhysics->AddImpulse(vec3(0, 100, 0));
		else
			singleRopePhysics->AddImpulse(vec3(0, 100, 0));
	}
	// Cut the rope
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_C))
	{
		if (cut)
		{
			cut = false;
		}
		else
		{
			cut = true;
		}
		cout << "cut triggered" << endl;
	}

	// BRIDGE SCENE
	if (switchScenes == false)
	{
		for (int i = 0; i < springs.size() - 1; i++)
		{
			if (cut && i == rope1.size() / 2)
			{
				continue;
			}
			// Get physics component for each particle on left rope
			auto b = rope1[i].get()->GetComponents("Physics");
			auto p = static_cast<cPhysics *>(b[0]);
		
			// Update spring force
			springs[i + 1].updateForce(p, 1.0f);
			
			// Get physics component for each particle on right rope
			auto b2 = rope2[i].get()->GetComponents("Physics");
			auto p2 = static_cast<cPhysics *>(b2[0]);
			
			// Update spring force
			springs2[i + 1].updateForce(p2, 1.0f);
			// Update spring force
			springsInbetween[i+1].updateForce(p2, 1.0f);
		
		}
		
		// Damping
		for (int i = 1; i < springs.size(); i++)
		{
			// LEFT ROPE
			auto b = rope1[i - 1].get()->GetComponents("Physics");
			auto p = static_cast<cPhysics *>(b[0]);
			pd.updateForce(p, 1.0f);
			// RIGHT ROPE
			auto b2 = rope2[i - 1].get()->GetComponents("Physics");
			auto p2 = static_cast<cPhysics *>(b2[0]);
			pd.updateForce(p2, 1.0f);

			// ROPES INBETWEEN
			pd.updateForce(p, 1.0f);
			pd.updateForce(p2, 1.0f);

		}
		// Update plank positions
		for (int i = 0; i < theSize; i++)
		{
			// RIGHT ROPE
			auto b = rope1[i].get()->GetComponents("Physics");
			auto p = static_cast<cPhysics *>(b[0]); // Get physics component
		
			// LEFT ROPE
			auto b2 = rope2[i].get()->GetComponents("Physics");
			auto p2 = static_cast<cPhysics *>(b2[0]); // Get physics component
			
			// The plank position
			vec3 plankPosition = (p->position + p2->position) / 2.0;
			
			// The plank rotation
			//float theta = acos(dot(normalize(p2->position - p->position), normalize(p2->position - p->prev_position)));

			//if (p->position.y >= p2->position.y)
			//{
			//	theta = -theta;
			//}
			//cout << theta << endl;
			//quat rx = quat(vec3(theta, 0.0f, 0.0f));

			//planks[i]->SetRotation(rx);

			//quat temp = planks[i]->GetRotation();
			//cout << temp.x << " " << temp.y << " " << temp.z << endl;
			
			//planks[i]->GetTranform();
			//planks[i]->Update(delta_time);
			

			auto physComp = planks[i].get()->GetComponents("Physics");
			auto plankPhys = static_cast<cPlank *>(physComp[0]); // Get physics component
			plankPhys->position = plankPosition;
			plankPhys->rightParticle = p;
			plankPhys->leftParticle = p2;
			
			
			auto collComp = planks[i].get()->GetComponents("AABBCollider");
			auto plankColl = static_cast<cAABB *>(collComp[0]); // Get collider component
			// Set center point
			plankColl->centerPoint = plankPosition;
			// Set x, y & z radii
			plankColl->radius[0] = planks[i]->GetScale().x / 2;
			plankColl->radius[1] = planks[i]->GetScale().y / 2;
			plankColl->radius[2] = planks[i]->GetScale().z / 2;
		}
	}
	// SINGLE ROPE SCENE
	else
	{
		// Update spring forces
		for (int i = 0; i < springsOfRopeSingle.size() - 1; i++)
		{
			if (cut && i == ropeSingle.size() / 2)
			{
				continue;
			}
			auto b = ropeSingle[i].get()->GetComponents("Physics");
			auto p = static_cast<cPhysics *>(b[0]);
			springsOfRopeSingle[i + 1].updateForce(p, 1.0f);
	
		}
		// Damping
		for (int i = 1; i < springsOfRopeSingle.size(); i++)
		{
			auto b = ropeSingle[i - 1].get()->GetComponents("Physics");
			auto p = static_cast<cPhysics *>(b[0]);
			pd.updateForce(p, 1.0f);
		}
	}

	static double t = 0.0;
	static double accumulator = 0.0;
	accumulator += delta_time;
	
	while (accumulator > physics_tick) {
		UpdatePhysics(t, physics_tick, collisionsEnabled);
		//UpdatePhysicsRK4(t, physics_tick);
		accumulator -= physics_tick;
		t += physics_tick;
	}

	fpsUpdater += delta_time;
	if (fpsUpdater >= 0.25)
	{
		fps = (1.0 / delta_time);
		fpsUpdater = 0;
	}
	// Display menu and edit values
	menu_handling::display(fps, switchScenes, camPos, drawLines, drawParticles,  pd,  playerPhysics, collisionsEnabled);

	// Update
	if (switchScenes == false)
	{
		for (auto &e : rope1)
		{
			e->Update(delta_time);
		}
		player->Update(delta_time);
	}
	else
	{
		for (auto &e : ropeSingle)
		{
			e->Update(delta_time);
		}
	}
	phys::Update(delta_time);
	return true;
}

// Initialize content
bool load_content() 
{
	// Imgui stuff
	glewInit();
	ImGui_ImplGlfwGL3_Init(renderer::get_window(), true);

	phys::Init();
	
	// Create ropes for first scene
	for (int i = 0; i < theSize; i++)
	{
		rope1.push_back(move(rope::CreateParticle(i, 30, 0, true, 2)));
		rope2.push_back(move(rope::CreateParticle(i, 30, 10, true, 2)));
		// Make player
		player = move(rope::CreateParticle(5, 40, 5, true, 0));
		
	}
	// Create rope for 2nd scene
	for (int i = 0; i < 10; i++)
	{
		ropeSingle.push_back(move(rope::CreateParticle(-10, i, 20, false, 1)));
	}

	// Edit player's settings
	player->SetName("Player");
	auto collComp = player->GetComponents("SphereCollider");
	auto coll = static_cast<cSphereCollider *>(collComp[0]); // Get Collider component
	coll->radius = 2.0f; // Set radius to 2

	auto physComp = player->GetComponents("Physics");
	auto playerPhysics = static_cast<cPhysics *>(physComp[0]); // Get physics component
	playerPhysics->mass = 1.0f; // Set mass to 1

	// FOR BRIDGE
	for (int i = 0; i < rope1.size(); i++)
	{
		// RIGHT ROPE
		auto b = rope1[i].get()->GetComponents("Physics");
		auto p = static_cast<cPhysics *>(b[0]); // Get physics component

		ps = ParticleSpring(p, 60.0f, 5.5f);

		springs.push_back(ps);

		// ROPES INBETWEEN
		ps = ParticleSpring(p, 0.5f, 10.0f);
		springsInbetween.push_back(ps);

		// LEFT ROPE
		auto b2 = rope2[i].get()->GetComponents("Physics");
		auto p2 = static_cast<cPhysics *>(b2[0]); // Get physics component

		ps = ParticleSpring(p2, 60.0f, 5.5f);
		springs2.push_back(ps);
	}
	// Set up ParticleDrag
	pd = ParticleDrag(100.0f, 100.0f);
	// Create planks and add them to planks list
	for (int i = 0; i < theSize; i++)
	{
		planks.push_back(move(rope::CreatePlanks()));
	}

	// FOR SINGLE ROPE
	for (int i = 0; i < ropeSingle.size(); i++)
	{
		auto b = ropeSingle[i].get()->GetComponents("Physics");
		auto p = static_cast<cPhysics *>(b[0]); // Get physics component

		ps = ParticleSpring(p, 10.0f, 0.5f);

		springsOfRopeSingle.push_back(ps);
	}

	floorEnt = unique_ptr<Entity>(new Entity());
	floorEnt->AddComponent(unique_ptr<Component>(new cPlaneCollider()));
	// Camera positions
	phys::SetCameraPos(vec3(60.0f, 80.0f, 60.0f));
	phys::SetCameraTarget(vec3(-50, 20.0f, 0));

	InitPhysics();
	return true;
}

// Render scene
bool render() 
{
	if (switchScenes == false)
	{
		// Draw particles for rope on left and right of bridge
		if (drawParticles)
		{
			for (auto &e : rope1)
			{
				e->Render();
			}

			for (auto &e : rope2)
			{
				e->Render();
			}
		}
		// Draw planks
		for (auto &e : planks)
		{
			e->Render();
		}
		// Draw the player
		player->Render();

		// Draw lines
		if (drawLines)
		{
			for (int i = 0; i < rope1.size() - 1; i++)
			{
				if (cut && i == rope1.size() / 2)
				{
					continue;
				}

				// ROPE LEFT
				auto b = rope1[i].get()->GetComponents("Physics");
				auto p = static_cast<cPhysics *>(b[0]);

				auto bNext = rope1[i + 1].get()->GetComponents("Physics");
				auto pNext = static_cast<cPhysics *>(bNext[0]);

				phys::DrawLine(p->position, pNext->position);

				// ROPE RIGHT
				auto b2 = rope2[i].get()->GetComponents("Physics");
				auto p2 = static_cast<cPhysics *>(b2[0]);

				auto b2Next = rope2[i + 1].get()->GetComponents("Physics");
				auto p2Next = static_cast<cPhysics *>(b2Next[0]);

				phys::DrawLine(p2->position, p2Next->position);

				// ROPES INBETWEEN
				phys::DrawLine(p->position, p2->position);
			}
		}
	}
	else
	{
		// Draw particles for single rope (2nd scene)
		if (drawParticles)
		{
			for (auto &e : ropeSingle)
			{
				e->Render();
			}
		}
		// Draw lines for single rope (2nd scene)
		if (drawLines)
		{
			for (int i = 0; i < ropeSingle.size() - 1; i++)
			{
				auto b = ropeSingle[i].get()->GetComponents("Physics");
				auto p = static_cast<cPhysics *>(b[0]);

				auto bNext = ropeSingle[i + 1].get()->GetComponents("Physics");
				auto pNext = static_cast<cPhysics *>(bNext[0]);

				phys::DrawLine(p->position, pNext->position);
			}
		}
	}

	phys::DrawScene();

	// Imgui rendering
	ImGui::Render();

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