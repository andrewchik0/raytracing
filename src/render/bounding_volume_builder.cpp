#include "bounding_volume_builder.h"

#include <numeric>

#include "shaders/uniforms.h"
#include "rt.h"

namespace raytracing
{
  void bounding_volume_builder::build_node(int32_t nodeIndex, std::vector<uint32_t>& triangleIndices, int32_t start, int32_t end)
  {
    BVHNode* node = &(mBVHNodes[nodeIndex]);
    node->start = start;
    node->count = end - start;

    auto& render = rt::get()->mRender;

    for (size_t i = start; i < end; ++i)
    {
      glm::ivec3 tri = render.mTriangles[triangleIndices[i]];
      glm::vec3 v0 = render.mVertices[tri.x].position;
      glm::vec3 v1 = render.mVertices[tri.y].position;
      glm::vec3 v2 = render.mVertices[tri.z].position;

      node->bounds.expand(v0);
      node->bounds.expand(v1);
      node->bounds.expand(v2);
    }

    node->bounds.min -= 1e-5f;
    node->bounds.max += 1e-5f;

    if (node->count <= mObjectPerNode)
    {
      return;
    }

    glm::vec3 size = node->bounds.max - node->bounds.min;
    int32_t axis = (size.x > size.y) ? ((size.x > size.z) ? 0 : 2) : ((size.y > size.z) ? 1 : 2);

    std::unordered_map<int32_t, float> centroidMap;
    for (int32_t i = start; i < end; ++i)
    {
      glm::ivec3 tri = render.mTriangles[triangleIndices[i]];
      glm::vec3 v0 = render.mVertices[tri.x].position;
      glm::vec3 v1 = render.mVertices[tri.y].position;
      glm::vec3 v2 = render.mVertices[tri.z].position;
      centroidMap[triangleIndices[i]] = (v0[axis] + v1[axis] + v2[axis]) / 3.0f;
    }

    int32_t mid = (start + end) / 2;
    std::nth_element(
      triangleIndices.begin() + start, triangleIndices.begin() + mid, triangleIndices.begin() + end,
      [&](int32_t a, int32_t b) {
        return centroidMap[a] < centroidMap[b];
      }
    );

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

    int maxTriangle = 0;

    for (auto it = mBVHNodes.begin(); it < mBVHNodes.end(); ++it)
    {
      if (rt::get()->mRender.mTriangles.size() > it->start && it->left == -1 && it->right == -1)
      {
        maxTriangle = glm::max(rt::get()->mRender.mTriangles[it->start].x, maxTriangle);
        maxTriangle = glm::max(rt::get()->mRender.mTriangles[it->start].y, maxTriangle);
        maxTriangle = glm::max(rt::get()->mRender.mTriangles[it->start].z, maxTriangle);
      }
      rt::get()->mRender.mBoundingVolumes.push_back(BoundingVolume
      {
        it->bounds.min,
        float(it->left),
        it->bounds.max,
        float(it->right),
        rt::get()->mRender.mTriangles.size() > it->start &&
        it->left == -1 &&
        it->right == -1 ?
          rt::get()->mRender.mTriangles[it->start] :
          glm::ivec4(0)
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

    std::vector<glm::ivec4> triangleCopies(rt::get()->mRender.mTriangles.size());
    for (size_t i = 0; i < triangleIndices.size(); ++i)
    {
      triangleCopies[i] = rt::get()->mRender.mTriangles[triangleIndices[i]];
    }
    std::copy_n(triangleCopies.begin(), triangleCopies.size(), rt::get()->mRender.mTriangles.begin());

    store_bvh();
    mBVHNodes.clear();
  }
}