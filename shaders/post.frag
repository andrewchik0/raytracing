#version 410

in vec2 passTexCoord;

uniform sampler2D renderedTexture;

void main()
{
  gl_FragColor = vec4(texture(renderedTexture, passTexCoord).xyz, 1.0);
}