float rayTriangleIntersect(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2)
{
  vec3 edge1 = vert1 - vert0;
  vec3 edge2 = vert2 - vert0;
  vec3 pvec = cross(dir, edge2);
  float det = dot(edge1, pvec);

  if (abs(det) < 1e-6) return FAR_PLANE;

  float invDet = 1.0 / det;
  vec3 tvec = orig - vert0;
  float u = dot(tvec, pvec) * invDet;

  if (u < 0.0 || u > 1.0) return FAR_PLANE;

  vec3 qvec = cross(tvec, edge1);
  float v = dot(dir, qvec) * invDet;

  if (v < 0.0 || u + v > 1.0) return FAR_PLANE;

  float t = dot(edge2, qvec) * invDet;

  return (t > 1e-6) ? t : FAR_PLANE;
}

float raySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sr)
{
  float a = dot(rd, rd);
  vec3 s0_r0 = r0 - s0;
  float b = 2.0 * dot(rd, s0_r0);
  float c = dot(s0_r0, s0_r0) - (sr * sr);
  float disriminant = b * b - 4.0 * a * c;
  if (disriminant < 0.0)
  {
    return FAR_PLANE;
  }
  return (-b - sqrt(disriminant)) / (2.0 * a);
}

float rayPlaneIntersect(vec3 r0, vec3 rd, vec3 n, float d) {
  float denom = dot(n, rd);

  if (abs(denom) <= 1e-4f)
    return FAR_PLANE;

  float t = -(dot(n, r0) + d) / denom;

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