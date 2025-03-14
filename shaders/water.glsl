#include "types.glsl"

float computeFoam(vec2 uv, sampler2D heightMap, float distance)
{
  float texSize = textureSize(heightMap, 0).x;
  uv = uv * water.size;
  vec2 texelPos = uv * texSize; // Convert to texel space

  vec2 i = floor(texelPos);
  vec2 f = fract(texelPos); // Fractional part for interpolation

  // Convert back to normalized UVs
  uv = i / texSize;
  float sampleSize = 10.0;
  vec2 uv00 = (i + vec2(-sampleSize, -sampleSize)) / texSize;
  vec2 uv10 = (i + vec2(sampleSize, -sampleSize)) / texSize;
  vec2 uv01 = (i + vec2(-sampleSize, sampleSize)) / texSize;
  vec2 uv11 = (i + vec2(sampleSize, sampleSize)) / texSize;

  // Sample height values
  float h = texture(heightMap, uv).r;
  float h00 = texture(heightMap, uv00).r;
  float h10 = texture(heightMap, uv10).r;
  float h01 = texture(heightMap, uv01).r;
  float h11 = texture(heightMap, uv11).r;

  // Compute local curvature (concavity) instead of just slope
  float foamFactor = (h00 + h01 + h10 + h11 - 4.0 * h);
  return clamp(-foamFactor * 10 * smoothstep(0.5, 1.0, h), 0, 1);
}

void waterMaterial(HitData hit, inout SampledMaterial mat, Ray ray)
{
  mat.normal = hit.normal;
  mat.uv = hit.textureCoordinates - floor(hit.textureCoordinates);
  mat.albedo = vec3(0.3, 0.4, 0.5);
  mat.albedo += computeFoam(hit.position.xz, noiseTexture, length(ray.origin - hit.position));
  mat.metallic = 0;
  mat.alpha = 1;
  mat.roughness = 1;
  mat.lod = 0;
  mat.emissivity = vec3(0);
  mat.specular = 0;
  mat.f0 = .0;
}
