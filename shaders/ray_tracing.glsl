#include "types.glsl"
#include "utils.glsl"
#include "intersections.glsl"
#include "bounding_volume.glsl"
#include "mandelbulb.glsl"
#include "hit.glsl"
#include "pbr.glsl"

vec3 calculateRayDirection(vec3 cameraDirection, vec3 cameraRight, vec3 cameraUp, vec2 texCoords, float halfWidth, float halfHeight)
{
  float u = (2.0f * texCoords.x - 1.0f) * halfWidth;
  float v = (1.0f - 2.0f * (1.0 - texCoords.y)) * halfHeight;
  return normalize(cameraDirection.xyz + cameraRight.xyz * u + cameraUp.xyz * v);
}

vec3 castRay(Ray inputRay)
{
  vec3 resultColor = vec3(0);

  uint sampleCounter;
  for (sampleCounter = 0; sampleCounter < u_samples; sampleCounter++)
  {
    Ray ray;
    ray.origin = inputRay.origin;
    ray.direction = inputRay.direction;
    vec3 sampleColor = vec3(1);

    for (uint i = 0; i < u_bounces; i++)
    {
      HitData hit = closestHit(ray);
      if (hit.distance == FAR_PLANE)
      {
        float theta = atan(sqrt(ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z), ray.direction.y);
        float phi = atan(ray.direction.x, ray.direction.z);
        vec3 skyColor = min(texture(u_sky, vec2(phi / PI / 2.0 + 0.5, theta / PI)).rgb, vec3(42.0));
        sampleColor *= skyColor;
        break;
      }
      else
      {
        if (!pbr(hit, sampleCounter, i, sampleColor, ray))
          break;
      }
    }

    resultColor += sampleColor;
  }
  return resultColor / u_samples;
}
