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
  int materialIndex;
  float distance;
};

// XYZ - are vertex indices, W is material index
ivec4 getTriangle(int index)
{
  return texelFetch(trianglesTexture, ivec3(index % maxTextureSize, index / maxTextureSize, 0), 0);
}

Vertex getVertex(int index)
{
  int absoluteIndex = index * VERTEX_SIZE;
  Vertex v;

  int positionIndex = absoluteIndex + VERTEX_POSITION;
  int normalIndex = absoluteIndex + VERTEX_NORMAL;
  int tangentIndex = absoluteIndex + VERTEX_TANGENT;
  int bitangentIndex = absoluteIndex + VERTEX_BITANGENT;

  v.position = texelFetch(verticesTexture, ivec3(positionIndex % maxTextureSize, positionIndex / maxTextureSize, 0), 0);
  v.normal = texelFetch(verticesTexture, ivec3(normalIndex % maxTextureSize, normalIndex / maxTextureSize, 0), 0);
  v.tangent = texelFetch(verticesTexture, ivec3(tangentIndex % maxTextureSize, tangentIndex / maxTextureSize, 0), 0);
  v.bitangent = texelFetch(verticesTexture, ivec3(bitangentIndex % maxTextureSize, bitangentIndex / maxTextureSize, 0), 0);

  return v;
}

BoundingVolume getBoundingVolume(int index)
{
  int absoluteIndex = index * BOUND_VOLUME_SIZE;
  BoundingVolume v;

  int minLeftIndex = absoluteIndex + BOUND_VOLUME_MIN_LEFT;
  int maxRightIndex = absoluteIndex + BOUND_VOLUME_MAX_RIGHT;
  int startCount = absoluteIndex + BOUND_VOLUME_COUNT_START;

  vec4 minLeft = texelFetch(boundingVolumesTexture, ivec3(minLeftIndex % maxTextureSize, minLeftIndex / maxTextureSize, 0), 0);
  vec4 maxRight = texelFetch(boundingVolumesTexture, ivec3(maxRightIndex % maxTextureSize, maxRightIndex / maxTextureSize, 0), 0);
  v.triangleCountTrianglesStart = texelFetch(boundingVolumesTexture, ivec3(startCount % maxTextureSize, startCount / maxTextureSize, 0), 0);

  v.min = minLeft.xyz;
  v.nodeLeft = minLeft.w;
  v.max = maxRight.xyz;
  v.nodeRight = maxRight.w;

  return v;
}

BVHHit intersectBVH(vec3 orig, vec3 dir)
{
  vec3 invDir = 1.0 / dir;
  BVHHit hit;
  hit.distance = FAR_PLANE;
  hit.triangleIndex = -1;

  float stack[STACK_SIZE];
  int stackPtr = 0;
  stack[stackPtr++] = 0;

  while (stackPtr > 0)
  {
    int nodeIndex = int(stack[--stackPtr]);

    BoundingVolume volume = getBoundingVolume(nodeIndex);

    if (rayAABBIntersect(orig, invDir, volume.min, volume.max) == FAR_PLANE)
      continue;

    if (volume.triangleCountTrianglesStart.x > 0.0)
    {
      int triStart = int(volume.triangleCountTrianglesStart.y);
      int triCount = int(volume.triangleCountTrianglesStart.x);

      for (int i = 0; i < triCount; i++)
      {
        ivec4 triangle = getTriangle(triStart + i);

        vec3 v0 = getVertex(triangle.x).position.xyz;
        vec3 v1 = getVertex(triangle.y).position.xyz;
        vec3 v2 = getVertex(triangle.z).position.xyz;

        float t = rayTriangleIntersect(orig, dir, v0, v1, v2);
        if (t > 0.0 && t < hit.distance)
        {
          hit.distance = t;
          hit.triangleIndex = triStart + i;
          hit.materialIndex = 1;
        }
      }
    }
    else
    {
      if (volume.nodeLeft != -1.0)
        stack[stackPtr++] = volume.nodeLeft;
      if (volume.nodeRight != -1.0)
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
    ivec4 triangle = getTriangle(bvhHit.triangleIndex);
    Vertex a = getVertex(triangle[0]);
    Vertex b = getVertex(triangle[1]);
    Vertex c = getVertex(triangle[2]);
    result.distance = bvhHit.distance;
    result.normal = normalize(a.normal.xyz + c.normal.xyz + b.normal.xyz);
    result.materialIndex = bvhHit.materialIndex;
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
    vec3 sampleColor = vec3(1);

    for (uint i = 0; i < bounces; i++)
    {
      ClosestHit hit = closestHit(org, dir);
      if (hit.distance == FAR_PLANE)
      {
        float theta = atan(sqrt(dir.x * dir.x + dir.z * dir.z), dir.y);
        float phi = atan(dir.x, dir.z);
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

        org = hit.position + normal * bias;
        normal = normalize(normal + rand3((dir + org) * (sampleCounter + 1.0)) * roughness);
        dir = reflect(dir, normal);
      }
    }

    resultColor += sampleColor;
  }
  return resultColor / samples;
}
