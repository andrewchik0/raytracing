#include "version.glsl"

layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2D renderedTexture;
uniform sampler2D bloomTexture;
uniform vec2 windowSize;
uniform bool useFXAA;
uniform float gamma;
uniform float exposure;

#include "fxaa.glsl"

void main()
{
  float r = 20;
  float x, y, rr = r * r, d, w, w0;
  vec2 p = 0.5 * (vec2(1.0, 1.0) + passTexCoord * 2 - 1);
  vec4 col1 = vec4(0.0, 0.0, 0.0, 0.0);
  w0 = 0.5135 / pow(r, 0.96);

  for (d = 1.0 / windowSize.x, x = -r, p.x += x * d; x <= r; x++)
  {
    w = w0 * exp((-x * x) / (2.0 * rr));
    col1 += texture(bloomTexture, p) * w;
    p.x += d;
  }

  p = 0.5 * (vec2(1.0, 1.0) + passTexCoord * 2 - 1);
  vec4 col2 = vec4(0.0, 0.0, 0.0, 0.0);
  w0 = 0.5135 / pow(r, 0.96);

  for (d = 1.0 / windowSize.y, y = -r, p.y += y * d; y <= r; y++)
  {
    w = w0 * exp((- y * y) / (2.0 * rr));
    col2 += texture(bloomTexture, p) * w;
    p.y += d;
  }
  vec4 col = max(col1, col2);

  if (useFXAA)
  {
    outColor = fxaa(renderedTexture, passTexCoord, windowSize);
  }
  else
  {
    outColor = vec4(texture(renderedTexture, passTexCoord).xyz, 1.0);
  }

  outColor += col;

  vec3 mapped = vec3(1.0) - exp(-outColor.rgb * exposure);
  mapped = pow(mapped, vec3(1.0 / gamma));
  outColor = vec4(mapped, 1.0);
}