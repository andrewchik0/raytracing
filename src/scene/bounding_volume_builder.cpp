#include "bounding_volume_builder.h"

#include <numeric>

#include "shaders/uniforms.h"
#include "../rt.h"

namespace raytracing
{
  void bounding_volume_builder::build_node(int32_t modelIndex, int32_t nodeIndex, std::vector<uint32_t>& triangleIndices, int32_t start, int32_t end)
  {
    BVHNode* node = &(mBVHNodes[modelIndex][nodeIndex]);
    node->start = start;
    node->count = end - start;

    auto& scene = rt::get()->mScene;

    for (size_t i = start; i < end; ++i)
    {
      glm::ivec3 tri = scene.mTriangles[modelIndex][triangleIndices[i]];
      glm::vec3 v0 = scene.mVertices[modelIndex][tri.x].position;
      glm::vec3 v1 = scene.mVertices[modelIndex][tri.y].position;
      glm::vec3 v2 = scene.mVertices[modelIndex][tri.z].position;

      node->bounds.expand(v0);
      node->bounds.expand(v1);
      node->bounds.expand(v2);
    }

    glm::vec3 boundSize = node->bounds.max - node->bounds.min;
    constexpr float epsilon = 1e-2;

    glm::vec3 adjust = glm::step(boundSize, glm::vec3(epsilon)) * epsilon;

    node->bounds.min -= adjust;
    node->bounds.max += adjust;

    if (node->count <= mObjectPerNode)
    {
      return;
    }

    glm::vec3 size = node->bounds.max - node->bounds.min;
    int32_t axis = (size.x > size.y) ? ((size.x > size.z) ? 0 : 2) : ((size.y > size.z) ? 1 : 2);

    std::unordered_map<int32_t, float> centroidMap;
    for (int32_t i = start; i < end; ++i)
    {
      glm::ivec3 tri = scene.mTriangles[modelIndex][triangleIndices[i]];
      glm::vec3 v0 = scene.mVertices[modelIndex][tri.x].position;
      glm::vec3 v1 = scene.mVertices[modelIndex][tri.y].position;
      glm::vec3 v2 = scene.mVertices[modelIndex][tri.z].position;
      centroidMap[triangleIndices[i]] = (v0[axis] + v1[axis] + v2[axis]) / 3.0f;
    }

    int32_t mid = (start + end) / 2;
    std::nth_element(
      triangleIndices.begin() + start, triangleIndices.begin() + mid, triangleIndices.begin() + end,
      [&](int32_t a, int32_t b) {
        return centroidMap[a] < centroidMap[b];
      }
    );

    int32_t leftIndex = mBVHNodes[modelIndex].size();
    mBVHNodes[modelIndex].emplace_back();
    int32_t rightIndex = mBVHNodes[modelIndex].size();
    mBVHNodes[modelIndex].emplace_back();

    mBVHNodes[modelIndex][nodeIndex].left = leftIndex;
    mBVHNodes[modelIndex][nodeIndex].right = rightIndex;

    mBVHNodes[modelIndex][nodeIndex].count = 0;
    mBVHNodes[modelIndex][nodeIndex].start = 0;

    build_node(modelIndex, leftIndex, triangleIndices, start, mid);
    build_node(modelIndex, rightIndex, triangleIndices, mid, end);
  }

  void bounding_volume_builder::store_bvh()
  {
    for (auto it = rt::get()->mScene.mBoundingVolumes.begin(); it != rt::get()->mScene.mBoundingVolumes.end(); ++it)
    {
      *it = BoundingVolume{};
    }

    int maxTriangle = 0;

    int currentIndex = 0;
    int currentTriangleIndex = 0;
    for (size_t i = 0; i < mBVHNodes.size(); ++i)
    {
      for (auto it = mBVHNodes[i].begin(); it < mBVHNodes[i].end(); ++it)
      {
        if (rt::get()->mScene.mTriangles[i].size() > it->start && it->left == -1 && it->right == -1)
        {
          maxTriangle = glm::max(rt::get()->mScene.mTriangles[i][it->start].x, maxTriangle);
          maxTriangle = glm::max(rt::get()->mScene.mTriangles[i][it->start].y, maxTriangle);
          maxTriangle = glm::max(rt::get()->mScene.mTriangles[i][it->start].z, maxTriangle);
        }
        glm::ivec4 triangle = rt::get()->mScene.mTriangles[i][it->start];
        triangle.x += currentTriangleIndex;
        triangle.y += currentTriangleIndex;
        triangle.z += currentTriangleIndex;
        rt::get()->mScene.mBoundingVolumes.push_back(BoundingVolume{
          it->bounds.min, float(it->left), it->bounds.max, float(it->right),
          rt::get()->mScene.mTriangles[i].size() > it->start && it->left == -1 && it->right == -1 ? triangle
                                                                                                  : glm::ivec4(0)});
      }
      rt::get()->mScene.mBVHEntriesCount++;
      bool applyWind = false;
      if (rt::get()->mScene.mWindAppliedMeshes.contains(i))
        applyWind = true;
      BoundingVolumeEntry entry =
      {
        ._ = glm::vec2(),
        .applyWind = applyWind,
        .index = currentIndex,
        .transform = glm::mat4(1.0f),
      };
      rt::get()->mScene.mBVHEntries.push_back(entry);
      currentIndex += mBVHNodes[i].size();
      currentTriangleIndex += rt::get()->mScene.mVertices[i].size();
    }
  }

  void bounding_volume_builder::build()
  {
    mBVHNodes.clear();
    mBVHNodes.resize(rt::get()->mScene.mTriangles.size());
    std::vector<std::future<void>> builderFutures;
    for (size_t i = 0; i < rt::get()->mScene.mTriangles.size(); ++i)
    {
      builderFutures.push_back(rt::get()->thread_pool().enqueue([&, i]
      {
        std::vector<uint32_t> triangleIndices(rt::get()->mScene.mTriangles[i].size());
        std::iota(triangleIndices.begin(), triangleIndices.end(), 0);

        mBVHNodes[i].clear();
        mBVHNodes[i].emplace_back();

        build_node(i, 0, triangleIndices, 0, rt::get()->mScene.mTriangles[i].size());

        std::vector<glm::ivec4> triangleCopies(rt::get()->mScene.mTriangles[i].size());
        for (size_t j = 0; j < triangleIndices.size(); ++j)
        {
          triangleCopies[j] = rt::get()->mScene.mTriangles[i][triangleIndices[j]];
        }
        std::copy_n(triangleCopies.begin(), triangleCopies.size(), rt::get()->mScene.mTriangles[i].begin());
      }));
    }

    for (auto& fut : builderFutures)
      fut.wait();

    store_bvh();
    mBVHNodes.clear();
  }
}