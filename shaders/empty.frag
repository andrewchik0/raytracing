#include "version.glsl"

out vec4 outColor;
in vec2 passTexCoord;
uniform sampler2D frameTexture;

void main()
{
  outColor = texture(frameTexture, passTexCoord);
}