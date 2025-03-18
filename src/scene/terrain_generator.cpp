#include "terrain_generator.h"

#include <PerlinNoise.hpp>

#include "rt.h"

namespace raytracing
{
  static std::vector<model> sGrass;
  constexpr size_t sGrassCount = 9;

  void terrain_generator::init(scene* scene)
  {
    constexpr float heightRate = .8f;
    constexpr float scale = 1.0f;

    const size_t size = scene->mTerrainOptions.size;
    const siv::PerlinNoise::seed_type seed = scene->mTerrainOptions.seed;
    const siv::PerlinNoise perlin{seed};

    std::vector<float> heightmap;
    heightmap.reserve(size * size);

    scene->mTerrain.seed = seed;
    scene->mTerrain.size = size;

    for (size_t y = 0; y < size; ++y)
    {
      for (size_t x = 0; x < size; ++x)
      {
        heightmap.emplace_back(
          powf(perlin.octave2D_01((float(x) / float(size)), (float(y) / float(size)), 8) * size, heightRate) -
          powf(size, heightRate) * 0.40f);
      }
    }
    scene->add_grid(grid(size, size, heightmap, scale, TERRAIN_MATERIAL));

    auto result = load_assets(scene);
    rt_assert(result == status::success, "Failed to load assets for terrain!");

    scene->mTriangles.emplace_back();
    scene->mVertices.emplace_back();

    auto globalTriangleIndex = scene->mTriangles.size() - 1;
    auto globalVertexIndex = scene->mVertices.size() - 1;
    size_t indexCounter = 0;
    for (size_t y = 0; y < size; ++y)
    {
      for (size_t x = 0; x < size; ++x)
      {
        if (heightmap[y * size + x] < 3)
          continue;
        const size_t random = std::rand() % sGrassCount;
        auto indices = sGrass[random].mTriangles;
        auto vertices = sGrass[random].mVertices;
        for (auto& triangle : indices)
        {
          triangle.x += indexCounter;
          triangle.y += indexCounter;
          triangle.z += indexCounter;
        }
        for (auto& vertex : vertices)
        {
          vertex.position.x += (x - size * 0.5) * scale;
          vertex.position.y += heightmap[y * size + x];
          vertex.position.z += (y - size * 0.5) * scale;
        }
        scene->mTriangles[globalTriangleIndex].insert(scene->mTriangles[globalTriangleIndex].end(), indices.begin(), indices.end());
        scene->mVertices[globalVertexIndex].insert(scene->mVertices[globalVertexIndex].end(), vertices.begin(), vertices.end());
        indexCounter += sGrass[random].mVertices.size();
      }
    }
  }

  status terrain_generator::load_assets(scene* scene)
  {
    auto result = status::success;

    scene->mTerrain.albedoIndexGrass = rt::get()->mRender.mTextures.add_texture("assets/materials/grass/albedo.png");
    scene->mTerrain.normalIndexGrass = rt::get()->mRender.mTextures.add_texture("assets/materials/grass/normal.png");
    scene->mTerrain.metallicIndexGrass = rt::get()->mRender.mTextures.add_texture("assets/materials/grass/metallic.psd");
    scene->mTerrain.albedoIndexSand = rt::get()->mRender.mTextures.add_texture("assets/materials/sand/albedo.png");
    scene->mTerrain.normalIndexSand = rt::get()->mRender.mTextures.add_texture("assets/materials/sand/normal.png");
    scene->mTerrain.metallicIndexSand = rt::get()->mRender.mTextures.add_texture("assets/materials/sand/metallic.psd");

    sGrass.resize(sGrassCount);
    result |= sGrass[0].load_from_file("assets/models/grass/small_1.gltf");
    result |= sGrass[1].load_from_file("assets/models/grass/small_2.gltf");
    result |= sGrass[2].load_from_file("assets/models/grass/small_3.gltf");
    result |= sGrass[3].load_from_file("assets/models/grass/medium_1.gltf");
    result |= sGrass[4].load_from_file("assets/models/grass/medium_2.gltf");
    result |= sGrass[5].load_from_file("assets/models/grass/medium_3.gltf");
    result |= sGrass[6].load_from_file("assets/models/grass/large_1.gltf");
    result |= sGrass[7].load_from_file("assets/models/grass/large_2.gltf");
    result |= sGrass[8].load_from_file("assets/models/grass/large_3.gltf");

    return result;
  }

} // namespace raytracing
