#pragma once

namespace raytracing
{
  class texture
  {
  public:

    texture() = default;
    texture(const texture&) = delete;

    ~texture();

    void from_file(const std::filesystem::path& path);
    void write_to_file(const std::filesystem::path& path);

    uint32_t width() const;
    uint32_t height() const;

    uint32_t get_handle() const;

  protected:

    void destroy();

    uint32_t mWidth = 0, mHeight = 0;
    uint32_t mTextureHandle = 0;
  };
}
