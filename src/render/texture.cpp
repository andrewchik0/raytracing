#include "texture.h"

#include <stb_image_write.h>
#include <stb_image.h>

namespace raytracing
{

  texture::~texture() { destroy(); }

  uint32_t texture::width() const { return mWidth; }
  uint32_t texture::height() const { return mHeight; }

  uint32_t texture::get_texture_handle() const { return mTextureHandle; }

  void texture::destroy()
  {
    if (glIsTexture(mTextureHandle))
      glDeleteTextures(1, &mTextureHandle);
    mTextureHandle = 0;
  }

  void texture::from_file(const std::filesystem::path& path)
  {
    int channels, w, h;
    uchar* data = stbi_load(path.string().c_str(), &w, &h, &channels, 4);
    if (!data)
      return;

    mWidth = w;
    mHeight = h;
    glGenTextures(1, &mTextureHandle);
    glBindTexture(GL_TEXTURE_2D, mTextureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int mipLevels = static_cast<int>(std::log2(std::max(mWidth, mHeight))) + 1;
    glTexStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA8, mWidth, mHeight);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void texture::write_to_file(const std::filesystem::path& path)
  {
    std::vector<uchar> pixels(mWidth * mHeight * 4);

    glBindTexture(GL_TEXTURE_2D, mTextureHandle);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_write_png(path.string().c_str(), mWidth, mHeight, 4, pixels.data(), 0);
  }

  void texture::copy_from(const texture& other)
  {
    glCopyImageSubData(other.get_texture_handle(), GL_TEXTURE_2D, 0, 0, 0, 0, mTextureHandle, GL_TEXTURE_2D, 0, 0, 0, 0,
                       mWidth, mHeight, 1);
  }

  void texture::set_repeated(bool repeated)
  {
    glBindTexture(GL_TEXTURE_2D, mTextureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
} // namespace raytracing
