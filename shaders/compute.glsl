layout(rgba32f, binding = 0) uniform writeonly image2D outputImage;

#include "work_group_size.h"
layout(local_size_x = WORK_GROUP_SIZE_X, local_size_y = WORK_GROUP_SIZE_Y) in;

ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
ivec2 texSize = imageSize(outputImage) - 1;
vec2 texCoordUpsideDown = vec2(pixelCoord) / vec2(texSize);
vec2 texCoord = vec2(texCoordUpsideDown.x, 1.0 - texCoordUpsideDown.y);

shared vec2 sharedUV[WORK_GROUP_SIZE_X][WORK_GROUP_SIZE_Y];
shared uint sharedMaterialID[WORK_GROUP_SIZE_X][WORK_GROUP_SIZE_Y];

void loadUV(vec2 uv, uint materialIndex)
{
  ivec2 lid = ivec2(gl_LocalInvocationID.xy);
  sharedUV[lid.x][lid.y] = uv;
  sharedMaterialID[lid.x][lid.y] = materialIndex;
  barrier(); // Ensure all threads have loaded data before using
}

vec2 getDDX()
{
  ivec2 lid = ivec2(gl_LocalInvocationID.xy);
  vec2 uv = vec2(0);
  uv = (lid.x < WORK_GROUP_SIZE_X - 1) ? (sharedUV[lid.x + 1][lid.y] - sharedUV[lid.x][lid.y]) : (sharedUV[lid.x - 1][lid.y] - sharedUV[lid.x][lid.y]);
  if (length(uv) > 0.1) return vec2(0.0);
  return uv;
}

vec2 getDDY()
{
  ivec2 lid = ivec2(gl_LocalInvocationID.xy);
  vec2 uv = vec2(0.0);
  uv = (lid.y < WORK_GROUP_SIZE_Y - 1) ? (sharedUV[lid.x][lid.y + 1] - sharedUV[lid.x][lid.y]) : (sharedUV[lid.x][lid.y - 1] - sharedUV[lid.x][lid.y]);
  if (length(uv) > 0.1) return vec2(0.0);
  return uv;
}

void outColor(vec4 color)
{
  imageStore(outputImage, pixelCoord, color);
}