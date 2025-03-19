#include "terrain_generator.h"

#include <PerlinNoise.hpp>

#include "glm/ext/matrix_transform.hpp"
#include "rt.h"

namespace raytracing
{
  static std::vector<model> sGrass, sRocks, sTrees;
  static model sShell;

  constexpr std::string_view sShellFilename = "assets/models/shell/shell.gltf";

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

  constexpr std::array<std::string_view, 11> sRockFilenames = {
    "assets/models/rocks/Rock0.fbx",
    "assets/models/rocks/Rock1.fbx",
    "assets/models/rocks/Rock2.fbx",
    "assets/models/rocks/Rock3.fbx",
    "assets/models/rocks/Rock4.fbx",
    "assets/models/rocks/Rock5.fbx",
    "assets/models/rocks/Rock6.fbx",
    "assets/models/rocks/Rock7.fbx",
    "assets/models/rocks/Rock8.fbx",
    "assets/models/rocks/Rock9.fbx",
    "assets/models/rocks/Rock10.fbx"
  };
  constexpr size_t sRocksCount = sRockFilenames.size();

  constexpr std::array<std::string_view, 4> sTreeFilenames = {
    "assets/models/trees/tree1.gltf",
    "assets/models/trees/tree2.gltf",
    "assets/models/trees/tree3.gltf",
    "assets/models/trees/tree4.gltf",
  };
  constexpr size_t sTreesCount = sTreeFilenames.size();

  float terrain_generator::get_height(const std::vector<float>& heights, const uint32_t size, float x, float z)
  {
    x = glm::max(0.0f, x);
    z = glm::max(0.0f, z);
    glm::ivec2 i = glm::floor(glm::vec2(x, z));
    glm::vec2 f = glm::fract(glm::vec2(x, z));

    glm::ivec2 uv00 = glm::min((i + glm::ivec2(0.0, 0.0)), glm::ivec2(size - 1));
    glm::ivec2 uv10 = glm::min((i + glm::ivec2(1.0, 0.0)), glm::ivec2(size - 1));
    glm::ivec2 uv01 = glm::min((i + glm::ivec2(0.0, 1.0)), glm::ivec2(size - 1));
    glm::ivec2 uv11 = glm::min((i + glm::ivec2(1.0, 1.0)), glm::ivec2(size - 1));

    float h00 = heights[uv00.x + uv00.y * size];
    float h10 = heights[uv10.x + uv10.y * size];
    float h01 = heights[uv01.x + uv01.y * size];
    float h11 = heights[uv11.x + uv11.y * size];

    float h0 = glm::mix(h00, h10, f.x);
    float h1 = glm::mix(h01, h11, f.x);
    return glm::mix(h0, h1, f.y);
  }

  glm::vec3 terrain_generator::get_normal(const std::vector<float>& heights, float size, const float x, const float z, const float scale)
  {
    const auto uv = glm::vec2(x, z);

    glm::vec2 uvL = uv - glm::vec2(1.0, 0.0);
    glm::vec2 uvR = uv + glm::vec2(1.0, 0.0);
    glm::vec2 uvD = uv - glm::vec2(0.0, 1.0);
    glm::vec2 uvU = uv + glm::vec2(0.0, 1.0);

    float hL = get_height(heights, size, uvL.x, uvL.y);
    float hR = get_height(heights, size, uvR.x, uvR.y);
    float hD = get_height(heights, size, uvD.x, uvD.y);
    float hU = get_height(heights, size, uvU.x, uvU.y);

    return glm::normalize(glm::vec3(hL - hR, 2.0f / scale, hD - hU));
  }

  glm::mat3 terrain_generator::get_normal_mat3(const std::vector<float>& heights, float size, const float x,
                                               const float z, const float scale, const float factor)
  {
    const auto surfaceNormal = get_normal(heights, size, x, z, scale);
    constexpr auto perfectNormal = glm::vec3(0, 1, 0);
    const auto normal = glm::mix(perfectNormal, surfaceNormal, factor);
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 up = glm::vec3(0, 1, 0);

    glm::vec3 tangent = glm::normalize(glm::cross(up, n));
    if (length(tangent) < 1e-5)
      tangent = glm::vec3(1, 0, 0);

    glm::vec3 bitangent = normalize(cross(n, tangent));

    return glm::mat3(tangent, n, bitangent);
  }

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

    apply_grass(scene, heightmap, seed, size, scale);
    apply_trees(scene, heightmap, seed, size, scale);
    apply_rocks(scene, heightmap, seed, size, scale);
    apply_shells(scene, heightmap, seed, size, scale);
  }

  void terrain_generator::apply_grass(scene* scene, const std::vector<float>& heightmap, const uint32_t seed,
                                      const float size,
                                      const float scale)
  {
    scene->mTriangles.emplace_back();
    scene->mVertices.emplace_back();

    const auto globalTriangleIndex = scene->mTriangles.size() - 1;
    const auto globalVertexIndex = scene->mVertices.size() - 1;
    size_t indexCounter = 0;
    srand(seed);

    auto posDistance = [&](const size_t x, const size_t z) -> float
    {
      float xWorld = x - size / 2;
      float zWorld = z - size / 2;
      return glm::distance(scene->mCamera.mPosition, glm::vec3(xWorld, 0, zWorld));
    };

    for (size_t y = 0; y < size; y += 5)
    {
      for (size_t x = 0; x < size; x += 5)
      {
        if (heightmap[y * size + x] < 3)
          continue;
        uint32_t grassDensity = pow(100.0 / posDistance(x, y), 2.);

        for (size_t z = 0; z < grassDensity; ++z)
        {
          const size_t random = rand() % sTreesCount;
          const auto randomOffsetX = rand() / (float)RAND_MAX * 10.0f - 5.f;
          const auto randomOffsetZ = rand() / (float)RAND_MAX * 10.0f - 5.f;
          const auto randomNormalFactor = rand() / (float)RAND_MAX * 0.7 + 0.3f;
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
            vertex.position = glm::vec4(get_normal_mat3(heightmap, size, x + randomOffsetX, y + randomOffsetZ, scale, randomNormalFactor) * glm::vec3(vertex.position), 1.0);

            vertex.position.x += (x - size * 0.5 + scale * randomOffsetX) * scale;
            vertex.position.y += get_height(heightmap, size, x + randomOffsetX, y + randomOffsetZ);
            vertex.position.z += (y - size * 0.5 + scale * randomOffsetZ) * scale;
          }
          scene->mTriangles[globalTriangleIndex].insert(scene->mTriangles[globalTriangleIndex].end(), indices.begin(), indices.end());
          scene->mVertices[globalVertexIndex].insert(scene->mVertices[globalVertexIndex].end(), vertices.begin(), vertices.end());
          indexCounter += sGrass[random].mVertices.size();
        }
      }
    }
    scene->mWindAppliedMeshes[globalTriangleIndex] = true;
  }

  void terrain_generator::apply_trees(scene* scene, const std::vector<float>& heightmap, const uint32_t seed,
                                      const float size,
                                      const float scale)
  {
    scene->mTriangles.emplace_back();
    scene->mVertices.emplace_back();

    const auto globalTriangleIndex = scene->mTriangles.size() - 1;
    const auto globalVertexIndex = scene->mVertices.size() - 1;
    size_t indexCounter = 0;
    srand(seed);

    auto posDistance = [&](const size_t x, const size_t z) -> float
    {
      float xWorld = x - size / 2;
      float zWorld = z - size / 2;
      return glm::distance(scene->mCamera.mPosition, glm::vec3(xWorld, 0, zWorld));
    };

    for (size_t y = 0; y < size; y += 5)
    {
      for (size_t x = 0; x < size; x += 5)
      {
        if (heightmap[y * size + x] < 3)
          continue;
        uint32_t treeDensity = glm::min(posDistance(x, y) * .01f, 30.0f);

        for (size_t z = 0; z < treeDensity; ++z)
        {
          const size_t random = rand() % sTreesCount;
          const auto randomOffsetX = rand() / (float)RAND_MAX * 10.0f - 5.f;
          const auto randomOffsetZ = rand() / (float)RAND_MAX * 10.0f - 5.f;
          const auto randomNormalFactor = rand() / (float)RAND_MAX * 0.5;
          auto indices = sTrees[random].mTriangles;
          auto vertices = sTrees[random].mVertices;
          for (auto& triangle : indices)
          {
            triangle.x += indexCounter;
            triangle.y += indexCounter;
            triangle.z += indexCounter;
          }
          for (auto& vertex : vertices)
          {
            vertex.position = glm::scale(glm::mat4(1.0f), glm::vec3(0.2)) * glm::vec4(get_normal_mat3(heightmap, size, x + randomOffsetX, y + randomOffsetZ, scale, randomNormalFactor) * glm::vec3(vertex.position), 1.0);

            vertex.position.x += (x - size * 0.5 + scale * randomOffsetX) * scale;
            vertex.position.y += get_height(heightmap, size, x + randomOffsetX, y + randomOffsetZ);
            vertex.position.z += (y - size * 0.5 + scale * randomOffsetZ) * scale;
          }
          scene->mTriangles[globalTriangleIndex].insert(scene->mTriangles[globalTriangleIndex].end(), indices.begin(), indices.end());
          scene->mVertices[globalVertexIndex].insert(scene->mVertices[globalVertexIndex].end(), vertices.begin(), vertices.end());
          indexCounter += sTrees[random].mVertices.size();
        }
      }
    }
    scene->mWindAppliedMeshes[globalTriangleIndex] = true;
  }

  void terrain_generator::apply_rocks(scene* scene, const std::vector<float>& heightmap, const uint32_t seed,
                                      const float size,
                                      const float scale)
  {
    scene->mTriangles.emplace_back();
    scene->mVertices.emplace_back();

    const auto globalTriangleIndex = scene->mTriangles.size() - 1;
    const auto globalVertexIndex = scene->mVertices.size() - 1;
    size_t indexCounter = 0;
    srand(seed);
    for (size_t y = 0; y < size; y += 10)
    {
      for (size_t x = 0; x < size; x += 10)
      {
        if (heightmap[y * size + x] < 1)
          continue;

        const size_t random = rand() % sRocksCount;
        const auto randomOffsetX = rand() / (float)RAND_MAX * 4.0f - 2.0f;
        const auto randomOffsetZ = rand() / (float)RAND_MAX * 4.0f - 2.0f;
        const auto randomNormalFactor = rand() / (float)RAND_MAX * 0.7;
        auto indices = sRocks[random].mTriangles;
        auto vertices = sRocks[random].mVertices;
        for (auto& triangle : indices)
        {
          triangle.x += indexCounter;
          triangle.y += indexCounter;
          triangle.z += indexCounter;
        }
        for (auto& vertex : vertices)
        {
          vertex.position = glm::scale(glm::mat4(1.0f), glm::vec3(0.001)) * glm::vec4(get_normal_mat3(heightmap, size, x + randomOffsetX, y + randomOffsetZ, scale, randomNormalFactor) * glm::vec3(vertex.position), 1.0);

          vertex.position.x += (x - size * 0.5 + scale * randomOffsetX) * scale;
          vertex.position.y += get_height(heightmap, size, x + randomOffsetX, y + randomOffsetZ);
          vertex.position.z += (y - size * 0.5 + scale * randomOffsetZ) * scale;
        }
        scene->mTriangles[globalTriangleIndex].insert(scene->mTriangles[globalTriangleIndex].end(), indices.begin(), indices.end());
        scene->mVertices[globalVertexIndex].insert(scene->mVertices[globalVertexIndex].end(), vertices.begin(), vertices.end());
        indexCounter += sRocks[random].mVertices.size();
      }
    }
  }

 void terrain_generator::apply_shells(scene* scene, const std::vector<float>& heightmap, const uint32_t seed,
                                      const float size,
                                      const float scale)
  {
    scene->mTriangles.emplace_back();
    scene->mVertices.emplace_back();

    const auto globalTriangleIndex = scene->mTriangles.size() - 1;
    const auto globalVertexIndex = scene->mVertices.size() - 1;
    size_t indexCounter = 0;
    srand(seed);
    for (size_t y = 0; y < size; y += 4)
    {
      for (size_t x = 0; x < size; x += 4)
      {
        if (heightmap[y * size + x] < -1 || heightmap[y * size + x] > 2.5)
          continue;

        const auto randomOffsetX = rand() / (float)RAND_MAX * 2.0f - 1.0f;
        const auto randomOffsetZ = rand() / (float)RAND_MAX * 2.0f - 1.0f;
        const auto randomNormalFactor = rand() / (float)RAND_MAX * 0.7;
        auto indices = sShell.mTriangles;
        auto vertices = sShell.mVertices;
        for (auto& triangle : indices)
        {
          triangle.x += indexCounter;
          triangle.y += indexCounter;
          triangle.z += indexCounter;
        }
        for (auto& vertex : vertices)
        {
          vertex.position = glm::scale(glm::mat4(1.0f), glm::vec3(0.05)) * glm::vec4(get_normal_mat3(heightmap, size, x + randomOffsetX, y + randomOffsetZ, scale, randomNormalFactor) * glm::vec3(vertex.position), 1.0);

          vertex.position.x += (x - size * 0.5 + scale * randomOffsetX) * scale;
          vertex.position.y += get_height(heightmap, size, x + randomOffsetX, y + randomOffsetZ);
          vertex.position.z += (y - size * 0.5 + scale * randomOffsetZ) * scale;
        }
        scene->mTriangles[globalTriangleIndex].insert(scene->mTriangles[globalTriangleIndex].end(), indices.begin(), indices.end());
        scene->mVertices[globalVertexIndex].insert(scene->mVertices[globalVertexIndex].end(), vertices.begin(), vertices.end());
        indexCounter += sShell.mVertices.size();
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

    sTrees.resize(sTreesCount);
    for (size_t i = 0; i < sTreesCount; ++i)
      result |= sTrees[i].load_from_file(sTreeFilenames[i]);

    sRocks.resize(sRocksCount);
    for (size_t i = 0; i < sRocksCount; ++i)
      result |= sRocks[i].load_from_file(sRockFilenames[i]);

    result |= sShell.load_from_file(sShellFilename);

    return result;
  }

} // namespace raytracing
