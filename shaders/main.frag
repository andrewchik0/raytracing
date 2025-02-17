#version 410

in vec2 passTexCoord;

uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float halfHeight;
uniform float halfWidth;
uniform vec2 windowSize;

#include "ray_tracing.glsl"

void main()
{
  vec3 rayDirection = calculateRayDirection();

  float sphereDistance = raySphereIntersect(cameraPosition, rayDirection, vec3(0.0), .8);
  if (sphereDistance == -1.0)
    discard;
  vec3 normal = normalize(rayDirection * sphereDistance + cameraPosition);
  gl_FragColor = vec4(vec3(dot(normal, normalize(vec3(0.8, 1.0, 1.0)))), 1.0);
}