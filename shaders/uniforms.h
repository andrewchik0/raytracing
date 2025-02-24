// ReSharper disable once CppMissingIncludeGuard
#ifdef __cplusplus
#pragma once
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
using ivec4 = glm::ivec4;
#endif

#define FAR_PLANE 1e12

#define UNDEFINED_OBJECT 0

#define SPHERE_OBJECT 1
#define MAX_SPHERES 32
struct SphereObject
{
  vec3 center;
  float radius;
  vec3 _;
  uint materialIndex;
};

#define PLANE_OBJECT 2
#define MAX_PLANES 6
struct PlaneObject
{
  vec3 normal;
  float distance;
  vec3 _;
  uint materialIndex;
};

#define TRIANGLE_OBJECT 3
#define MAX_TRIANGLES 1024
struct TriangleObject
{
  ivec4 indices;
};

#define MAX_VERTICES 512
struct Vertex
{
  vec4 position;
  vec4 normal;
  vec4 tangent;
  vec4 bitangent;
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
  vec3 emissivity
#ifdef __cplusplus
  = vec3(0);
#endif
  ;
  float _;
};

#define MAX_BOUNDING_VOLUMES 256
struct BoundingVolume
{
  vec3 min;
  int nodeLeft
#ifdef __cplusplus
  = -1
#endif
  ;
  vec3 max;
  int nodeRight
#ifdef __cplusplus
  = -1
#endif
  ;
  ivec4 triangleCountTrianglesStart;
};

#define SCENE_BINDING 1
#define SceneBufferStruct SceneBuffer           \
{                                               \
  SphereObject spheres[MAX_SPHERES];            \
  PlaneObject planes[MAX_PLANES];               \
  TriangleObject triangles[MAX_TRIANGLES];      \
  Vertex vertices[MAX_VERTICES];                \
  Material materials[MAX_MATERIALS];            \
  BoundingVolume volumes[MAX_BOUNDING_VOLUMES]; \
  uint volumesCount;                            \
  uint trianglesCount;                          \
  uint spheresCount;                            \
  uint planesCount;                             \
}

#define GLOBAL_DATA_BINDING 2
#define GlobalDataStruct GlobalData \
{                                   \
  vec3 cameraPosition;              \
  float halfHeight;                 \
  vec3 cameraDirection;             \
  float halfWidth;                  \
  vec3 cameraRight;                 \
  float time;                       \
  vec3 cameraUp;                    \
  int samples;                      \
  vec2 windowSize;                  \
  int bounces;                      \
  bool useFXAA;                     \
  float gamma;                      \
  float exposure;                   \
  float blurSize;                   \
}

#ifdef __cplusplus
struct SceneBufferStruct;
struct GlobalDataStruct;
#endif
