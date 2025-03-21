#include "types.glsl"
#include "utils.glsl"

void terrainMaterial(HitData hit, inout SampledMaterial mat, Ray ray)
{
  const float multiplier = 100;
  mat.uv = hit.textureCoordinates * multiplier;
  mat.lod = 0;
  mat3 TBN = mat3(hit.tangent, hit.bitangent, hit.normal);

  vec3 grassAlbedo = textureLod(u_texArray, vec3(mat.uv, u_terrain.albedoIndexGrass), mat.lod).rgb;
  vec3 grassNormal = TBN * (textureLod(u_texArray, vec3(mat.uv, u_terrain.normalIndexGrass), mat.lod).rgb * 2.0 - 1);
  float grassMetallic = textureLod(u_texArray, vec3(mat.uv, u_terrain.metallicIndexGrass), mat.lod).r;

  vec3 sandAlbedo = textureLod(u_texArray, vec3(mat.uv, u_terrain.albedoIndexSand), mat.lod).rgb;
  vec3 sandNormal = TBN * (textureLod(u_texArray, vec3(mat.uv, u_terrain.normalIndexSand), mat.lod).rgb * 2.0 - 1);
  float sandMetallic = textureLod(u_texArray, vec3(mat.uv, u_terrain.metallicIndexSand), mat.lod).r;

  float beachFactor = 1.0 - smoothstep(2.0, 2.5, hit.position.y);

  mat.albedo = (beachFactor) * sandAlbedo + (1.0 - beachFactor) * grassAlbedo;
  mat.metallic = (beachFactor) * sandMetallic + (1.0 - beachFactor) * grassMetallic;
  mat.normal = (beachFactor) * sandNormal + (1.0 - beachFactor) * grassNormal;

  mat.roughness = 1.0;
  mat.emissivity = vec3(0);
  mat.specular = 0;
  mat.alpha = 1.0;
  mat.f0 = -1;
}
