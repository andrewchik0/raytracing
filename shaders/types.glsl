
#define FAR_PLANE 1e12

struct HitData
{
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
  vec2 textureCoordinates;
  uint materialIndex;
  float distance;
};

struct Ray
{
  vec3 origin;
  vec3 direction;
};

struct SampledMaterial
{
  vec2 uv;
  float lod;
  vec3 albedo;
  float roughness;
  float specular;
  float metallic;
  vec3 normal;
  float alpha;
  float f0;
  vec3 emissivity;
};
