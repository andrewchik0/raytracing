#include "uniforms.h"

// DO NOT TOUCH IN CLion it will crash GLGL plugin
#define STACK_SIZE 128

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

BVHHit intersectBVH(Ray ray)
{
  vec3 invDir = 1.0 / ray.direction;
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

    if (rayAABBIntersect(ray.origin, invDir, volume.min, volume.max) == FAR_PLANE)
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

        float t = rayTriangleIntersect(ray, v0, v1, v2);
        if (t < hit.distance)
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
