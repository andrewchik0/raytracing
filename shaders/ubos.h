// ReSharper disable CppMissingIncludeGuard

#define WATER_MATERIAL (MAX_MATERIALS + 1)
struct Water
{
  int isShown;
  float speed;
  float amplitude;
  float animationTime;
  vec2 _;
  float size;
  int samples;
};

#define MANDELBULB_MATERIAL (MAX_MATERIALS + 2)
struct Mandelbulb
{
  vec3 position;
  int isShown;
};

#define SCENE_BINDING 1
#define SceneBufferStruct SceneBuffer           \
{                                               \
  SphereObject spheres[MAX_SPHERES];            \
  PlaneObject planes[MAX_PLANES];               \
  Material materials[MAX_MATERIALS];            \
  Water water;                                  \
  Mandelbulb mandelbulb;                        \
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
  float gamma;                      \
  vec4 windowSize;                  \
  float exposure;                   \
  float blurSize;                   \
  int samples;                      \
  int bounces;                      \
  int maxTextureSize;               \
  int useFXAA;                      \
  int renderMode;                   \
  int interpolateNormals;           \
  int showTextures;                 \
  int postProcessing;               \
  int debugTextureLayer;            \
  int useSSAA;                      \
  int SSAAGridSize;                 \
  int accumulationIndex;            \
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
