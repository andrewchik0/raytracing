#include "uniforms.h"
#include "utils.glsl"
#include "types.glsl"
#include "water.glsl"

vec3 hash3(uvec3 p)
{
  p = 1103515245U * ((p >> 1U) ^ p.yzx);
  p = 1103515245U * ((p >> 1U) ^ p.yzx);
  return vec3(p & 0xFFFFFFU) / float(0xFFFFFFU);
}

float fresnelSchlik(float cosTheta, float f0)
{
  return f0 + (1.0 - f0) * pow(1 - max(cosTheta, 0.0), 5.0);
}

SampledMaterial sampleMaterial(HitData hit, inout SampledMaterial mat, Ray ray)
{
  if (hit.materialIndex == WATER_MATERIAL)
  {
    waterMaterial(hit, mat, ray);
    return mat;
  }

  mat.f0 = -1.0;

  mat.uv = hit.textureCoordinates * materials[hit.materialIndex].textureCoordinatesMultiplier;
  loadUV(mat.uv, hit.materialIndex);
  vec2 dudx = getDDX();
  vec2 dudy = getDDY();
  float lambda = max(length(dudx), length(dudy));
  mat.lod = log2(max(lambda * textureSize(texArray, 0).x, 1.0));

  mat.albedo = max(
    float(materials[hit.materialIndex].textureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].textureIndex), mat.lod).rgb +
    float(materials[hit.materialIndex].textureIndex == -1) * materials[hit.materialIndex].albedo,
    vec3(0.05));

  mat.metallic =
    float(materials[hit.materialIndex].metallicTextureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].metallicTextureIndex), mat.lod).b +
    float(materials[hit.materialIndex].metallicTextureIndex == -1) * materials[hit.materialIndex].metallic;

  mat.specular =
    float(materials[hit.materialIndex].specularTextureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].specularTextureIndex), mat.lod).r * materials[hit.materialIndex].specular +
    float(materials[hit.materialIndex].specularTextureIndex == -1) * materials[hit.materialIndex].specular;

  mat.roughness =
    materials[hit.materialIndex].sg == 1 ?
      (1.0 - textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].roughnessTextureIndex), mat.lod).a) :
      float(materials[hit.materialIndex].roughnessTextureIndex != -1) *
      textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].roughnessTextureIndex), mat.lod).g +
      float(materials[hit.materialIndex].roughnessTextureIndex == -1) * materials[hit.materialIndex].roughness;

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

  mat.alpha =
    float(materials[hit.materialIndex].textureIndex != -1) *
    textureLod(texArray, vec3(mat.uv, materials[hit.materialIndex].textureIndex), mat.lod).a +
    float(materials[hit.materialIndex].textureIndex == -1) * materials[hit.materialIndex].alpha;
  return mat;
}

/*
 * Returns `true` if ray should reflect, otherwise - `false`
 */
bool pbr(inout HitData hit, uint sampleCounter, uint bounceCounter, inout vec3 sampleColor, inout Ray ray)
{
  if (renderMode != 1 && showTextures != 1)
  {
    sampleColor = vec3(max(dot(normalize((vec3(1.0))), hit.normal) + 1.0, 0.02)) / 3.5;
    return false;
  }

  float bias = 1e-5;
  SampledMaterial mat;
  sampleMaterial(hit, mat, ray);

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

  // Pass ray through if alpha is near zero (for grass, leaves, etc...)
  if (mat.alpha <= 0.1)
  {
    ray.origin = hit.position + ray.direction * bias;
    return true;
  }

  // Do not refract if material is emissive
  if (mat.emissivity.x + mat.emissivity.y + mat.emissivity.z > 0)
  {
    sampleColor = mat.emissivity;
    return false;
  }

  // Convert specular to metallic if needed
  if (materials[hit.materialIndex].sg == 1)
  {
    mat.metallic = clamp((mat.specular - 0.04) / (1.0 - 0.04), 0.0, 1.0);
    if (mat.specular < 1e-3)
    {
      mat.roughness = 1.0;
    }
  }

  // Set new ray origin
  ray.origin = hit.position + mat.normal * bias;

  // Random value for mixing
  float random0to1 =
    (random(ray.direction.x + gl_LocalInvocationID.x + gl_GlobalInvocationID.y + sampleCounter) +
    random(ray.direction.z + gl_LocalInvocationID.y + gl_GlobalInvocationID.y + sampleCounter) +
    random(ray.direction.y + gl_LocalInvocationID.x + gl_GlobalInvocationID.x + sampleCounter)) / 3;

  // Calculate metallic & roughness coefficients
  float f0 = mat.f0 == -1.0 ? mix(0.3, .35, 1.0 - mat.metallic) : mat.f0;
  float fresnel = mat.f0 == -1.0 ? min(fresnelSchlik(abs(dot(-ray.direction, mat.normal)), f0), 0.4) : fresnelSchlik(abs(dot(-ray.direction, mat.normal)), f0);

  // Get random and specular new ray directions
  vec3 seed = hash3(uvec3(uvec2(texCoord * windowSize.xy), int(time + sampleCounter)) ^ floatBitsToUint(ray.direction * 4096.0)); // tired of looking for nice seed, just use hash
  vec3 randomDir = randomHemisphereDirection(mat.normal, seed);
  vec3 specularDir = reflect(ray.direction, mat.normal);

  vec3 metallicDir, metallicColor, dielectricDir = randomDir, dielectricColor = sampleColor * mat.albedo;
  if (fresnel > random0to1)
  {
    dielectricDir = mix(specularDir, randomDir, mat.roughness);
    dielectricColor = sampleColor + mat.emissivity;
  }
  else if (mat.alpha > 0.1 && mat.alpha < 0.99)
  {
    ray.origin = hit.position + ray.direction * bias;
    return true;
  }
  metallicDir = mix(specularDir, randomDir, mat.roughness);
  metallicColor = sampleColor * mat.albedo + mat.emissivity;

  ray.direction = mix(dielectricDir, metallicDir, mat.metallic);
  sampleColor = mix(dielectricColor, metallicColor, mat.metallic);

  return true;
}