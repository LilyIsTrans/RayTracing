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
	
	static float Q_rsqrt(const float& number)
	{
		long i;
		float x2, y;
		const float threehalfs = 1.5f;

		x2 = number * 0.5f;
		y = number;
		i = *(long*)&y;
		i = 0x5f3759df - (i >> 1);
		y = *(float*)&i;
		y = y * (threehalfs - (x2 * y * y));
		//y = y * (threehalfs - (x2 * y * y)); // Optional second newton iteration

		return y;
	}


	glm::vec3 Fast_normalize(const glm::vec3& v)
	{
		return v * Q_rsqrt(glm::dot(v, v));
	}
}


void Renderer::lightDirUpdated()
{
	lightDir = glm::normalize(lightDirProxy);
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
	lightDirUpdated();
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			glm::vec4 colour = TraceRay(scene, ray);
			colour = glm::clamp(colour, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(colour);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}



glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray, uint32_t bounce)
{
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0

	// a: Ray origin
	// b: Ray direction
	// r: radius
	// t = hit distance

	if (scene.Spheres.size() == 0)
		return glm::vec4(0, 0, 0, 1);


	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();
	float a = glm::dot(ray.Direction, ray.Direction); //(bx^2 + by^2)
	for (const Sphere& sphere : scene.Spheres)
	{
		glm::vec3 origin = ray.Origin - sphere.Position;

		
		float bo2 = glm::dot(origin, ray.Direction); //(2(axbx + ayby))
		float b = 2.0f * bo2;
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius; //(ax^2 + ay^2 - r^2)

		// Quadratic formula discriminant:
		// b^2 - 4ac

		float discriminant = b * b - (4.0f * a * c); // Actually 0.25 times the discriminant, since we might only care if it's nonnegative
		if (discriminant < 0.0f)
			continue;

		

		//float t0 = ( - b + sqrtf(discriminant)) / (2.0f * a);
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		if (closestT < hitDistance && closestT > 0.1f)
		{
			hitDistance = closestT;
			closestSphere = &sphere;
		}
	}

	if (closestSphere == nullptr)
		return glm::vec4(-0.0f, -0.0f, -0.0f, -0.0f);

	if (!doShading)
		return glm::vec4(closestSphere->Albedo, 1.0f);

	glm::vec3 origin = ray.Origin - closestSphere->Position;
	
	
	glm::vec3 hitPoint = origin + ray.Direction * hitDistance;

	Ray newRay;
	newRay.Origin = ray.Origin + ray.Direction * hitDistance;
	if (closestSphere->Albedo != glm::vec3(0.0f))
	{
		newRay.Direction = -lightDir;
		if (TraceRay(scene, newRay, bounce + 1) == glm::vec4(-0.0f, -0.0f, -0.0f, -0.0f))
		{
			float lightLevel = glm::max(glm::dot(Utils::Fast_normalize(hitPoint), -lightDir), 0.0f);
			return glm::vec4(closestSphere->Albedo * lightLevel, 1.0f);
		}
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		
		newRay.Direction = glm::normalize(hitPoint);
		return TraceRay(scene, newRay, bounce + 1);
	}
}