#pragma once
#include "shader.h"
#include "texture.h"

namespace raytracing
{
  class render_texture : public texture
  {
  public:

    render_texture() = default;
    render_texture(const render_texture&) = delete;

    ~render_texture();

    bool resize(uint32_t width, uint32_t height);
    void clear();
    void draw(shader& shader);

  private:

    void destroy();

    uint32_t mFrameBufferHandle = 0;

    static uint32_t sQuadVAO, sQuadVBO;
  };
}
