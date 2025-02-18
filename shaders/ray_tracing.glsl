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

vec3 castRay(vec3 rayOrigin, vec3 rayDirection)
{
  uint objectType = 0;
  uint objectIndex;
  float closest = FAR_PLANE;
  vec3 normal;
  vec3 hitAlbedo;

  vec3 normalizedLight = normalize(lightDirection);

  for (uint i = 0; i < spheresCount; i++)
  {
    float d = raySphereIntersect(rayOrigin, rayDirection, spheres[i].center, spheres[i].radius);
    if (d > 0 && closest > d)
    {
      closest = d;
      normal = normalize(rayDirection * d + rayOrigin - spheres[i].center);
      hitAlbedo = spheres[i].albedo.rgb;

      if (castShadow(rayOrigin + d * rayDirection, normalizedLight) != FAR_PLANE)
      {
        hitAlbedo *= 0.5;
      }
    }
  }

  for (uint i = 0; i < planesCount; i++)
  {
    float d = rayPlaneIntersect(rayOrigin, rayDirection, planes[i].normal, planes[i].distance);
    if (d > 0 && closest > d)
    {
      closest = d;
      normal = planes[i].normal;
      hitAlbedo = planes[i].albedo.rgb;

      if (castShadow(rayOrigin + d * rayDirection, normalizedLight) != FAR_PLANE)
      {
        hitAlbedo *= 0.5;
      }
    }
  }

  if (closest == FAR_PLANE)
  {
    return vec3(0.5, 0.7, 1.0);
  }

  return max(vec3(0.1), dot(normal, normalizedLight) * hitAlbedo);
}