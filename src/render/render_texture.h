#pragma once
#include "shader.h"

namespace raytracing
{
  class render_texture
  {
  public:
    static uint32_t sQuadVAO, sQuadVBO;

    render_texture() = default;
    render_texture(const render_texture&) = delete;

    ~render_texture();

    bool resize(uint32_t width, uint32_t height);
    void clear();
    void draw(shader& shader);

    uint32_t width() const;
    uint32_t height() const;

    uint32_t get_texture() const;

  private:

    void destroy();

    uint32_t mWidth = 0, mHeight = 0;
    uint32_t mTextureHandle = 0;
    uint32_t mFrameBufferHandle = 0;

  };
}
