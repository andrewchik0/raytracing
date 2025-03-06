float random(vec2 st)
{
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float random(float seed)
{
  return random(vec2(time, seed));
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

vec3 randomOnSphere(vec3 seed)
{
  vec3 rand = vec3(random(vec2(time, seed.x)), random(vec2(time, seed.y)), random(vec2(time, seed.z)));
  float theta = rand.x * 2.0 * 3.14159265;
  float v = rand.y;
  float phi = acos(2.0 * v - 1.0);
  float r = pow(rand.z, 1.0 / 3.0);
  float x = r * sin(phi) * cos(theta);
  float y = r * sin(phi) * sin(theta);
  float z = r * cos(phi);
  return vec3(x, y, z);
}


float atan2(float y, float x)
{
  bool s = bool(abs(x) > abs(y));
  return mix(PI / 2.0 - atan(x, y), atan(y, x), s);
}

vec3 gaussian_blur(sampler2D tex, vec2 windowSize, vec2 texCoords, float radius)
{
  float radiusSquared = radius * radius;
  float pixelSizeX = 1.0 / windowSize.x;
  float pixelSizeY = 1.0 / windowSize.y;
  vec2 center = vec2(texCoords.x, 1.0 - texCoords.y);
  float baseWeight = 1 / (2.0 * PI * radiusSquared);
  vec3 resultColor = vec3(0.0);

  for (float x = center.x - pixelSizeX * radius; x <= center.x + pixelSizeX * radius; x += pixelSizeX)
  {
    for (float y = center.y - pixelSizeY * radius; y <= center.y + pixelSizeY * radius; y += pixelSizeY)
    {
      float weight = baseWeight * exp(-(x * x + y * y) / (2.0 * radiusSquared));
      resultColor += texture(tex, vec2(x, y)).rgb * weight;
    }
  }

  return resultColor;
}
