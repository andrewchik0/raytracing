#pragma once

#include "pch.h"
#include "shaders/uniforms.h"

namespace raytracing
{
  class grid
  {
  public:
    grid() = default;
    grid(grid&&) = default;
    grid(const uint32_t width, const uint32_t depth, const std::vector<float>& heights, const float scale = 1.0f, const uint32_t materialIndex = 0)
      { load(width, depth, heights, scale, materialIndex); }
    grid& operator=(grid&&) = default;

    grid(const grid&) = delete;
    grid& operator=(const grid&) = delete;

    void load(uint32_t width, uint32_t depth, const std::vector<float>& heights, float scale = 1.0f, uint32_t materialIndex = 0);

    std::vector<glm::ivec4> mTriangles;
    std::vector<Vertex> mVertices;
  private:
  };
}