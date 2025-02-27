#include "utils.glsl"
#include "intersections.glsl"
#include "bounding_volume.glsl"

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

  for (uint i = 0; i < spheresCount; i++)
  {
    float d = raySphereIntersect(ray, spheres[i].center.xyz, spheres[i].radius);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = normalize(ray.direction * d + ray.origin - spheres[i].center.xyz);
      result.materialIndex = spheres[i].materialIndex;
      float theta = atan(sqrt(result.normal.x * result.normal.x + result.normal.z * result.normal.z), result.normal.y);
      float phi = atan(result.normal.x, result.normal.z);
      result.textureCoordinates = vec2(phi / PI / 2.0f + 0.5, theta / PI);
      vec3 arbitrary = abs(result.normal.y) < (1 - 1e-5) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
      result.tangent = normalize(cross(arbitrary, result.normal));
      result.bitangent = cross(result.normal, result.tangent);
    }
  }

  for (uint i = 0; i < planesCount; i++)
  {
    float d = rayPlaneIntersect(ray, planes[i].normal.xyz, planes[i].distance);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = planes[i].normal.xyz;
      result.materialIndex = planes[i].materialIndex;
      result.textureCoordinates = (ray.direction * d + ray.origin).xz;
      result.tangent = vec3(1, 0, 0);
      result.bitangent = vec3(0, 0, 1);
      result.normal = vec3(0, 1, 0);
    }
  }

  HitData bvhHit = intersectBVH(ray);
  if (result.distance > bvhHit.distance)
  {
    result.distance = bvhHit.distance;
    result.normal = bvhHit.normal;
    result.materialIndex = bvhHit.materialIndex;
    result.textureCoordinates = bvhHit.textureCoordinates;
    result.tangent = bvhHit.tangent;
    result.bitangent = bvhHit.bitangent;
  }

  result.position = ray.direction * (result.distance) + ray.origin;
  return result;
}

vec3 castRay(Ray inputRay)
{
  float bias = 1e-5;
  vec3 resultColor = vec3(0);

  for (uint sampleCounter = 0; sampleCounter < samples; sampleCounter++)
  {
    Ray ray;
    ray.origin = inputRay.origin;
    ray.direction = inputRay.direction;
    vec3 sampleColor = vec3(1);

    for (uint i = 0; i < bounces; i++)
    {
      HitData hit = closestHit(ray);
      if (hit.distance == FAR_PLANE)
      {
        float theta = atan(sqrt(ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z), ray.direction.y);
        float phi = atan(ray.direction.x, ray.direction.z);
        vec3 skyColor = min(texture(sky, vec2(phi / PI / 2.0 + 0.5, theta / PI)).rgb, vec3(42.0));
        sampleColor *= skyColor;
        break;
      }
      else
      {
        vec2 texCoords = hit.textureCoordinates * materials[hit.materialIndex].textureCoordinatesMultiplier;

        vec3 albedo =
          float(materials[hit.materialIndex].textureIndex != -1) *
          texture(texArray, vec3(texCoords, materials[hit.materialIndex].textureIndex)).rgb +
          float(materials[hit.materialIndex].textureIndex == -1) * materials[hit.materialIndex].albedo;

        float roughness =
          float(materials[hit.materialIndex].metallicTextureIndex != -1) *
          (1.0 - texture(texArray, vec3(texCoords, materials[hit.materialIndex].metallicTextureIndex)).r) +
          float(materials[hit.materialIndex].metallicTextureIndex == -1) * materials[hit.materialIndex].roughness;

        vec3 normal;
        mat3 TBN = mat3(hit.tangent, hit.bitangent, hit.normal);
        normal =
          float((materials[hit.materialIndex].normalTextureIndex != -1)) *
          TBN * (texture(texArray, vec3(texCoords, materials[hit.materialIndex].normalTextureIndex)).rgb * 2.0 - 1) +
          float((materials[hit.materialIndex].normalTextureIndex == -1)) * hit.normal;

        vec3 e = materials[hit.materialIndex].emissivity;
        if (e.x + e.y + e.z != 0)
        {
          sampleColor = e;
          break;
        }
        sampleColor = sampleColor * albedo + e;

        if (renderMode != 1)
        {
          sampleColor = albedo * max(dot(normalize((vec3(1.0))), normal), 0.1);
          break;
        }

        ray.origin = hit.position + normal * bias;
        normal = normalize(normal + rand3((ray.direction + ray.origin) * (sampleCounter + 1.0)) * roughness);
        ray.direction = reflect(ray.direction, normal);
      }
    }

    resultColor += sampleColor;
  }
  return resultColor / samples;
}
