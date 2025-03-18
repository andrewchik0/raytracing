#include "terrain_generator.h"

#include <PerlinNoise.hpp>

#include "rt.h"

namespace raytracing
{
  static std::vector<model> sGrass;
  constexpr std::array<std::string_view, 9> sGrassFilenames = {
    "assets/models/grass/small_1.gltf",
    "assets/models/grass/small_2.gltf",
    "assets/models/grass/small_3.gltf",
    "assets/models/grass/medium_1.gltf",
    "assets/models/grass/medium_2.gltf",
    "assets/models/grass/medium_3.gltf",
    "assets/models/grass/large_1.gltf",
    "assets/models/grass/large_2.gltf",
    "assets/models/grass/large_3.gltf"
  };
  constexpr size_t sGrassCount = sGrassFilenames.size();

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

    const auto globalTriangleIndex = scene->mTriangles.size() - 1;
    const auto globalVertexIndex = scene->mVertices.size() - 1;
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

    const std::vector<std::pair<std::string, int&>> textures = {{
      {"assets/materials/grass/albedo.png",   scene->mTerrain.albedoIndexGrass},
      {"assets/materials/grass/normal.png",   scene->mTerrain.normalIndexGrass},
      {"assets/materials/grass/metallic.psd", scene->mTerrain.metallicIndexGrass},
      {"assets/materials/sand/albedo.png",    scene->mTerrain.albedoIndexSand},
      {"assets/materials/sand/normal.png",    scene->mTerrain.normalIndexSand},
      {"assets/materials/sand/metallic.psd",  scene->mTerrain.metallicIndexSand}
    }};

    for (auto& [path, index] : textures)
    {
      index = rt::get()->mRender.mTextures.add_texture(path);
    }

    sGrass.resize(sGrassCount);
    for (size_t i = 0; i < sGrassCount; ++i)
      result |= sGrass[i].load_from_file(sGrassFilenames[i]);

    return result;
  }

} // namespace raytracing
