#include "RandUtils.h"
#ifndef GLM_FORCE_INTRINSICS
#define GLM_FORCE_INTRINSICS
#endif // !GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <iostream>

constexpr uint32_t Squirrel3(uint32_t x)
{
	constexpr uint32_t BIT_NOISE1 = 0xB5297A4D;
	constexpr uint32_t BIT_NOISE2 = 0x68E31DA4;
	constexpr uint32_t BIT_NOISE3 = 0x1B56C4E9;

	x *= BIT_NOISE1;
	x ^= (x >> 8);
	x += BIT_NOISE2;
	x ^= (x << 8);
	x *= BIT_NOISE3;
	x ^= (x >> 8);
	return x;
}


float float_from_uint32(uint32_t x)
{
	return (x >> 8) * 0x1.0p-24f;
}

constexpr uint32_t mash_seeds(uint32_t x, uint32_t y, uint32_t z)
{
	return x ^ y ^ z;
}

glm::vec3 rand_vec3(uint32_t x, uint32_t y, uint32_t z, float min, float max)
{
	return glm::vec3(float_in_range_from_uint32(Squirrel3(mash_seeds(x, y, z)), min, max), float_in_range_from_uint32(Squirrel3(mash_seeds(y, z, x)), min, max), float_in_range_from_uint32(Squirrel3(mash_seeds(z, x, y)), min, max));
}

float float_in_range_from_uint32(uint32_t x, float min, float max)
{
	return (float_from_uint32(x) * (max - min)) + min;
}
