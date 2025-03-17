#include "grid.h"

namespace raytracing
{
  void grid::load(uint32_t width, uint32_t depth, const std::vector<float>& heights, const float scale /* = 1.0 */, const uint32_t materialIndex /* = 0 */)
  {
    mVertices.clear();
    mTriangles.clear();

    auto getHeight = [&](const int x, const int z) -> float
    {
      if (x < 0 || x >= width || z < 0 || z >= depth)
        return 0.0f;
      return heights[z * width + x];
    };

    for (size_t j = 0; j < depth; j++)
    {
      for (size_t i = 0; i < width; i++)
      {
        float hL = getHeight(i - 1, j); // Left
        float hR = getHeight(i + 1, j); // Right
        float hD = getHeight(i, j - 1); // Down
        float hU = getHeight(i, j + 1); // Up
        float centerHeight = getHeight(i, j);

        // Compute tangent vectors
        glm::vec3 dx = glm::normalize(glm::vec3(2.0f, hR - hL, 0.0f));
        glm::vec3 dz = glm::normalize(glm::vec3(0.0f, hU - hD, 2.0f));

        // Compute normal as cross product
        glm::vec3 normal = glm::normalize(glm::cross(dz, dx));

        Vertex vertex;
        vertex.position = glm::vec4(-(float)width / 2.0f + i, centerHeight, -(float)depth / 2.0f + j, 1.0f);
        vertex.position.x *= scale;
        vertex.position.z *= scale;
        vertex.normal = glm::vec4(normal, 0.0f);
        vertex.tangent = glm::vec4(dx, float(i) / width);
        vertex.bitangent = glm::vec4(dz, float(j) / depth);

        mVertices.push_back(vertex);
      }
    }

    for (size_t j = 0; j < depth - 1; j++)
    {
      for (size_t i = 0; i < width - 1; i++)
      {
        uint32_t topLeft = j * width + i;
        uint32_t topRight = j * width + (i + 1);
        uint32_t bottomLeft = (j + 1) * width + i;
        uint32_t bottomRight = (j + 1) * width + (i + 1);

        mTriangles.push_back(glm::ivec4(topLeft, bottomLeft, topRight, materialIndex));
        mTriangles.push_back(glm::ivec4(topRight, bottomLeft, bottomRight, materialIndex));
      }
    }
  }
}
