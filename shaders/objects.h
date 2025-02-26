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
struct TriangleObject
{
  ivec3 indices;
  int materialIndex
#ifdef __cplusplus
  = 0
#endif
  ;
};

#define MAX_VERTICES 512
struct Vertex
{
  vec4 position;
  vec4 normal;
  vec4 tangent;
  vec4 bitangent;
};