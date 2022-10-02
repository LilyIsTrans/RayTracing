#pragma once


#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include "Walnut/Random.h"
#ifndef GLM_FORCE_INTRINSICS
#define GLM_FORCE_INTRINSICS
#endif // !GLM_FORCE_INTRINSICS

#include <glm/glm.hpp>





class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void lightDirUpdated();
private:

	struct HitPayload
	{
		float HitDistance;

		glm::vec3 WorldNormal;
		glm::vec3 WorldPosition;


		int ObjectIndex;
	};


	glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec3 lightDir = glm::normalize(lightDirProxy);

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

public:
	glm::vec3 lightDirProxy = glm::vec3(-1, -1, -1);
	bool doShading = true;
};