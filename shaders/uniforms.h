// ReSharper disable once CppMissingIncludeGuard
#ifdef __cplusplus
#pragma once

#include <glm\glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

#define SCENE_BINDING 1
#define MAX_SPHERES 32
struct SphereObject
{
  vec3 center;
  float radius;
  vec4 albedo;
};

#define MAX_PLANES 32
struct PlaneObject
{
  vec3 normal;
  float distance;
  vec4 albedo;
};
