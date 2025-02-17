#version 420
layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float halfHeight;
uniform float halfWidth;
uniform vec2 windowSize;
uniform vec3 lightDirection;

#include "uniforms.h"

layout (std140, binding = SCENE_BINDING) uniform SceneBufferStruct;

#include "ray_tracing.glsl"

void main()
{
  vec3 rayDirection = calculateRayDirection();

  CastRay ray = castRay(cameraPosition, rayDirection);

  if (ray.distance == FAR_PLANE)
  {
    outColor = vec4(0.5, 0.7, 1.0, 1.0);
    return;
  }

  outColor = vec4(max(vec3(dot(ray.normal, normalize(lightDirection))) * ray.color, 0.1), 1.0);
}