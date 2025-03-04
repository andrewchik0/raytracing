#pragma once

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

namespace raytracing::math
{
  inline glm::vec3 getNormalizedProjection(const glm::vec3 v)
  {
    return glm::normalize(glm::vec3(v.x, 0.0, v.z));
  }
}