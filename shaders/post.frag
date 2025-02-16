#version 410

in vec2 passTexCoord;

uniform sampler2D renderedTexture;
uniform vec2 windowSize;

#include "fxaa.glsl"

void main()
{
  gl_FragColor = fxaa(renderedTexture, passTexCoord, windowSize);
}