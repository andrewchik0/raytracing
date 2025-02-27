#include "uniforms.h"

struct Ray
{
  vec3 origin;
  vec3 direction;
};

float rayTriangleIntersect(Ray ray, vec3 vert0, vec3 vert1, vec3 vert2, out float u, out float v)
{
  vec3 edge1 = vert1 - vert0;
  vec3 edge2 = vert2 - vert0;
  vec3 pvec = cross(ray.direction, edge2);
  float det = dot(edge1, pvec);

  if (abs(det) < 1e-6) return FAR_PLANE;

  float invDet = 1.0 / det;
  vec3 tvec = ray.origin - vert0;
  u = dot(tvec, pvec) * invDet;

  if (u < 0.0 || u > 1.0) return FAR_PLANE;

  vec3 qvec = cross(tvec, edge1);
  v = dot(ray.direction, qvec) * invDet;

  if (v < 0.0 || (u + v) > 1.0) return FAR_PLANE;

  float t = dot(edge2, qvec) * invDet;

  return (t > 1e-6) ? t : FAR_PLANE;
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
  vec3 tbot = rayDirInv * (boxMin - rayOrigin);
  vec3 ttop = rayDirInv * (boxMax - rayOrigin);
  vec3 tmin = min(ttop, tbot);
  vec3 tmax = max(ttop, tbot);
  vec2 t = max(tmin.xx, tmin.yz);
  float t0 = max(t.x, t.y);
  t = min(tmax.xx, tmax.yz);
  float t1 = min(t.x, t.y);
  return t1 > max(t0, 0.0) ? t0 : FAR_PLANE;
};