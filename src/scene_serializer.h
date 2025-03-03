#pragma once

#include "pch.h"

namespace raytracing
{
  class scene_serializer
  {
  public:
    scene_serializer() = default;
    scene_serializer(const scene_serializer&) = delete;

    void save(const std::filesystem::path& filename);
    void load(const std::filesystem::path& filename);
    void load();
  };
} // namespace raytracing
