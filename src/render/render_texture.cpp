#include "render_texture.h"

namespace raytracing
{
  uint32_t render_texture::sQuadVAO = 0;
  uint32_t render_texture::sQuadVBO = 0;

  render_texture::~render_texture() { destroy(); }

  bool render_texture::resize(const uint32_t width, const uint32_t height)
  {
    if (sQuadVAO == 0 || sQuadVBO == 0)
    {
      GLfloat vertices[] =
      {
        // Positions         // Texture Coords (optional)
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // Top-left (v0)
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // Top-right (v1)
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // Bottom-left (v2)
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f  // Bottom-right (v3)
      };
      glGenVertexArrays(1, &sQuadVAO);
      glGenBuffers(1, &sQuadVBO);

      glBindVertexArray(sQuadVAO);
      glBindBuffer(GL_ARRAY_BUFFER, sQuadVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      // Position attribute (assuming vec3 position)
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
      glEnableVertexAttribArray(0);

      // Texture coordinates attribute (optional)
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
      glEnableVertexAttribArray(1);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    destroy();
    mWidth = width;
    mHeight = height;
    glGenFramebuffers(1, &mFrameBufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferHandle);

    glGenTextures(1, &mTextureHandle);
    glBindTexture(GL_TEXTURE_2D, mTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureHandle, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      return false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
  }

  void render_texture::clear()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferHandle);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void render_texture::draw(shader& shader)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferHandle);
    glBindVertexArray(sQuadVAO);
    shader.bind_textures();
    glViewport(0, 0, mWidth, mHeight);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  uint32_t render_texture::width() const { return mWidth; }
  uint32_t render_texture::height() const { return mHeight; }

  uint32_t render_texture::get_texture() const
  {
    return mTextureHandle;
  }

  void render_texture::destroy()
  {
    if (glIsTexture(mTextureHandle))
      glDeleteTextures(1, &mTextureHandle);
    if (glIsFramebuffer(mFrameBufferHandle))
      glDeleteFramebuffers(1, &mFrameBufferHandle);
    mWidth = 0, mHeight = 0;
  }
} // namespace raytracing
