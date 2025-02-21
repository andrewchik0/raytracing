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

const float PI = 3.141592653589;
float atan2(float y, float x)
{
  bool s = (abs(x) > abs(y));
  return mix(PI / 2.0 - atan(x, y), atan(y, x), s);
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
  vec3 tangent;
  vec3 bitangent;
  vec3 position;
  vec2 textureCoordinates;
  float distance;
  uint materialIndex;
};

ClosestHit closestHit(vec3 rayOrigin, vec3 rayDirection)
{
  ClosestHit result;
  result.distance = FAR_PLANE;

  for (uint i = 0; i < spheresCount; i++)
  {
    float d = raySphereIntersect(rayOrigin, rayDirection, spheres[i].center.xyz, spheres[i].radius);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = normalize(rayDirection * d + rayOrigin - spheres[i].center.xyz);
      result.materialIndex = spheres[i].materialIndex;
      float theta = atan(sqrt(result.normal.x * result.normal.x + result.normal.z * result.normal.z), result.normal.y);
      float phi = atan(result.normal.x, result.normal.z);
      result.textureCoordinates = vec2(phi / PI / 2.0f + 0.5, theta / PI);
      result.tangent = normalize(vec3(-cos(theta) * sin(phi), 0, cos(theta) * cos(phi)));
      result.bitangent = cross(result.normal, result.tangent);
    }
  }

  for (uint i = 0; i < planesCount; i++)
  {
    float d = rayPlaneIntersect(rayOrigin, rayDirection, planes[i].normal.xyz, planes[i].distance);
    if (d > 0 && result.distance > d)
    {
      result.distance = d;
      result.normal = planes[i].normal.xyz;
      result.materialIndex = planes[i].materialIndex;
      result.textureCoordinates = (rayDirection * d + rayOrigin).xz;
      result.tangent = vec3(1, 0, 0);
      result.bitangent = vec3(0, 0, 1);
    }
  }
  result.position = rayDirection * (result.distance) + rayOrigin;
  return result;
}

vec3 castRay(vec3 rayOrigin, vec3 rayDirection)
{
  vec3 resultColor = vec3(0);

  for (uint sampleCounter = 0; sampleCounter < samples; sampleCounter++)
  {
    vec3 org = rayOrigin, dir = rayDirection;
    vec3 sampleColor = vec3(0);

    for (uint i = 0; i < bounces; i++)
    {
      ClosestHit hit = closestHit(org, dir);
      if (hit.distance == FAR_PLANE)
      {
        float theta = atan(sqrt(dir.x * dir.x + dir.z * dir.z), dir.y);
        float phi = atan(dir.x, dir.z);
        if (sampleColor == vec3(0))
          sampleColor = texture(sky, vec2(phi / PI / 2.0f + 0.5, theta / PI)).rgb;
        else
          sampleColor *= texture(sky, vec2(phi / PI / 2.0f + 0.5, theta / PI)).rgb;
        break;
      }
      else
      {

        vec3 albedo;
        if (materials[hit.materialIndex].textureIndex != -1)
          albedo = texture(texArray, vec3(hit.textureCoordinates * materials[hit.materialIndex].textureCoordinatesMultiplier, materials[hit.materialIndex].textureIndex)).rgb;
        else
          albedo = materials[hit.materialIndex].albedo;

        float roughness;
        if (materials[hit.materialIndex].metallicTextureIndex != -1)
          roughness = texture(texArray, vec3(hit.textureCoordinates * materials[hit.materialIndex].textureCoordinatesMultiplier, materials[hit.materialIndex].metallicTextureIndex)).r;
        else
          roughness = materials[hit.materialIndex].roughness;

        vec3 normal;
        if (materials[hit.materialIndex].normalTextureIndex != -1)
        {
          normal = texture(texArray, vec3(hit.textureCoordinates * materials[hit.materialIndex].textureCoordinatesMultiplier, materials[hit.materialIndex].normalTextureIndex)).rgb * 2.0 - 1;
          mat3 TBN = mat3(hit.tangent, hit.bitangent, hit.normal);
          normal = TBN * normal;
        }
        else
          normal = hit.normal;

        if (sampleColor == vec3(0))
          sampleColor = albedo;
        else
          sampleColor *= albedo;

        org = hit.position + normal * 1e-6;
        dir = reflect(dir, normal + rand3(dir + sampleCounter) * roughness);
      }
    }

    resultColor += sampleColor;
  }
  return resultColor / samples;
}
