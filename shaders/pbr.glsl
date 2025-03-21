#include "uniforms.h"
#include "utils.glsl"
#include "types.glsl"
#include "water.glsl"
#include "hit.glsl"
#include "terrain.glsl"

layout(std430, binding = POINT_LIGHTS_BINDING) buffer PointLightsBuffer
{
  PointLight pointLights[];
};

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
  if (hit.materialIndex == TERRAIN_MATERIAL)
  {
    terrainMaterial(hit, mat, ray);
    return mat;
  }
  if (hit.materialIndex == MANDELBULB_MATERIAL)
  {
    mandelbulbMaterial(hit, mat, ray);
    return mat;
  }

  mat.f0 = -1.0;

  mat.uv = hit.textureCoordinates * u_materials[hit.materialIndex].textureCoordinatesMultiplier;
  mat.lod = 0;

  mat.albedo = max(
    float(u_materials[hit.materialIndex].textureIndex != -1) *
    textureLod(u_texArray, vec3(mat.uv, u_materials[hit.materialIndex].textureIndex), mat.lod).rgb +
    float(u_materials[hit.materialIndex].textureIndex == -1) * u_materials[hit.materialIndex].albedo,
    vec3(0.05));

  mat.metallic =
    float(u_materials[hit.materialIndex].metallicTextureIndex != -1) *
    textureLod(u_texArray, vec3(mat.uv, u_materials[hit.materialIndex].metallicTextureIndex), mat.lod).b +
    float(u_materials[hit.materialIndex].metallicTextureIndex == -1) * u_materials[hit.materialIndex].metallic;

  mat.specular =
    float(u_materials[hit.materialIndex].specularTextureIndex != -1) *
    textureLod(u_texArray, vec3(mat.uv, u_materials[hit.materialIndex].specularTextureIndex), mat.lod).r * u_materials[hit.materialIndex].specular +
    float(u_materials[hit.materialIndex].specularTextureIndex == -1) * u_materials[hit.materialIndex].specular;

  mat.roughness =
    u_materials[hit.materialIndex].sg == 1 ?
      (1.0 - textureLod(u_texArray, vec3(mat.uv, u_materials[hit.materialIndex].roughnessTextureIndex), mat.lod).a) :
      float(u_materials[hit.materialIndex].roughnessTextureIndex != -1) *
      textureLod(u_texArray, vec3(mat.uv, u_materials[hit.materialIndex].roughnessTextureIndex), mat.lod).g +
      float(u_materials[hit.materialIndex].roughnessTextureIndex == -1) * u_materials[hit.materialIndex].roughness;

  mat3 TBN = mat3(hit.tangent, hit.bitangent, hit.normal);
  mat.normal =
    float((u_materials[hit.materialIndex].normalTextureIndex != -1)) *
    TBN * (textureLod(u_texArray, vec3(mat.uv, u_materials[hit.materialIndex].normalTextureIndex), mat.lod).rgb * 2.0 - 1) +
    float((u_materials[hit.materialIndex].normalTextureIndex == -1)) * hit.normal;

  mat.emissivity =
    float(u_materials[hit.materialIndex].emissiveTextureIndex != -1) *
    textureLod(u_texArray, vec3(mat.uv, u_materials[hit.materialIndex].emissiveTextureIndex), mat.lod).rgb *
    u_materials[hit.materialIndex].emissivity +
    float(u_materials[hit.materialIndex].emissiveTextureIndex == -1) *
    u_materials[hit.materialIndex].emissivity;

  mat.alpha =
    float(u_materials[hit.materialIndex].textureIndex != -1) *
    textureLod(u_texArray, vec3(mat.uv, u_materials[hit.materialIndex].textureIndex), mat.lod).a +
    float(u_materials[hit.materialIndex].textureIndex == -1) * u_materials[hit.materialIndex].alpha;
  return mat;
}

void drawTextureLayer(inout vec3 sampleColor, inout SampledMaterial mat)
{
  switch (u_debugTextureLayer)
  {
    case DEBUG_TEXTURE_LAYER_DEFAULT:
      sampleColor = mat.albedo;
      return;
    case DEBUG_TEXTURE_LAYER_NORMAL:
      sampleColor = mat.normal;
      return;
    case DEBUG_TEXTURE_LAYER_ROUGHNESS:
      sampleColor = vec3(mat.roughness);
      return;
    case DEBUG_TEXTURE_LAYER_METALLIC:
      sampleColor = vec3(mat.metallic);
      return;
    case DEBUG_TEXTURE_LAYER_SPECULAR:
      sampleColor = vec3(mat.specular);
      return;
    case DEBUG_TEXTURE_LAYER_ALPHA:
      sampleColor = vec3(mat.alpha);
      return;
    case DEBUG_TEXTURE_LAYER_EMISSIVE:
      sampleColor = mat.emissivity;
      return;
    case DEBUG_TEXTURE_LAYER_UV:
      sampleColor = vec3(mat.uv, 0.0);
      return;
  }
}

/*
 * Returns `true` if ray should reflect or refract, otherwise - `false`
 */
bool pbr(inout HitData hit, uint sampleCounter, uint bounceCounter, inout vec3 sampleColor, inout Ray ray)
{
  if (u_renderMode != 1 && u_showTextures != 1)
  {
    sampleColor = vec3(max(dot(normalize((vec3(1.0))), hit.normal) + 1.0, 0.02)) / 3.5;
    return false;
  }

  float bias = max(1e-6 * hit.distance, 1e-6);
  SampledMaterial mat;
  sampleMaterial(hit, mat, ray);

  if (u_renderMode != 1)
  {
    drawTextureLayer(sampleColor, mat);
    return false;
  }

  // Pass ray through if alpha is near zero (for grass, leaves, etc...)
  if (mat.alpha <= 0.1)
  {
    ray.origin = hit.position + ray.direction * 0.001;
    return true;
  }

  // Do not refract if material is emissive
  if (mat.emissivity.x + mat.emissivity.y + mat.emissivity.z > 0)
  {
    sampleColor = mat.emissivity;
    return false;
  }

  // Convert specular to metallic if needed
  if (u_materials[hit.materialIndex].sg == 1)
  {
    mat.metallic = clamp((mat.specular - 0.04) / (1.0 - 0.04), 0.0, 1.0);
    if (mat.specular < 1e-3)
    {
      mat.roughness = 1.0;
    }
  }

  vec3 seed = hash3(uvec3(uvec2(texCoord * u_windowSize.xy), int(u_time * 1000 + sampleCounter)) ^ floatBitsToUint(ray.direction * 4096.0) ^ floatBitsToUint(ray.origin * 1024.0));

  // Set new ray origin
  ray.origin = hit.position + mat.normal * bias;

  // Random value for mixing
  float random0to1 = (random(seed.x) + random(seed.y) + random(seed.z)) / 3;

  // Calculate fresnel coefficient
  float f0 = mat.f0 == -1.0 ? mix(0.3, .35, 1.0 - mat.metallic) : mat.f0;
  float fresnel = mat.f0 == -1.0 ? min(fresnelSchlik(abs(dot(-ray.direction, mat.normal)), f0), 0.4) : fresnelSchlik(abs(dot(-ray.direction, mat.normal)), f0);

  // Get random and specular new ray directions
  vec3 randomDir = randomHemisphereDirection(mat.normal, seed);
  vec3 specularDir = reflect(ray.direction, mat.normal);

  vec3 metallicDir, metallicColor, dielectricDir = randomDir, dielectricColor = sampleColor * mat.albedo;
  if (fresnel > random0to1)
  {
    dielectricDir = mix(specularDir, randomDir, mat.roughness);
    dielectricColor = sampleColor + mat.emissivity;
  }
  else if (mat.alpha > 0.1 && mat.alpha < 0.99 && random(seed.x + seed.y + seed.z) > mat.alpha / 2)
  {
    ray.origin = hit.position + ray.direction * bias;
    return true;
  }
  metallicDir = mix(specularDir, randomDir, mat.roughness);
  metallicColor = sampleColor * mat.albedo + mat.emissivity;

  ray.direction = mix(dielectricDir, metallicDir, mat.metallic);
  sampleColor = mix(dielectricColor, metallicColor, mat.metallic);

  if (bounceCounter < 1)
    for (int i = 0; i < u_pointLightsCount; i++)
    {
      pointLights[i].radius = 0;
      pointLights[i].intensity = vec3(1);
      pointLights[i].position += randomOnSphere(seed) * pointLights[i].radius;

      Ray pointLightRay;
      pointLightRay.direction = normalize(pointLights[i].position - hit.position);
      pointLightRay.origin = hit.position + pointLightRay.direction * bias;
      HitData pointLightHit = closestHit(pointLightRay);
      if (pointLightHit.distance == FAR_PLANE);
        sampleColor += sampleColor * max(dot(hit.normal, normalize(pointLights[i].position - hit.position)), 0) * pointLights[i].intensity / pow(distance(pointLights[i].position, hit.position), 2.0);
    }

  return true;
}