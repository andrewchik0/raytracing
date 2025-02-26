#define MAX_SPHERES 32
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

struct TriangleObject
{
  ivec3 indices;
  int materialIndex
#ifdef __cplusplus
  = 0
#endif
  ;
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
  vec4 tangent;
  vec4 bitangent;
};

#define BOUND_VOLUME_SIZE 3
#define BOUND_VOLUME_MIN_LEFT 0
#define BOUND_VOLUME_MAX_RIGHT 1
#define BOUND_VOLUME_COUNT_START 2
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
  vec4 triangleCountTrianglesStart;
};