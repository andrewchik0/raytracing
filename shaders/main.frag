#include "version.glsl"
layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2DArray sky;
uniform sampler2DArray texArray;

#include "uniforms.h"

layout (std140, binding = GLOBAL_DATA_BINDING) uniform GlobalDataStruct;
layout (std140, binding = SCENE_BINDING) uniform SceneBufferStruct;

#include "ray_tracing.glsl"

void main()
{
  vec3 rayDirection = calculateRayDirection();
  outColor = vec4(castRay(cameraPosition, rayDirection), 1.0);
}