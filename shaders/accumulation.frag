#include "version.glsl"

out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2D lastFrameTexture;
uniform sampler2D accumulatedTexture;
uniform int frameIndex;

void main()
{
  vec4 accumulated = texture(accumulatedTexture, passTexCoord);
  vec4 lastFrame = texture(lastFrameTexture, passTexCoord);
  outColor = (accumulated * (frameIndex - 1) + lastFrame) / frameIndex;
}