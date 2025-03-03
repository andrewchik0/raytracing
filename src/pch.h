#pragma once

#include <set>
#include <unordered_map>
#include <array>
#include <cstdint>
#include <cstdio>
#include <imgui.h>
#include <string>
#include <filesystem>

#include <glm/glm.hpp>

#include <GL/glew.h>

namespace raytracing
{
  using uchar = unsigned char;

  enum class status
  {
    success = 0,
    error = 1,
    file_not_found = 2,
  };
}
