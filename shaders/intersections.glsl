#include "uniforms.h"
#include "types.glsl"

float rayTriangleIntersect(Ray ray, vec3 vert0, vec3 vert1, vec3 vert2, out float u, out float v)
{
  vec3 edge1 = vert1 - vert0;
  vec3 edge2 = vert2 - vert0;
  vec3 pvec = cross(ray.direction, edge2);
  float det = dot(edge1, pvec);

  float invDet = 1.0 / det;
  vec3 tvec = ray.origin - vert0;

  u = dot(tvec, pvec) * invDet;
  vec3 qvec = cross(tvec, edge1);
  v = dot(ray.direction, qvec) * invDet;

  float t = dot(edge2, qvec) * invDet;

  bool valid = bool(abs(det) > 1e-6) && (u >= 0) && (v >= 0) && ((u + v) <= 1.0) && (t > 1e-6);
  return valid ? t : FAR_PLANE;
}

float raySphereIntersect(Ray ray, vec3 s0, float sr)
{
  float a = dot(ray.direction, ray.direction);
  vec3 s0_r0 = ray.origin - s0;
  float b = 2.0 * dot(ray.direction, s0_r0);
  float c = dot(s0_r0, s0_r0) - (sr * sr);
  float disriminant = b * b - 4.0 * a * c;
  if (disriminant < 0.0)
  {
    return FAR_PLANE;
  }
  return (-b - sqrt(disriminant)) / (2.0 * a);
}

float rayPlaneIntersect(Ray ray, vec3 n, float d) {
  float denom = dot(n, ray.direction);

  if (abs(denom) <= 1e-4f)
    return FAR_PLANE;

  float t = -(dot(n, ray.origin) + d) / denom;

  if (t <= 1e-4)
    return FAR_PLANE;

  return t;
}

float rayAABBIntersect(vec3 rayOrigin, vec3 rayDirInv, vec3 boxMin, vec3 boxMax)
{
  vec3 t0 = (boxMin - rayOrigin) * rayDirInv;
  vec3 t1 = (boxMax - rayOrigin) * rayDirInv;

  vec3 tmin = min(t0, t1);
  vec3 tmax = max(t0, t1);

  float tNear = max(max(tmin.x, tmin.y), tmin.z);
  float tFar = min(min(tmax.x, tmax.y), tmax.z);

  return (tFar > max(tNear, 0.0)) ? tNear : FAR_PLANE;
}

// Manual Bilinear Interpolation
float getHeight(vec2 texCoords, float size, sampler2D heightMap)
{
  float texSize = textureSize(heightMap, 0).x;
  vec2 uv = texCoords * size;
  vec2 texelPos = uv * texSize; // Convert to texel space

  vec2 i = floor(texelPos);
  vec2 f = fract(texelPos); // Fractional part for interpolation

  // Convert back to normalized UVs
  vec2 uv00 = (i + vec2(0.0, 0.0)) / texSize;
  vec2 uv10 = (i + vec2(1.0, 0.0)) / texSize;
  vec2 uv01 = (i + vec2(0.0, 1.0)) / texSize;
  vec2 uv11 = (i + vec2(1.0, 1.0)) / texSize;

  // Sample height values
  float h00 = texture(heightMap, uv00).r;
  float h10 = texture(heightMap, uv10).r;
  float h01 = texture(heightMap, uv01).r;
  float h11 = texture(heightMap, uv11).r;

  // Bilinear interpolation
  float h0 = mix(h00, h10, f.x);
  float h1 = mix(h01, h11, f.x);
  return u_interpolateNormals == 1 ? mix(h0, h1, f.y) : (h00 + h01 + h10 + h11) / 4.0;
}

float rayIntersectsHeightmap(Ray ray, sampler2D heightMap, float intensity, int samples, float size)
{
  // Don't intersect if ray origin below heightmap
  if (ray.origin.y < 0) return FAR_PLANE;

  // Intersect ray with bottom and top planes (y=0, y=intensity)
  float t_bottomPlane = -ray.origin.y / ray.direction.y;
  float t_upperPlane = (intensity - ray.origin.y) / ray.direction.y;

  // Ray is pointing away
  if (t_upperPlane < 0 && t_bottomPlane < 0) return FAR_PLANE;

  t_bottomPlane = max(t_bottomPlane, 0.0);
  t_upperPlane = max(t_upperPlane, 0.0);

  // Swap planes in ray is pointing upwards
  if (t_upperPlane > t_bottomPlane)
  {
    float tmp = t_upperPlane;
    t_upperPlane = t_bottomPlane;
    t_bottomPlane = tmp;
  }

  float dt = (t_bottomPlane - t_upperPlane) / float(samples);
  for (float t = t_upperPlane; t < t_bottomPlane; t += dt)
  {
    vec3 midPoint = ray.origin + t * ray.direction;
    float sampledHeight = getHeight(midPoint.xz, size, heightMap) * intensity;

    if (midPoint.y < sampledHeight) return t;
  }
  // No intersection
  return FAR_PLANE;
}

vec3 getHeightMapNormal(vec2 pos, sampler2D heightMap, float intensity, float size)
{
  vec2 uv = pos * size;
  float texelSize = 1.0 / textureSize(heightMap, 0).x; // Texel size in UV space

  vec2 uvL = uv - vec2(texelSize, 0.0);
  vec2 uvR = uv + vec2(texelSize, 0.0);
  vec2 uvD = uv - vec2(0.0, texelSize);
  vec2 uvU = uv + vec2(0.0, texelSize);

  float hL = getHeight(uvL, 1.0, heightMap) * intensity;
  float hR = getHeight(uvR, 1.0, heightMap) * intensity;
  float hD = getHeight(uvD, 1.0, heightMap) * intensity;
  float hU = getHeight(uvU, 1.0, heightMap) * intensity;

  vec3 normal = normalize(vec3((hL - hR), 2.0 * texelSize.x / size, (hD - hU)));
  return normal;
}
