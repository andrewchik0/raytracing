#pragma once

#include "pch.h"

namespace YAML
{
  class Emitter;
};

namespace raytracing
{
  class scene_serializer
  {
  public:
    void save(const std::filesystem::path& filename);
    void load(const std::filesystem::path& filename);
    void load();
  };
} // namespace raytracing
