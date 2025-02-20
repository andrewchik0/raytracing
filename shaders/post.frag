#include "version.glsl"

layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2D renderedTexture;
uniform vec2 windowSize;
uniform bool useFXAA;

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
}