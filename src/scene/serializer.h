#pragma once

#include "../pch.h"

namespace raytracing
{
  class serializer
  {
  public:
    static void save(const std::filesystem::path& filename);
    static void load(const std::filesystem::path& filename);
    static void load();
  };
} // namespace raytracing
