vec3 skyColor = vec3(0.6f, 0.7f, 1.0f);

float random(vec2 st)
{
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}
vec3 rand3(vec3 seed)
{
  return vec3(
    random(vec2(time, seed.x)) - 0.5,
    random(vec2(time, seed.y)) - 0.5,
    random(vec2(time, seed.z)) - 0.5
  );
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

vec3 calculateRayDirection()
{
  float u = (2.0f * passTexCoord.x - 1.0f) * halfWidth;
  float v = (1.0f - 2.0f * (1.0 - passTexCoord.y)) * halfHeight;
  return normalize(cameraDirection + cameraRight * u + cameraUp * v);
}

struct ClosestHit
{
  vec3 normal;
  vec3 position;
  float distance;
  vec3 albedo;
};

ClosestHit closestHit(vec3 rayOrigin, vec3 rayDirection)
{
  ClosestHit result;
  result.distance = FAR_PLANE;

  for (uint i = 0; i < spheresCount; i++)
  {
    float d = raySphereIntersect(rayOrigin, rayDirection, spheres[i].center, spheres[i].radius);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = normalize(rayDirection * d + rayOrigin - spheres[i].center);
      result.albedo = spheres[i].albedo.rgb;
    }
  }

  for (uint i = 0; i < planesCount; i++)
  {
    float d = rayPlaneIntersect(rayOrigin, rayDirection, planes[i].normal, planes[i].distance);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = planes[i].normal;
      result.albedo = planes[i].albedo.rgb;
    }
  }
  result.position = rayDirection * result.distance + rayOrigin + result.normal * 0.0001;
  return result;
}

vec3 castRayFinal(vec3 rayOrigin, vec3 rayDirection)
{
  ClosestHit hit = closestHit(rayOrigin, rayDirection);
  if (hit.distance == FAR_PLANE)
  {
    return skyColor;
  }
  return max(vec3(0.1), dot(hit.normal, lightDirection) * hit.albedo);
}

vec3 castRay(vec3 rayOrigin, vec3 rayDirection)
{
  vec3 org = rayOrigin, dir = rayDirection;
  uint bounces = 5;
  float multiplier = 1.0f;
  vec3 resultColor;

  for (uint i = 0; i < bounces; i++)
  {
    ClosestHit hit = closestHit(org, dir);
    if (hit.distance == FAR_PLANE)
    {
      resultColor = skyColor * multiplier;
    }
    else
    {
      resultColor = max(vec3(0), dot(hit.normal, lightDirection) * hit.albedo) * multiplier;
      org = hit.position;
      dir = reflect(dir, hit.normal * rand3(dir) * 0.5);
    }
  }
  return resultColor;
}
