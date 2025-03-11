#include "uniforms.h"
#include "types.glsl"

layout(std430, binding = BVH_ENTRIES_BINDING) buffer BVHEntries
{
  int entriesCount;
  int entries[];
};

layout(std430, binding = BVH_BINDING) buffer BVHBuffer
{
  BoundingVolume bvhNodes[];
};

layout(std430, binding = VERTICES_BINDING) buffer VertexBuffer
{
  Vertex vertices[];
};

// DO NOT TOUCH IN CLion it will crash GLSL plugin
#define STACK_SIZE 32

HitData intersectBVH(Ray ray, int modelIndex)
{
  vec3 invDir = 1.0 / ray.direction;
  HitData hit;
  hit.distance = FAR_PLANE;
  ivec4 foundTriangle;
  vec2 foundUV;

  float stack[STACK_SIZE];
  int stackPtr = 0;
  stack[stackPtr++] = 0;

  while (stackPtr > 0)
  {
    int nodeIndex = int(stack[--stackPtr]);

    BoundingVolume volume = bvhNodes[nodeIndex + entries[modelIndex]];

    if (rayAABBIntersect(ray.origin, invDir, volume.min, volume.max) == FAR_PLANE)
      continue;

    if (volume.triangle.x != volume.triangle.y)
    {
      ivec4 triangle = ivec4(volume.triangle);

      Vertex v0 = vertices[triangle.x];
      Vertex v1 = vertices[triangle.y];
      Vertex v2 = vertices[triangle.z];

      float u, v;
      float t = rayTriangleIntersect(ray, v0.position.xyz, v1.position.xyz, v2.position.xyz, u, v);
      if (t < hit.distance)
      {
        foundTriangle = triangle;
        hit.distance = t;
        foundUV = vec2(u, v);
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

    Vertex v0 = vertices[foundTriangle.x];
    Vertex v1 = vertices[foundTriangle.y];
    Vertex v2 = vertices[foundTriangle.z];

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
    hit.textureCoordinates.y = 1.0 - hit.textureCoordinates.y;
    hit.materialIndex = foundTriangle.w;
  }

  return hit;
}
