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

  float invDet = 1.0 / det;
  vec3 tvec = ray.origin - vert0;

  u = dot(tvec, pvec) * invDet;
  vec3 qvec = cross(tvec, edge1);
  v = dot(ray.direction, qvec) * invDet;

  float t = dot(edge2, qvec) * invDet;

  bool valid = (abs(det) > 1e-6) && (u >= 0.0) && (v >= 0.0) && ((u + v) <= 1.0) && (t > 1e-6);
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