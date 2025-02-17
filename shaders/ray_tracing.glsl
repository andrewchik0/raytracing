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

vec3 calculateRayDirection()
{
  float u = (2.0f * passTexCoord.x - 1.0f) * halfWidth;
  float v = (1.0f - 2.0f * (1.0 - passTexCoord.y)) * halfHeight;
  return normalize(cameraDirection + cameraRight * u + cameraUp * v);
}

struct CastRay
{
  float distance;
  vec3 normal;
  vec3 color;
};

float castShadow(vec3 rayOrigin, vec3 rayDirection)
{
  float closest = FAR_PLANE;

  for (uint i = 0; i < spheresCount; i++)
  {
    float d = raySphereIntersect(rayOrigin, rayDirection, spheres[i].center, spheres[i].radius);
    if (d > 0 && closest > d)
    {
      closest = d;
    }
  }

  for (uint i = 0; i < planesCount; i++)
  {
    float d = rayPlaneIntersect(rayOrigin, rayDirection, planes[i].normal, planes[i].distance);
    if (d > 0 && closest > d)
    {
      closest = d;
    }
  }
  return closest;
}

CastRay castRay(vec3 rayOrigin, vec3 rayDirection)
{
  uint objectType = 0;
  uint objectIndex;

  CastRay ray;

  ray.distance = FAR_PLANE;

  vec3 normalizedLight = normalize(lightDirection);

  for (uint i = 0; i < spheresCount; i++)
  {
    float d = raySphereIntersect(rayOrigin, rayDirection, spheres[i].center, spheres[i].radius);
    if (d > 0 && ray.distance > d)
    {
      ray.distance = d;
      ray.normal = normalize(rayDirection * d + rayOrigin);
      ray.color = spheres[i].albedo.rgb;

      if (castShadow(rayOrigin + d * rayDirection, normalizedLight) != FAR_PLANE)
      {
        ray.color *= 0.5;
      }
    }
  }

  for (uint i = 0; i < planesCount; i++)
  {
    float d = rayPlaneIntersect(rayOrigin, rayDirection, planes[i].normal, planes[i].distance);
    if (d > 0 && ray.distance > d)
    {
      ray.distance = d;
      ray.normal = planes[i].normal;
      ray.color = planes[i].albedo.rgb;

      if (castShadow(rayOrigin + d * rayDirection, normalizedLight) != FAR_PLANE)
      {
        ray.color *= 0.5;
      }
    }
  }

  return ray;
}