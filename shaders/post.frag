#include "version.glsl"

layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2D renderedTexture;
uniform sampler2D bloomTexture;
uniform vec2 windowSize;
uniform bool useFXAA;
uniform float gamma;
uniform float exposure;
uniform float blurSize;
uniform float time;

#include "utils.glsl"
#include "fxaa.glsl"

void main()
{
  float radiusSquared = blurSize * blurSize;
  float pixelSizeX = 1.0 / windowSize.x;
  float pixelSizeY = 1.0 / windowSize.y;
  vec2 center = passTexCoord;
  float baseWeight = 1 / (2.0 * PI * radiusSquared);
  vec3 resultColor = vec3(0.0);

  for (float x = center.x - pixelSizeX * blurSize; x <= center.x + pixelSizeX * blurSize; x += pixelSizeX)
  {
    for (float y = center.y - pixelSizeY * blurSize; y <= center.y + pixelSizeY * blurSize; y += pixelSizeY)
    {
      float weight = baseWeight * exp(-(x * x + y * y) / (2.0 * radiusSquared));
      resultColor += texture(bloomTexture, vec2(x, y)).rgb * weight;
    }
  }

  if (useFXAA)
  {
    outColor = fxaa(renderedTexture, passTexCoord, windowSize);
  }
  else
  {
    outColor = vec4(texture(renderedTexture, passTexCoord).xyz, 1.0);
  }

  outColor.rgb += max(resultColor, vec3(0.0));

  vec3 mapped = vec3(1.0) - exp(-outColor.rgb * exposure);
  mapped = pow(mapped, vec3(1.0 / gamma));
  outColor = vec4(mapped, 1.0);
}