#pragma once


#include "Walnut/Image.h"

#include <memory>
#include "Walnut/Random.h"
#include <glm/glm.hpp>





class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render();

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }
private:
	glm::vec4 PerPixel(glm::vec2 coord);

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec3 lightDir = glm::normalize(lightDirProxy);

public:
	glm::vec3 SphereOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 lightDirProxy = glm::vec3(-1, -1, 1);

	bool doShading = true;
	float radius = 0.5f;
};