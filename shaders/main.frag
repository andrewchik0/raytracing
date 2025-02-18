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
  outColor = vec4(castRay(cameraPosition, rayDirection), 1.0);
}