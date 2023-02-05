#include "menu_handling.h"

void menu_handling::display(double &fps, bool &switchScenes, int &camPos, bool &drawLines, bool &drawParticles, ParticleDrag &pd, cPhysics *playerPhysics, bool &collisionsEnabled)
// Show a debug window
{
	glfwPollEvents();
	ImGui_ImplGlfwGL3_NewFrame();

	ImGui::Text("FPS: %.3f ", fps);
	if (ImGui::Button("Switch scenes"))
	{
		if (switchScenes)
			switchScenes = false;
		else
			switchScenes = true;
	}
	if (ImGui::Button("Change camera position"))
	{
		if (camPos == 0)
			camPos = 1;
		else
			camPos = 0;
	}

	if (ImGui::Button("Draw lines"))
	{
		if (drawLines)
			drawLines = false;
		else
			drawLines = true;
	}

	if (ImGui::Button("Draw Particles"))
	{
		if (drawParticles)
			drawParticles = false;
		else
			drawParticles = true;
	}

	static float ropeDamping = 100.0f;
	ImGui::SliderFloat("Rope damping force", &ropeDamping, 0.0f, 1000.0f);

	if (switchScenes == false)
	{

		ImGui::Text("Physics Parameters");

		ImGui::Text("Use WASD or arrow keys to move, press 'c' to cut rope");

		pd = ParticleDrag(ropeDamping, ropeDamping);

		static float playerMass = 1.0f;
		ImGui::SliderFloat("Player mass", &playerMass, 1.0f, 10.0f);
		playerPhysics->mass = playerMass;

		if (ImGui::Button("Enable/Disable Collisions"))
		{
			if (collisionsEnabled)
				collisionsEnabled = false;
			else
				collisionsEnabled = true;
		}

		if (ImGui::Button("Reset Player"))
		{
			playerPhysics->position = vec3(-50, 40, 5);
			playerPhysics->prev_position = vec3(-50, 40, 5);
		}
	}



}
