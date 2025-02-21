float rayTriangleIntersect(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2)
{
  vec3 edge1 = vert1 - vert0;
  vec3 edge2 = vert2 - vert0;
  vec3 baryPosition;

  vec3 p = cross(dir, edge2);
  float det = dot(edge1, p);
  vec3 Perpendicular = vec3(0);

  if (det > 0)
  {
    vec3 dist = orig - vert0;

    baryPosition.x = dot(dist, p);
    if (baryPosition.x < 0 || baryPosition.x > det)
      return FAR_PLANE;

    Perpendicular = cross(dist, edge1);

    baryPosition.y = dot(dir, Perpendicular);
    if((baryPosition.y < 0) || ((baryPosition.x + baryPosition.y) > det))
      return FAR_PLANE;
  }
  else if(det < 0)
  {
    vec3 dist = orig - vert0;

    baryPosition.x = dot(dist, p);
    if((baryPosition.x > 0) || (baryPosition.x < det))
      return FAR_PLANE;

    Perpendicular = cross(dist, edge1);

    baryPosition.y = dot(dir, Perpendicular);
    if((baryPosition.y > 0) || (baryPosition.x + baryPosition.y < det))
      return FAR_PLANE;
  }
  else
    return FAR_PLANE;

  float inv_det = 1 / det;

  float distance = dot(edge2, Perpendicular) * inv_det;
  baryPosition *= inv_det;

  return distance;
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