#define MAX_SPHERES 128
struct SphereObject
{
  vec3 center;
  float radius;
  vec3 _;
  uint materialIndex;
};

#define MAX_PLANES 6
struct PlaneObject
{
  vec3 normal;
  float distance;
  vec3 _;
  uint materialIndex;
};

#define VERTEX_SIZE 4
#define VERTEX_POSITION 0
#define VERTEX_NORMAL 1
#define VERTEX_TANGENT 2
#define VERTEX_BITANGENT 3
struct Vertex
{
  vec4 position;
  vec4 normal;
  vec4 tangent;    // 4th component used for texture coordinates U
  vec4 bitangent;  // 4th component used for texture coordinates V
};

#define BOUND_VOLUME_MIN_LEFT 0
#define BOUND_VOLUME_MAX_RIGHT 1
#define BOUND_VOLUME_TRIANGLE 2
#define BOUND_VOLUME_SIZE 3
struct BoundingVolume
{
  vec3 min;
  float nodeLeft
#ifdef __cplusplus
  = -1
#endif
  ;
  vec3 max;
  float nodeRight
#ifdef __cplusplus
  = -1
#endif
  ;
  vec4 triangle;
};

struct BoundingVolumeEntry
{
  vec2 _;
  int applyWind;
  int index;
  mat4 transform;
};

struct PointLight
{
  vec3 position;
  float radius;
  vec3 intensity;
  float _;
};
