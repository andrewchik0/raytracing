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

float rayIntersectsHeightmap(Ray ray, sampler2D heightMap, float intensity, int samples, float size)
{
  // intersect with bounding box of heightmap
  vec3 min = vec3(-FAR_PLANE, 0, -FAR_PLANE);
  vec3 max = vec3(FAR_PLANE, intensity, FAR_PLANE);
  float t_aabb = rayAABBIntersect(ray.origin, 1.0 / ray.direction, min, max);
  if (t_aabb == FAR_PLANE) return FAR_PLANE; // Ray is pointing away

  // Intersect ray with flat plane (y=0)
  float t_plane = -ray.origin.y / ray.direction.y;

  // Compute the height at (x, z)
  vec3 hit = ray.origin + t_plane * ray.direction;
  float h = texture(heightMap, hit.xz * size).r * intensity;

  // Binary search to refine intersection
  float t_min = 0, t_max = t_plane;
  for (int i = 0; i < samples; i++) // Increase iterations for better accuracy
  {
    float t_mid = 0.5f * (t_min + t_max);
    vec3 mid_point = ray.origin + t_mid * ray.direction;
    float h_mid = texture(heightMap, mid_point.xz * size).r * intensity;

    if (mid_point.y > h_mid)
      t_min = t_mid;
    else
      t_max = t_mid;
  }

  return t_max;
}

vec3 getHeightMapNormal(vec2 pos, sampler2D heightMap, float intensity, float size)
{
  float dx = 1.0 / float(textureSize(heightMap, 0).x - 1);
  float dy = 1.0 / float(textureSize(heightMap, 0).y - 1);

  vec2 uvL = vec2(pos.x - dx * 100, pos.y);
  vec2 uvR = vec2(pos.x + dx * 100, pos.y);
  vec2 uvD = vec2(pos.x, pos.y - dy);
  vec2 uvU = vec2(pos.x, pos.y + dy);

  uvL = uvL - floor(uvL);
  uvR = uvR - floor(uvR);
  uvD = uvD - floor(uvD);
  uvU = uvU - floor(uvU);

  float hL = texture(heightMap, uvL * size).r * intensity;
  float hR = texture(heightMap, uvR * size).r * intensity;
  float hD = texture(heightMap, uvD * size).r * intensity;
  float hU = texture(heightMap, uvU * size).r * intensity;

  vec3 normal = normalize(vec3((hL - hR) * intensity, 2.0, (hD - hU) * intensity));
  return normal;
}