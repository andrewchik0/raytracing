#include "types.glsl"

void terrainMaterial(HitData hit, inout SampledMaterial mat, Ray ray)
{
  const float multiplier = 500;
  mat.uv = hit.textureCoordinates * multiplier;
  loadUV(mat.uv, hit.materialIndex);
  vec2 dudx = getDDX();
  vec2 dudy = getDDY();
  float lambda = max(length(dudx), length(dudy));
  mat.lod = log2(max(lambda * textureSize(u_texArray, 0).x, 1.0));
  mat3 TBN = mat3(hit.tangent, hit.bitangent, hit.normal);

  int albedoIndexGrass = 0;
  int normalIndexGrass = 1;
  int metallicIndexGrass = 2;
  int albedoIndexSand = 3;
  int normalIndexSand = 4;
  int metallicIndexSand = 5;

  vec3 grassAlbedo = textureLod(u_texArray, vec3(mat.uv, albedoIndexGrass), mat.lod).rgb;
  vec3 grassNormal = TBN * (textureLod(u_texArray, vec3(mat.uv, normalIndexGrass), mat.lod).rgb * 2.0 - 1);
  float grassMetallic = textureLod(u_texArray, vec3(mat.uv, metallicIndexGrass), mat.lod).r;

  vec3 sandAlbedo = textureLod(u_texArray, vec3(mat.uv, albedoIndexSand), mat.lod).rgb;
  vec3 sandNormal = TBN * (textureLod(u_texArray, vec3(mat.uv, normalIndexSand), mat.lod).rgb * 2.0 - 1);
  float sandMetallic = textureLod(u_texArray, vec3(mat.uv, metallicIndexSand), mat.lod).r;

  float beachFactor = smoothstep(2.0, 2.5, hit.position.y);

  mat.albedo = (1.0 - beachFactor) * sandAlbedo + (beachFactor) * grassAlbedo;
  mat.metallic = (1.0 - beachFactor) * sandMetallic + (beachFactor) * grassMetallic;
  mat.normal = (1.0 - beachFactor) * sandNormal + (beachFactor) * grassNormal;

  mat.roughness = 1.0;
  mat.emissivity = vec3(0);
  mat.specular = 0;
  mat.alpha = 1.0;
  mat.f0 = -1;
}
