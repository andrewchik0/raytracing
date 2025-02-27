#include "uniforms.h"

// DO NOT TOUCH IN CLion it will crash GLGL plugin
#define STACK_SIZE 128

struct HitData
{
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
  vec2 textureCoordinates;
  uint materialIndex;
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

HitData intersectBVH(Ray ray)
{
  vec3 invDir = 1.0 / ray.direction;
  HitData hit;
  hit.distance = FAR_PLANE;
  int triangleIndex = -1;
  vec2 foundUV;

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

        Vertex v0 = getVertex(triangle.x);
        Vertex v1 = getVertex(triangle.y);
        Vertex v2 = getVertex(triangle.z);

        float u, v;
        float t = rayTriangleIntersect(ray, v0.position.xyz, v1.position.xyz, v2.position.xyz, u, v);
        if (t < hit.distance)
        {
          triangleIndex = triStart + i;
          hit.distance = t;
          foundUV = vec2(u, v);
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

  if (hit.distance != FAR_PLANE)
  {
    float u = foundUV.x;
    float v = foundUV.y;
    float w = 1.0 - u - v;
    ivec4 triangle = getTriangle(triangleIndex);

    Vertex v0 = getVertex(triangle.x);
    Vertex v1 = getVertex(triangle.y);
    Vertex v2 = getVertex(triangle.z);

    if (interpolateNormals == 1)
    {
      hit.normal = normalize(w * v0.normal.xyz + u * v1.normal.xyz + v * v2.normal.xyz);
      hit.tangent = normalize(w * v0.tangent.xyz + u * v1.tangent.xyz + v * v2.tangent.xyz);
      hit.bitangent = normalize(w * v0.bitangent.xyz + u * v1.bitangent.xyz + v * v2.bitangent.xyz);
    }
    else
    {
      hit.normal = normalize(v0.normal.xyz + v1.normal.xyz + v2.normal.xyz);
      hit.tangent = normalize(v0.tangent.xyz + v1.tangent.xyz + v2.tangent.xyz);
      hit.bitangent = normalize(v0.bitangent.xyz + v1.bitangent.xyz + v2.bitangent.xyz);
    }
    vec2 texCoords0 = vec2(v0.tangent.w, v0.bitangent.w);
    vec2 texCoords1 = vec2(v1.tangent.w, v1.bitangent.w);
    vec2 texCoords2 = vec2(v2.tangent.w, v2.bitangent.w);
    hit.textureCoordinates = w * texCoords0 + u * texCoords1 + v * texCoords2;
    hit.materialIndex = triangle.w;
  }

  return hit;
}
