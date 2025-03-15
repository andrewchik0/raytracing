// ReSharper disable CppMissingIncludeGuard

#define WATER_MATERIAL (MAX_MATERIALS + 1)
struct Water
{
  vec3 albedo;
  int isShown;

  float speed;
  float amplitude;
  float animationTime;
  float size;

  vec2 _;
  int samples;
  float roughness;
};

#define MANDELBULB_MATERIAL (MAX_MATERIALS + 2)
struct Mandelbulb
{
  vec3 position;
  int isShown;
};

#define SCENE_BINDING 1
#define SceneBufferStruct SceneBuffer             \
{                                                 \
  SphereObject u_spheres[MAX_SPHERES];            \
  PlaneObject u_planes[MAX_PLANES];               \
  Material u_materials[MAX_MATERIALS];            \
  Water u_water;                                  \
  Mandelbulb u_mandelbulb;                        \
  uint u_spheresCount;                            \
  uint u_planesCount;                             \
}

#define GLOBAL_DATA_BINDING 2
#define GlobalDataStruct GlobalData   \
{                                     \
  vec3 u_cameraPosition;              \
  float u_halfHeight;                 \
  vec3 u_cameraDirection;             \
  float u_halfWidth;                  \
  vec3 u_cameraRight;                 \
  float u_time;                       \
  vec3 u_cameraUp;                    \
  float u_gamma;                      \
  vec4 u_windowSize;                  \
  float u_exposure;                   \
  float u_blurSize;                   \
  int u_samples;                      \
  int u_bounces;                      \
  int u_maxTextureSize;               \
  int u_useFXAA;                      \
  int u_renderMode;                   \
  int u_interpolateNormals;           \
  int u_showTextures;                 \
  int u_postProcessing;               \
  int u_debugTextureLayer;            \
  int u_useSSAA;                      \
  int u_SSAAGridSize;                 \
  int u_accumulationIndex;            \
}

#define BVH_ENTRIES_BINDING 3
#define BVH_BINDING 4
#define VERTICES_BINDING 5

#define DEBUG_TEXTURE_LAYER_DEFAULT 0
#define DEBUG_TEXTURE_LAYER_NORMAL 1
#define DEBUG_TEXTURE_LAYER_ROUGHNESS 2
#define DEBUG_TEXTURE_LAYER_METALLIC 3
#define DEBUG_TEXTURE_LAYER_ALPHA 4
#define DEBUG_TEXTURE_LAYER_EMISSIVE 5
#define DEBUG_TEXTURE_LAYER_UV 6
#define DEBUG_TEXTURE_LAYER_SPECULAR 7

#ifdef __cplusplus
struct SceneBufferStruct;
struct GlobalDataStruct;
#endif
