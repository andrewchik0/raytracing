#include "version.glsl"

layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2D renderedTexture;
uniform vec2 windowSize;
uniform bool useFXAA;
uniform float gamma;
uniform float exposure;

#include "fxaa.glsl"

void main()
{
  if (useFXAA)
  {
    outColor = fxaa(renderedTexture, passTexCoord, windowSize);
  }
  else
  {
    outColor = vec4(texture(renderedTexture, passTexCoord).xyz, 1.0);
  }

  vec3 mapped = vec3(1.0) - exp(-outColor.rgb * exposure);
  mapped = pow(mapped, vec3(1.0 / gamma));
  outColor = vec4(mapped, 1.0);
}