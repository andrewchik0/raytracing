#include "terrain_generator.h"

#include <PerlinNoise.hpp>

#include "rt.h"

namespace raytracing
{
  void terrain_generator::init(scene* scene)
  {
    constexpr size_t heightRate = 1.5;

    const size_t size = scene->mTerrainOptions.size;
    const siv::PerlinNoise::seed_type seed = scene->mTerrainOptions.seed;
    const siv::PerlinNoise perlin{ seed };

    std::vector<float> heightmap;
    heightmap.reserve(size * size);

    for (size_t y = 0; y < size; ++y)
    {
      for (size_t x = 0; x < size; ++x)
      {
        heightmap.emplace_back(powf(perlin.octave2D_01((float(x) / float(size)), (float(y) / float(size)), 10) * size, heightRate) - powf(size, heightRate) * 0.45f);
      }
    }
    scene->add_grid(grid(size, size, heightmap, 10, TERRAIN_MATERIAL));
  }

}