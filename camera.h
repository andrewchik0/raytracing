#pragma once

#include <glm\glm.hpp>

namespace raytracing
{
  class camera
  {
  public:
    glm::vec3 position { 0, 0, -2 };
    glm::vec3 direction { 0, 0, 1 };
  };
}