// ReSharper disable file CppMissingIncludeGuard
// ReSharper disable file CppUnusedIncludeDirective

#ifdef __cplusplus
#pragma once
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
using ivec2 = glm::ivec2;
using ivec3 = glm::ivec3;
using ivec4 = glm::ivec4;
#endif

#define FAR_PLANE 1e12

#define MAX_MATERIALS 64
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
  vec3 emissivity
#ifdef __cplusplus
  = vec3(0);
#endif
  ;
  float _;
};

#include "objects.h"
#include "ubos.h"
