#include "uniforms.h"
#include "utils.glsl"
#include "types.glsl"

float fresnelSchlick(float cosTheta, float f0)
{
  return f0 + (1.0 - f0) * pow(1 - cosTheta, 5.0);
}

/*
 * Returns `true` if ray should reflect, otherwise - `false`
 */
bool pbr(inout HitData hit, uint sampleCounter, uint bounceCounter, inout vec3 sampleColor, inout Ray ray)
{
  if (renderMode != 1 && showTextures != 1)
  {
    sampleColor = vec3(max(dot(normalize((vec3(1.0))) - 0.4, hit.normal), 0.02));
    return false;
  }

  vec2 texCoords = hit.textureCoordinates * materials[hit.materialIndex].textureCoordinatesMultiplier;
  loadUV(texCoords);
  vec2 dudx = getDDX();
  vec2 dudy = getDDY();
  float lambda = max(length(dudx), length(dudy));
  float lod = log2(max(lambda * textureSize(texArray, 0).x, 1.0));

  float bias = 1e-5;

  vec3 albedo =
    float(materials[hit.materialIndex].textureIndex != -1) *
    textureLod(texArray, vec3(texCoords, materials[hit.materialIndex].textureIndex), lod).rgb +
    float(materials[hit.materialIndex].textureIndex == -1) * materials[hit.materialIndex].albedo;

  float roughness =
    float(materials[hit.materialIndex].metallicTextureIndex != -1) *
    (1.0 - textureLod(texArray, vec3(texCoords, materials[hit.materialIndex].metallicTextureIndex), lod).r) +

    pow(float(materials[hit.materialIndex].specularTextureIndex != -1) *
    (
      bool(materials[hit.materialIndex].roughness) ?
      1.0 - textureLod(texArray, vec3(texCoords, materials[hit.materialIndex].specularTextureIndex), lod).r :
      textureLod(texArray, vec3(texCoords, materials[hit.materialIndex].specularTextureIndex), lod).r
    ), 3) +
    float(materials[hit.materialIndex].metallicTextureIndex == -1) * float(materials[hit.materialIndex].specularTextureIndex == -1) * materials[hit.materialIndex].roughness;

  mat3 TBN = mat3(hit.tangent, hit.bitangent, hit.normal);
  vec3 normal =
    float((materials[hit.materialIndex].normalTextureIndex != -1)) *
    TBN * (textureLod(texArray, vec3(texCoords, materials[hit.materialIndex].normalTextureIndex), lod).rgb * 2.0 - 1) +
    float((materials[hit.materialIndex].normalTextureIndex == -1)) * hit.normal;
  float alpha = textureLod(texArray, vec3(texCoords, materials[hit.materialIndex].textureIndex), lod).a;
  vec3 e = materials[hit.materialIndex].emissivity;

  if (e.x + e.y + e.z != 0)
  {
    sampleColor = e;
    return false;
  }

  if (renderMode != 1)
  {
    sampleColor = albedo * max(dot(normalize((vec3(1.0))), normal), 0.1);
//    sampleColor = vec3(lod);
    return false;
  }

  if (alpha > 0.8)
  {
    sampleColor = sampleColor * albedo + e;
    ray.origin = hit.position + normal * bias;

    vec3 coatNormal = normalize(normal + rand3((ray.direction + ray.origin) * (sampleCounter + 1.0)) * 0.02 * (roughness + 0.912));
    float F = fresnelSchlick(dot(ray.direction, normal), roughness + 0.978);

    if (random(ray.origin.x + ray.origin.y + ray.origin.z) < F)
    {
      ray.direction = reflect(ray.direction, coatNormal);
    }
    else
    {
      normal = normalize(normal + rand3((ray.direction + ray.origin) * (sampleCounter + 1.0)) * roughness);
      ray.direction = reflect(ray.direction, normal);
    }
  }
  else
  {
    ray.origin = hit.position + ray.direction * bias;
  }
  return true;
}