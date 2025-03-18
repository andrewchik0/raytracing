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
    file_not_found = 2
  };

  inline status operator|(status left, status right)
  {
    return static_cast<status>(static_cast<int>(left) | static_cast<int>(right));
  }

  inline status operator|=(status& _this, status other)
  {
    return _this = static_cast<status>(static_cast<int>(_this) | static_cast<int>(other));
  }

  inline status operator&(status left, status right)
  {
    return static_cast<status>(static_cast<int>(left) & static_cast<int>(right));
  }

  inline status operator&=(status& _this, status other)
  {
    return _this = static_cast<status>(static_cast<int>(_this) & static_cast<int>(other));
  }

  struct init_options
  {
    std::string title = "Ray Tracing";
    std::filesystem::path scene_filename = "scenes/empty.yaml";
    bool maximized = true;
    uint32_t width = 1200, height = 700;
  };
}
