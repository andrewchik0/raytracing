#version 420

in vec2 texCoord;
out vec2 passTexCoord;

void main()
{
  passTexCoord = texCoord;
  gl_Position = vec4(texCoord * 2.0 - 1.0, 0.0, 1.0);
}