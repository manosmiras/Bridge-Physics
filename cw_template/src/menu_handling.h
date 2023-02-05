#pragma once
#include "physics.h"
// ImGui includes
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;
class menu_handling
{
public:
	static void display(double &fps, bool &switchScenes, int &camPos, bool &drawLines, bool &drawParticles, ParticleDrag &pd, cPhysics *playerPhysics, bool &collisionsEnabled);
};