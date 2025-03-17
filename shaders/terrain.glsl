#include "types.glsl"

void terrainMaterial(HitData hit, inout SampledMaterial mat, Ray ray)
{
  mat.normal = hit.normal;
  mat.uv = hit.textureCoordinates - floor(hit.textureCoordinates);
  float beachFactor = smoothstep(2.0, 2.5, hit.position.y);
  mat.albedo = (1.0 - beachFactor) * vec3(0.9, 0.7, 0.2) + (beachFactor) * vec3(0.1, 0.15, 0.1);
  mat.metallic = 0.0;
  mat.alpha = 1.0;
  mat.roughness = 1.0;
  mat.lod = 0;
  mat.emissivity = vec3(0);
  mat.specular = 0;
  mat.f0 = -1;
}
