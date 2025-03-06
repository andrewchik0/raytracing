layout(rgba32f, binding = 0) uniform writeonly image2D outputImage;

layout(local_size_x = 16, local_size_y = 16) in;

ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
ivec2 texSize = imageSize(outputImage) - 1;
vec2 texCoordUpsideDown = vec2(pixelCoord) / vec2(texSize);
vec2 texCoord = vec2(texCoordUpsideDown.x, 1.0 - texCoordUpsideDown.y);

void outColor(vec4 color)
{
  imageStore(outputImage, pixelCoord, color);
}