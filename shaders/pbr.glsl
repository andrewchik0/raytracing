#include "uniforms.h"
#include "utils.glsl"
#include "types.glsl"

float fresnelSchlick(float cosTheta, float f0)
{
  return f0 + (1.0 - f0) * pow(1 - cosTheta, 5.0);
}

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
  vec3 emissivity;
};

vec3 reflectance(Ray ray, inout SampledMaterial material)
{
  float r = max(material.roughness, 0.01);
  float m = material.metallic;

  if (m > 0.99)
  {
    return reflect(ray.direction, material.normal);
  }
  else
  {
    vec3 diffuse = randomHemisphereDirection(material.normal, material.normal + ray.direction + gl_GlobalInvocationID);
    vec3 specular = reflect(ray.direction, material.normal);

    return mix(specular, diffuse, r);
  }
}

SampledMaterial sampleMaterial(HitData hit, inout SampledMaterial mat)
{
  mat.uv = hit.textureCoordinates * materials[hit.materialIndex].textureCoordinatesMultiplier;
  loadUV(mat.uv, hit.materialIndex);
  vec2 dudx = getDDX();
  vec2 dudy = getDDY();
  float lambda = max(length(dudx), length(dudy));
  mat.lod = log2(max(lambda * textureSize(texArray, 0).x, 1.0));

  mat.albedo =
    float(materials[hit.materialIndex].textureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].textureIndex), mat.lod).rgb +
    float(materials[hit.materialIndex].textureIndex == -1) * materials[hit.materialIndex].albedo;

  mat.metallic =
    float(materials[hit.materialIndex].metallicTextureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].metallicTextureIndex), mat.lod).b;

  mat.specular =
    float(materials[hit.materialIndex].specularTextureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].specularTextureIndex), mat.lod).r +
    float(materials[hit.materialIndex].specularTextureIndex == -1) * 0.5;

  mat.roughness =
    float(materials[hit.materialIndex].roughnessTextureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].roughnessTextureIndex), mat.lod).g +
    float(materials[hit.materialIndex].roughnessTextureIndex == -1) * 0.5;

  mat3 TBN = mat3(hit.tangent, hit.bitangent, hit.normal);
  mat.normal =
    float((materials[hit.materialIndex].normalTextureIndex != -1)) *
    TBN * (textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].normalTextureIndex), mat.lod).rgb * 2.0 - 1) +
    float((materials[hit.materialIndex].normalTextureIndex == -1)) * hit.normal;

  mat.emissivity =
    float(materials[hit.materialIndex].emissiveTextureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].emissiveTextureIndex), mat.lod).rgb *
    materials[hit.materialIndex].emissivity +
    float(materials[hit.materialIndex].emissiveTextureIndex == -1) *
    materials[hit.materialIndex].emissivity;

  mat.alpha = textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].textureIndex), mat.lod).a;
  return mat;
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

  float bias = 1e-5;
  SampledMaterial mat;
  sampleMaterial(hit, mat);

  if (renderMode != 1)
  {
    switch (debugTextureLayer)
    {
    case DEBUG_TEXTURE_LAYER_DEFAULT:
      sampleColor = mat.albedo;
      return false;
    case DEBUG_TEXTURE_LAYER_NORMAL:
      sampleColor = mat.normal;
      return false;
    case DEBUG_TEXTURE_LAYER_ROUGHNESS:
      sampleColor = vec3(mat.roughness);
      return false;
    case DEBUG_TEXTURE_LAYER_METALLIC:
      sampleColor = vec3(mat.metallic);
      return false;
    case DEBUG_TEXTURE_LAYER_SPECULAR:
      sampleColor = vec3(mat.specular);
      return false;
    case DEBUG_TEXTURE_LAYER_ALPHA:
      sampleColor = vec3(mat.alpha);
      return false;
    case DEBUG_TEXTURE_LAYER_EMISSIVE:
      sampleColor = mat.emissivity;
      return false;
    case DEBUG_TEXTURE_LAYER_UV:
      sampleColor = vec3(mat.uv, 0.0);
      return false;
    }
  }

  if (mat.emissivity.x + mat.emissivity.y + mat.emissivity.z > 0)
  {
    sampleColor = mat.emissivity;
    return false;
  }

  float fresnel = fresnelSchlick(abs(dot(-ray.direction, mat.normal)), mat.alpha < 0.99 ? 0.2 : 0.4);
  float random0to1 =
    random(ray.direction.x + gl_LocalInvocationID.x + gl_GlobalInvocationID.y) +
    random(ray.direction.z + gl_LocalInvocationID.y + gl_GlobalInvocationID.y) +
    random(ray.direction.y + gl_LocalInvocationID.x + gl_GlobalInvocationID.x);
  random0to1 /= 3;

  if (mat.alpha < 0.99 && random0to1 > fresnel)
  {
    ray.origin = hit.position + ray.direction * bias;
    return true;
  }

  if (random0to1 > fresnel || mat.roughness > 0.45)
  {
    mat.roughness = 1.0;
    sampleColor = sampleColor * mat.albedo + mat.emissivity;
    ray.origin = hit.position + mat.normal * bias;
    ray.direction = reflectance(ray, mat);
  }
  else
  {
    mat.albedo = vec3(1);
    mat.roughness = 0;
    sampleColor = sampleColor * mat.albedo + mat.emissivity;
    ray.origin = hit.position + mat.normal * bias;
    ray.direction = reflectance(ray, mat);
  }
  return true;
}