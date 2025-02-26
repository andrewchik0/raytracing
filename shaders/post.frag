#include "version.glsl"

layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

#include "uniforms.h"
layout (std140, binding = GLOBAL_DATA_BINDING) uniform GlobalDataStruct;

#include "utils.glsl"
#include "fxaa.glsl"

uniform sampler2D renderedTexture;
uniform sampler2D bloomTexture;

void main()
{
  if (useFXAA == 1)
  {
    outColor = fxaa(renderedTexture, passTexCoord, windowSize.xy);
  }
  else
  {
    outColor = vec4(texture(renderedTexture, passTexCoord).xyz, 1.0);
  }

  if (renderMode == 1)
  {
    outColor.rgb += max(gaussian_blur(bloomTexture, windowSize.xy, passTexCoord, blurSize), vec3(0.0));
  }

  vec3 mapped = vec3(1.0) - exp(-outColor.rgb * exposure);
  mapped = pow(mapped, vec3(1.0 / gamma));
  outColor = vec4(mapped, 1.0);
}