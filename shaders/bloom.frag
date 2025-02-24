#include "version.glsl"

layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2D renderedTexture;

void main()
{
  vec3 color = texture(renderedTexture, passTexCoord).xyz;
  float brightness = dot(color, vec3(1.0));
  if (brightness > 1.0)
    outColor = vec4(color, 1.0);
  else
    outColor = vec4(0.0);
}