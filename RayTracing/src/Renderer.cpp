#include "Renderer.h"

#include <execution>

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
	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];
	lightDirUpdated();

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (uint32_t i = 0; i < width; i++)
	{
		m_ImageHorizontalIter[i] = i;
	}
	for (uint32_t i = 0; i < height; i++)
	{
		m_ImageVerticalIter[i] = i;
	}

	ResetFrameIndex();
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{

	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetHeight() * m_FinalImage->GetWidth() * sizeof(glm::vec4));
#define MT 1
#if MT

	std::for_each(std::execution::par_unseq, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t y)
		{

			for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
			{

				glm::vec4 colour = PerPixel(x, y);
				m_AccumulationData[x + y * m_FinalImage->GetWidth()] += colour;

				glm::vec4 accumulatedColour = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
				accumulatedColour /= (float)m_FrameIndex;

				colour = glm::clamp(accumulatedColour, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(colour);
			}

		});
#else
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			
			glm::vec4 colour = PerPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += colour;

			glm::vec4 accumulatedColour = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColour /= (float)m_FrameIndex;

			colour = glm::clamp(accumulatedColour, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(colour);
		}
	}
#endif
	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}


glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 colour(0.0f);

	float multiplier = 1.0f;

	int bounces = 5;

	for (int i = 0; i < bounces; i++)
	{
		auto payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColour = glm::vec3(0.6f, 0.7f, 0.9f);
			colour += skyColour * multiplier;
			break;
		}

		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

		float lightIntensity;

		if (doShading)
			lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f);
		else
			lightIntensity = 1.0f;

		glm::vec3 sphereColour = material.Albedo;
		sphereColour *= lightIntensity;
		colour += sphereColour * multiplier;

		multiplier *= 0.5f;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, 
			payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}

	return glm::vec4(colour, 1.0f);
}


Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0

	// a: Ray origin
	// b: Ray direction
	// r: radius
	// t = hit distance
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();
	

	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];

		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction); //(bx^2 + by^2)
		float bo2 = glm::dot(origin, ray.Direction); //(2(axbx + ayby))
		float b = 2.0f * bo2;
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius; //(ax^2 + ay^2 - r^2)

		// Quadratic formula discriminant:
		// b^2 - 4ac

		float discriminant = b * b - (4.0f * a * c);
		if (discriminant < 0.0f)
			continue;

		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		if (closestT < hitDistance && closestT > 0.0f)
		{
			hitDistance = closestT;
			closestSphere = (int)i;
		}
	}

	if (closestSphere < 0)
		return Miss(ray);
	
	return ClosestHit(ray, hitDistance, closestSphere);
}

inline Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere.Position;


	payload.WorldPosition = origin + ray.Direction * hitDistance;

	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestSphere.Position;

	return payload;
}


inline Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}
