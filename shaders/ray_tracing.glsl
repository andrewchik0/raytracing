float raySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sr)
{
  // - r0: ray origin
  // - rd: normalized ray direction
  // - s0: sphere center
  // - sr: sphere radius
  // - Returns distance from r0 to first intersecion with sphere,
  //   or -1.0 if no intersection.
  float a = dot(rd, rd);
  vec3 s0_r0 = r0 - s0;
  float b = 2.0 * dot(rd, s0_r0);
  float c = dot(s0_r0, s0_r0) - (sr * sr);
  float disriminant = b * b - 4.0 * a * c;
  if (disriminant < 0.0)
  {
    return -1.0;
  }
  return (-b - sqrt(disriminant)) / (2.0 * a);
}

float rayPlaneIntersect(vec3 r0, vec3 rd, vec3 n, float d) {
  float denom = dot(n, rd);

  if (abs(denom) <= 1e-4f)
    return -1.0;

  float t = -(dot(n, r0) + d) / denom;

  if (t <= 1e-4)
    return -1.0;

  return t;
}

vec3 calculateRayDirection()
{
  float u = (2.0f * passTexCoord.x - 1.0f) * halfWidth;
  float v = (1.0f - 2.0f * (1.0 - passTexCoord.y)) * halfHeight;
  return normalize(cameraDirection + cameraRight * u + cameraUp * v);
}