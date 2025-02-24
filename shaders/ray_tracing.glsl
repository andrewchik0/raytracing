#include "utils.glsl"
#include "intersections.glsl"

#define STACK_SIZE 128

vec3 calculateRayDirection()
{
  float u = (2.0f * passTexCoord.x - 1.0f) * halfWidth;
  float v = (1.0f - 2.0f * (1.0 - passTexCoord.y)) * halfHeight;
  return normalize(cameraDirection + cameraRight * u + cameraUp * v);
}

struct BVHHit
{
  int triangleIndex;
  float distance;
};

BVHHit intersectBVH(vec3 orig, vec3 dir)
{
  vec3 invDir = 1.0 / dir;
  BVHHit hit;
  hit.distance = FAR_PLANE;
  hit.triangleIndex = -1;

  int stack[STACK_SIZE];
  int stackPtr = 0;
  stack[stackPtr++] = 0;

  while (stackPtr > 0)
  {
    int nodeIndex = stack[--stackPtr];

    BoundingVolume volume = volumes[nodeIndex];

    if (rayAABBIntersect(orig, invDir, volume.min, volume.max) == FAR_PLANE)
      continue;

    if (volume.triangleCountTrianglesStart.x > 0)
    {
      int triStart = volume.triangleCountTrianglesStart.y;
      int triCount = volume.triangleCountTrianglesStart.x;

      for (int i = 0; i < triCount; i++)
      {
        TriangleObject tri = triangles[triStart + i];

        vec3 v0 = vertices[tri.indices.x].position.xyz;
        vec3 v1 = vertices[tri.indices.y].position.xyz;
        vec3 v2 = vertices[tri.indices.z].position.xyz;

        float t = rayTriangleIntersect(orig, dir, v0, v1, v2);
        if (t > 0.0 && t < hit.distance)
        {
          hit.distance = t;
          hit.triangleIndex = triStart + i;
        }
      }
    }
    else
    {
      if (volume.nodeLeft != -1)
        stack[stackPtr++] = volume.nodeLeft;
      if (volume.nodeRight != -1)
        stack[stackPtr++] = volume.nodeRight;
    }

    if (stackPtr >= STACK_SIZE) break;
  }

  return hit;
}

struct ClosestHit
{
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
  vec3 position;
  vec2 textureCoordinates;
  float distance;
  uint materialIndex;
};

ClosestHit closestHit(vec3 rayOrigin, vec3 rayDirection)
{
  ClosestHit result;
  result.distance = FAR_PLANE;

  for (uint i = 0; i < spheresCount; i++)
  {
    float d = raySphereIntersect(rayOrigin, rayDirection, spheres[i].center.xyz, spheres[i].radius);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = normalize(rayDirection * d + rayOrigin - spheres[i].center.xyz);
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
    float d = rayPlaneIntersect(rayOrigin, rayDirection, planes[i].normal.xyz, planes[i].distance);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = planes[i].normal.xyz;
      result.materialIndex = planes[i].materialIndex;
      result.textureCoordinates = (rayDirection * d + rayOrigin).xz;
      result.tangent = vec3(1, 0, 0);
      result.bitangent = vec3(0, 0, 1);
      result.normal = vec3(0, 1, 0);
    }
  }

  BVHHit bvhHit = intersectBVH(rayOrigin, rayDirection);
  if (result.distance > bvhHit.distance)
  {
    Vertex a = vertices[triangles[bvhHit.triangleIndex].indices[0]];
    Vertex b = vertices[triangles[bvhHit.triangleIndex].indices[1]];
    Vertex c = vertices[triangles[bvhHit.triangleIndex].indices[2]];
    result.distance = bvhHit.distance;
    result.normal = normalize(a.normal.xyz + c.normal.xyz + b.normal.xyz);
    result.materialIndex = 1;
    result.textureCoordinates = a.position.xy;
    result.tangent = a.tangent.xyz;
    result.bitangent = a.bitangent.xyz;
  }

  result.position = rayDirection * (result.distance) + rayOrigin;
  return result;
}

vec3 castRay(vec3 rayOrigin, vec3 rayDirection)
{
  float bias = 1e-6;
  vec3 resultColor = vec3(0);

  for (uint sampleCounter = 0; sampleCounter < samples; sampleCounter++)
  {
    vec3 org = rayOrigin, dir = rayDirection;
    vec3 sampleColor = vec3(0);

    for (uint i = 0; i < bounces; i++)
    {
      ClosestHit hit = closestHit(org, dir);
      if (hit.distance == FAR_PLANE)
      {
        float theta = atan(sqrt(dir.x * dir.x + dir.z * dir.z), dir.y);
        float phi = atan(dir.x, dir.z);
        vec3 skyColor = min(texture(sky, vec3(phi / PI / 2.0 + 0.5, theta / PI, 0)).rgb, vec3(42.0));
        if (sampleColor == vec3(0))
          sampleColor = skyColor;
        else
          sampleColor = sampleColor * skyColor;
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
        if (sampleColor == vec3(0))
          sampleColor = albedo + e;
        else
          sampleColor = sampleColor * albedo + e;

        org = hit.position + normal * bias;
        normal = normalize(normal + rand3((dir + org) * (sampleCounter + 1.0)) * roughness);
        dir = reflect(dir, normal);
      }
    }

    resultColor += sampleColor;
  }
  return resultColor / samples;
}
