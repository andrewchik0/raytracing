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
    grid(uint32_t width, uint32_t depth, const std::vector<float>& heights) { load(width, depth, heights); }
    grid& operator=(grid&&) = default;

    grid(const grid&) = delete;
    grid& operator=(const grid&) = delete;

    void load(uint32_t width, uint32_t depth, const std::vector<float>& heights);

    std::vector<glm::ivec4> mTriangles;
    std::vector<Vertex> mVertices;
  private:
  };
}