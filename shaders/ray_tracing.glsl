#include "types.glsl"
#include "utils.glsl"
#include "intersections.glsl"
#include "bounding_volume.glsl"
#include "mandelbulb.glsl"
#include "pbr.glsl"

vec3 calculateRayDirection(vec3 cameraDirection, vec3 cameraRight, vec3 cameraUp, vec2 texCoords, float halfWidth, float halfHeight)
{
  float u = (2.0f * texCoords.x - 1.0f) * halfWidth;
  float v = (1.0f - 2.0f * (1.0 - texCoords.y)) * halfHeight;
  return normalize(cameraDirection.xyz + cameraRight.xyz * u + cameraUp.xyz * v);
}

HitData closestHit(Ray ray)
{
  HitData result;
  result.distance = FAR_PLANE;

  for (uint i = 0; i < u_spheresCount; i++)
  {
    float d = raySphereIntersect(ray, u_spheres[i].center.xyz, u_spheres[i].radius);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = normalize(ray.direction * d + ray.origin - u_spheres[i].center.xyz);
      result.materialIndex = u_spheres[i].materialIndex;
      float theta = atan(sqrt(result.normal.x * result.normal.x + result.normal.z * result.normal.z), result.normal.y);
      float phi = atan(result.normal.x, result.normal.z);
      result.textureCoordinates = vec2(phi / PI / 2.0f + 0.5, theta / PI);
      vec3 arbitrary = abs(result.normal.y) < (1 - 1e-5) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
      result.tangent = normalize(cross(arbitrary, result.normal));
      result.bitangent = cross(result.normal, result.tangent);
    }
  }

  for (uint i = 0; i < u_planesCount; i++)
  {
    float d = rayPlaneIntersect(ray, u_planes[i].normal.xyz, u_planes[i].distance);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = u_planes[i].normal.xyz;
      result.materialIndex = u_planes[i].materialIndex;
      result.textureCoordinates = (ray.direction * d + ray.origin).xz;
      result.tangent = vec3(1, 0, 0);
      result.bitangent = vec3(0, 0, 1);
      result.normal = vec3(0, 1, 0);
    }
  }

  if (u_water.isShown == 1)
  {
    float d = rayIntersectsHeightmap(ray, u_noiseTexture, u_water.amplitude, u_water.samples, u_water.size);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.position = ray.direction * (result.distance) + ray.origin;
      result.normal = getHeightMapNormal(result.position.xz, u_noiseTexture, u_water.amplitude, u_water.size);
      result.materialIndex = WATER_MATERIAL;
      result.textureCoordinates = vec2(0);
    }
  }

  for (int i = 0; i < u_bvhEntriesCount; i++)
  {
    HitData bvhHit = intersectBVH(ray, i);
    if (result.distance > bvhHit.distance)
    {
      result.distance = bvhHit.distance;
      result.normal = bvhHit.normal;
      result.materialIndex = bvhHit.materialIndex;
      result.textureCoordinates = bvhHit.textureCoordinates;
      result.tangent = bvhHit.tangent;
      result.bitangent = bvhHit.bitangent;
    }
  }

  if (u_mandelbulb.isShown == 1)
  {
    HitData rayMarchHit = mandelbulbIntersect(ray);
    if (rayMarchHit.distance < result.distance)
    {
      result.distance = rayMarchHit.distance;
      result.normal = rayMarchHit.normal;
      result.materialIndex = rayMarchHit.materialIndex;
      result.textureCoordinates = rayMarchHit.textureCoordinates;
      result.tangent = rayMarchHit.tangent;
      result.bitangent = rayMarchHit.bitangent;
    }
  }

  result.position = ray.direction * (result.distance) + ray.origin;
  return result;
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
