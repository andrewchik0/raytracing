#include "types.glsl"

float computePeaks(vec2 uv, sampler2D heightMap, float sampleSize)
{
  uv = uv * u_water.size;
  float texelSize = 1.0 / textureSize(heightMap, 0).x; // Texel size in UV space

  vec2 uvL = uv - vec2(texelSize, 0.0) * sampleSize;
  vec2 uvR = uv + vec2(texelSize, 0.0) * sampleSize;
  vec2 uvD = uv - vec2(0.0, texelSize) * sampleSize;
  vec2 uvU = uv + vec2(0.0, texelSize) * sampleSize;

  float h = getHeight(uv, 1.0, heightMap);
  float h00 = getHeight(uvL, 1.0, heightMap);
  float h10 = getHeight(uvR, 1.0, heightMap);
  float h01 = getHeight(uvD, 1.0, heightMap);
  float h11 = getHeight(uvU, 1.0, heightMap);

  // Compute local curvature (concavity) instead of just slope
  float foamFactor = (h00 + h01 + h10 + h11 - 4.0 * h);
  return clamp(-foamFactor * 20, 0, 1);
}

void waterMaterial(HitData hit, inout SampledMaterial mat, Ray ray)
{
  mat.normal = hit.normal;
  mat.uv = hit.textureCoordinates - floor(hit.textureCoordinates);
  mat.albedo = u_water.albedo;
  mat.metallic = 0.0;
  mat.alpha = .7;
  mat.roughness = u_water.roughness;
  mat.lod = 0;
  mat.emissivity = vec3(0);
  mat.specular = 0;
  mat.f0 = -1;
}
