#pragma once

#include <set>
#include <cstdint>
#include <cstdio>
#include <imgui.h>
#include <string>
#include <filesystem>

#include <glm/glm.hpp>

namespace raytracing
{
  enum class status
  {
    success = 0,
    error = 1,
    file_not_found = 2,
  };
}