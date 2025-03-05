#pragma once

namespace raytracing
{
  using uchar = unsigned char;
  using byte = char;
  using ubyte = unsigned char;

  enum class status
  {
    success = 0,
    error = 1,
    file_not_found = 2,
  };

  struct init_options
  {
    std::string title = "Ray Tracing";
    std::filesystem::path scene_filename;
    bool maximized = true;
    uint32_t width = 1200, height = 700;
  };
}
