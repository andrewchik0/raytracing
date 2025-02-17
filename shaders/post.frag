#version 420
layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2D renderedTexture;
uniform vec2 windowSize;

#include "fxaa.glsl"

void main()
{
  outColor = fxaa(renderedTexture, passTexCoord, windowSize);
}