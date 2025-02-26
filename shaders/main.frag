#include "version.glsl"
layout(location = 0) out vec4 outColor;

in vec2 passTexCoord;

uniform sampler2D sky;
uniform sampler2DArray texArray;
uniform isampler2DArray trianglesTexture;
uniform sampler2DArray verticesTexture;
uniform sampler2DArray boundingVolumesTexture;

#include "uniforms.h"

layout (std140, binding = GLOBAL_DATA_BINDING) uniform GlobalDataStruct;
layout (std140, binding = SCENE_BINDING) uniform SceneBufferStruct;

#include "ray_tracing.glsl"

void main()
{
  Ray ray;
  ray.origin = cameraPosition.xyz;
  ray.direction = calculateRayDirection(cameraDirection.xyz, cameraRight.xyz, cameraUp.xyz, passTexCoord, halfWidth, halfHeight);
  outColor = vec4(castRay(ray), 1.0);
}