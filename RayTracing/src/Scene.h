#pragma once

#ifndef GLM_FORCE_INTRINSICS
#define GLM_FORCE_INTRINSICS
#endif // !GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <vector>


struct Sphere
{
	glm::vec3 Position{0.0f};
	float Radius = 0.5f;

	glm::vec3 Albedo{1.0f};
};

struct Scene
{
	std::vector<Sphere> Spheres;
};