#pragma once

#include <glm/glm.hpp>

namespace raytracing
{
  struct AABB
  {
    glm::vec3 min, max;

    AABB() : min(glm::vec3(FLT_MAX)), max(glm::vec3(-FLT_MAX)) {}

    void expand(const glm::vec3& point)
    {
      min = glm::min(min, point);
      max = glm::max(max, point);
    }

    void expand(const AABB& other)
    {
      min = glm::min(min, other.min);
      max = glm::max(max, other.max);
    }

    glm::vec3 centroid() const
    {
      return (min + max) * 0.5f;
    }
  };

  struct BVHNode
  {
    AABB bounds;
    int left = -1, right = -1;
    int start = 0, count = 0;
  };


  class bounding_volume_builder
  {
  public:
    uint32_t mObjectPerNode = 8;

    void build();

  private:
    std::vector<BVHNode> mBVHNodes;

    void store_bvh();
    void build_node(int32_t nodeIndex, std::vector<uint32_t>& triangleIndices, int32_t start, int32_t end);
  };
}
