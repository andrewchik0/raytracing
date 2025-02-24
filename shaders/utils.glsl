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

vec3 gaussian_blur(sampler2D tex, vec2 windowSize, vec2 texCoords, float radius)
{
  float radiusSquared = radius * radius;
  float pixelSizeX = 1.0 / windowSize.x;
  float pixelSizeY = 1.0 / windowSize.y;
  vec2 center = texCoords;
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