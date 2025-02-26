// ReSharper disable CppMissingIncludeGuard
#define SCENE_BINDING 1
#define SceneBufferStruct SceneBuffer           \
{                                               \
  SphereObject spheres[MAX_SPHERES];            \
  PlaneObject planes[MAX_PLANES];               \
  Material materials[MAX_MATERIALS];            \
  uint volumesCount;                            \
  uint spheresCount;                            \
  uint planesCount;                             \
}

#define GLOBAL_DATA_BINDING 2
#define GlobalDataStruct GlobalData \
{                                   \
  vec4 cameraPosition;              \
  vec4 cameraDirection;             \
  vec4 cameraRight;                 \
  vec4 cameraUp;                    \
  vec4 windowSize;                  \
  float halfHeight;                 \
  float halfWidth;                  \
  float time;                       \
  float gamma;                      \
  float exposure;                   \
  float blurSize;                   \
  int samples;                      \
  int bounces;                      \
  int maxTextureSize;               \
  int useFXAA;                     \
  int renderMode;                  \
  float _; \
}

#ifdef __cplusplus
struct SceneBufferStruct;
struct GlobalDataStruct;
#endif
