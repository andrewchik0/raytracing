#version 410

in vec2 passTexCoord;

uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float halfHeight;
uniform float halfWidth;
uniform vec2 windowSize;

#include "uniforms.h"
#include "ray_tracing.glsl"

void main()
{
  vec3 rayDirection = calculateRayDirection();
  vec3 lightDirection = vec3(0.8, 1.0, 1.0);

  float sphereDistance = raySphereIntersect(cameraPosition, rayDirection, vec3(0.0), .8);
  float planeDistance = rayPlaneIntersect(cameraPosition, rayDirection, vec3(0.0, 1.0, 0.0), .8);

  vec3 normal;
  float att = 1.0;
  if (sphereDistance == -1.0 && planeDistance == -1.0)
  {
    gl_FragColor = vec4(0.5, 0.7, 1.0, 1.0);
    return;
  }
  if (sphereDistance != -1.0 && planeDistance != -1.0)
  {
    if (sphereDistance < planeDistance)
    {
      normal = normalize(rayDirection * sphereDistance + cameraPosition);
    }
    else
    {
      normal = vec3(0.0, 1.0, 0.0);
    }
  }
  else if (sphereDistance != -1.0)
  {
    normal = normalize(rayDirection * sphereDistance + cameraPosition);
  }
  else
  {
    normal = vec3(0.0, 1.0, 0.0);
    if (raySphereIntersect(cameraPosition + planeDistance * rayDirection, -lightDirection, vec3(0.0), .8) != -1.0)
    {
      att = 0.5;
    }
  }
  gl_FragColor = vec4(max(vec3(dot(normal, normalize(lightDirection))), 0.1) * att, 1.0);
}