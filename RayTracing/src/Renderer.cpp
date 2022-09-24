#include "Renderer.h"

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& colour)
	{
		uint8_t r = colour.r * 255.0f;
		uint8_t g = colour.g * 255.0f;
		uint8_t b = colour.b * 255.0f;
		uint8_t a = colour.a * 255.0f;

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}


void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	lightDir = glm::normalize(lightDirProxy);
	float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f;
			coord.x *= aspectRatio;
			glm::vec4 colour = PerPixel(coord);
			colour = glm::clamp(colour, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(colour);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}



glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
	//rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0

	// a: Ray origin
	// b: Ray direction
	// r: radius
	// t = hit distance

	float a = glm::dot(rayDirection, rayDirection); //(bx^2 + by^2)
	float b = 2.0f * glm::dot(rayOrigin - SphereOrigin, rayDirection); //(2(axbx + ayby))
	float c = glm::dot(rayOrigin - SphereOrigin, rayOrigin - SphereOrigin) - radius * radius; //(ax^2 + ay^2 - r^2)

	// Quadratic formula discriminant:
	// b^2 - 4ac

	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f)
		return glm::vec4(0, 0, 0, 1);
	
	if (!doShading)
		return glm::vec4(1, 0, 1, 1);

	/*
	float a2 = 2.0f * a;
	float nbo2a = b / -a2;
	float do2a = sqrtf(discriminant) / a2;

	float t0 = nbo2a - do2a;
	float t1 = nbo2a + do2a;
	*/
	//float t0 = ( - b + sqrtf(discriminant)) / (2.0f * a);
	float closestT = ( - b - glm::sqrt(discriminant)) / (2.0f * a);

	
	//glm::vec3 h0 = rayOrigin + rayDirection * t0;
	glm::vec3 hitPoint = rayOrigin + rayDirection * closestT;

	//return glm::vec4(hitPoint, 1.0f);

	float lightLevel = (glm::dot(glm::normalize(hitPoint - SphereOrigin), -lightDir) + 1.0f) / 2.0f;
	
	return glm::vec4(lightLevel, lightLevel, lightLevel, 1);
	

	
}