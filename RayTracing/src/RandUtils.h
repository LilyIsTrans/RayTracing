#pragma once
#include <cstdint>
#ifndef GLM_FORCE_INTRINSICS
#define GLM_FORCE_INTRINSICS
#endif // !GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>


constexpr uint32_t mash_seeds(uint32_t, uint32_t, uint32_t);


glm::vec3 rand_vec3(uint32_t x, uint32_t y, uint32_t z, float min, float max);


float float_from_uint32(uint32_t x);
float float_in_range_from_uint32(uint32_t x, float min, float max);
