#pragma once


#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include "Walnut/Random.h"
#include <glm/glm.hpp>





class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height, uint32_t SSAALevel);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void lightDirUpdated();
private:
	glm::vec4 TraceRay(const Scene& scene, const Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_ImageDataSSAABuffer = nullptr;
	glm::vec3 lightDir = glm::normalize(lightDirProxy);

public:
	glm::vec3 lightDirProxy = glm::vec3(-1, -1, 1);
	bool doShading = true;
};