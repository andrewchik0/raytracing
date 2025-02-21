// ReSharper disable once CppMissingIncludeGuard
#ifdef __cplusplus
#pragma once

#include <glm/glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

#define FAR_PLANE 1000000.0f

#define SCENE_BINDING 1
#define SKYBOX_BINDING 6
#define UNDEFINED_OBJECT 0

#define MAX_SPHERES 32
#define SPHERE_OBJECT 1
struct SphereObject
{
  vec3 center;
  float radius;
  vec3 _;
  uint materialIndex;
};

#define MAX_PLANES 32
#define PLANE_OBJECT 2
struct PlaneObject
{
  vec3 normal;
  float distance;
  vec3 _;
  uint materialIndex;
};

#define MAX_MATERIALS 32
struct Material
{
  vec3 albedo;
  float roughness;
  float textureCoordinatesMultiplier
#ifdef __cplusplus
  = 1.0f;
#endif
  ;
  int textureIndex
#ifdef __cplusplus
  = -1;
#endif
  ;
  int normalTextureIndex
#ifdef __cplusplus
  = -1;
#endif
  ;
  int metallicTextureIndex
#ifdef __cplusplus
  = -1;
#endif
  ;
};

#define SceneBufferStruct SceneBuffer   \
{                                       \
  SphereObject spheres[MAX_SPHERES];    \
  PlaneObject planes[MAX_PLANES];       \
  Material materials[MAX_MATERIALS];    \
  uint spheresCount;                    \
  uint planesCount;                     \
}

