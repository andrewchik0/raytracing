#version 410

in vec2 passTexCoord;

uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform vec2 windowSize;

float raySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sr)
{
  // - r0: ray origin
  // - rd: normalized ray direction
  // - s0: sphere center
  // - sr: sphere radius
  // - Returns distance from r0 to first intersecion with sphere,
  //   or -1.0 if no intersection.
  float a = dot(rd, rd);
  vec3 s0_r0 = r0 - s0;
  float b = 2.0 * dot(rd, s0_r0);
  float c = dot(s0_r0, s0_r0) - (sr * sr);
  if (b * b - 4.0 * a * c < 0.0)
  {
    return -1.0;
  }
  return (-b - sqrt((b * b) - 4.0 * a * c)) / (2.0 * a);
}

void main()
{
  vec2 rayShift = vec2(passTexCoord * 2.0 - 1);
  rayShift.x *= windowSize.x / windowSize.y;
  float sphere = raySphereIntersect(cameraPosition, normalize(cameraDirection + vec3(rayShift, 0.0)), vec3(0.0), .8);
  gl_FragColor = vec4(1.0 - vec3(pow(sphere / 2, 2.0)), 1.0);
}