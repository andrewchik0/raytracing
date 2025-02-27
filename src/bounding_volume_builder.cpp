#include "bounding_volume_builder.h"

#include <numeric>

#include "../shaders/uniforms.h"
#include "rt.h"

namespace raytracing
{
  void bounding_volume_builder::build_node(int32_t nodeIndex, std::vector<uint32_t>& triangleIndices, int32_t start, int32_t end)
  {
    BVHNode* node = &(mBVHNodes[nodeIndex]);
    node->start = start;
    node->count = end - start;

    for (size_t i = start; i < end; ++i)
    {
      TriangleObject& tri = rt::get()->mRender.mTriangles[triangleIndices[i]];
      glm::vec3 v0 = rt::get()->mRender.mVertices[tri.indices.x].position;
      glm::vec3 v1 = rt::get()->mRender.mVertices[tri.indices.y].position;
      glm::vec3 v2 = rt::get()->mRender.mVertices[tri.indices.z].position;

      node->bounds.expand(v0);
      node->bounds.expand(v1);
      node->bounds.expand(v2);
    }

    if (node->count <= mObjectPerNode)
    {
      return;
    }

    glm::vec3 size = node->bounds.max - node->bounds.min;
    int32_t axis = size.x > size.y ? (size.x > size.z ? 0 : 2) : (size.y > size.z ? 1 : 2);

    std::sort(
      triangleIndices.begin() + start, triangleIndices.begin() + end,
      [&](int32_t a, int32_t b)
      {
        glm::vec3 ca = (rt::get()->mRender.mVertices[rt::get()->mRender.mTriangles[a].indices.x].position +
                        rt::get()->mRender.mVertices[rt::get()->mRender.mTriangles[a].indices.y].position +
                        rt::get()->mRender.mVertices[rt::get()->mRender.mTriangles[a].indices.z].position) / 3.0f;
        glm::vec3 cb = (rt::get()->mRender.mVertices[rt::get()->mRender.mTriangles[b].indices.x].position +
                        rt::get()->mRender.mVertices[rt::get()->mRender.mTriangles[b].indices.y].position +
                        rt::get()->mRender.mVertices[rt::get()->mRender.mTriangles[b].indices.z].position) / 3.0f;
        return ca[axis] < cb[axis];
      }
    );

    int32_t mid = (start + end) / 2;

    int32_t leftIndex = mBVHNodes.size();
    mBVHNodes.emplace_back();
    int32_t rightIndex = mBVHNodes.size();
    mBVHNodes.emplace_back();

    mBVHNodes[nodeIndex].left = leftIndex;
    mBVHNodes[nodeIndex].right = rightIndex;

    mBVHNodes[nodeIndex].count = 0;
    mBVHNodes[nodeIndex].start = 0;

    build_node(leftIndex, triangleIndices, start, mid);
    build_node(rightIndex, triangleIndices, mid, end);
  }

  void bounding_volume_builder::store_bvh()
  {
    for (auto it = rt::get()->mRender.mBoundingVolumes.begin(); it != rt::get()->mRender.mBoundingVolumes.end(); ++it)
    {
      *it = BoundingVolume {};
    }

    for (auto it = mBVHNodes.begin(); it < mBVHNodes.end(); ++it)
    {
      rt::get()->mRender.mBoundingVolumes.push_back(BoundingVolume
      {
        it->bounds.min,
        float(it->left),
        it->bounds.max,
        float(it->right),
        glm::ivec4(it->count, it->start, 0, 0)
      });
    }
  }

  void bounding_volume_builder::build()
  {
    std::vector<uint32_t> triangleIndices(rt::get()->mRender.mTriangles.size());
    std::iota(triangleIndices.begin(), triangleIndices.end(), 0);

    mBVHNodes.clear();
    mBVHNodes.emplace_back();

    build_node(0, triangleIndices, 0, rt::get()->mRender.mTriangles.size());
    store_bvh();

    std::vector<TriangleObject> triangleCopies(rt::get()->mRender.mTriangles.size());
    for (size_t i = 0; i < triangleIndices.size(); ++i)
    {
      triangleCopies[i] = rt::get()->mRender.mTriangles[triangleIndices[i]];
    }
    std::copy_n(triangleCopies.begin(), triangleCopies.size(), rt::get()->mRender.mTriangles.begin());

    mBVHNodes.clear();
  }
}