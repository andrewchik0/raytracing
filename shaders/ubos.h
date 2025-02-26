// ReSharper disable CppMissingIncludeGuard
#define SCENE_BINDING 1
#define SceneBufferStruct SceneBuffer           \
{                                               \
  SphereObject spheres[MAX_SPHERES];            \
  PlaneObject planes[MAX_PLANES];               \
  Vertex vertices[MAX_VERTICES];                \
  Material materials[MAX_MATERIALS];            \
  BoundingVolume volumes[MAX_BOUNDING_VOLUMES]; \
  uint volumesCount;                            \
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
  int maxTextureSize;               \
}

#ifdef __cplusplus
struct SceneBufferStruct;
struct GlobalDataStruct;
#endif
