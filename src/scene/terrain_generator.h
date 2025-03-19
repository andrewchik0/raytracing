#pragma once
#include "model.h"

namespace raytracing
{
  class scene;

  class terrain_generator
  {
  public:
    static void init(scene* scene);

  private:

    static void apply_grass(scene* scene, const std::vector<float>& heightmap, uint32_t seed, float size, float scale);
    static void apply_trees(scene* scene, const std::vector<float>& heightmap, uint32_t seed, float size, float scale);
    static void apply_rocks(scene* scene, const std::vector<float>& heightmap, uint32_t seed, float size, float scale);
    static void apply_shells(scene* scene, const std::vector<float>& heightmap, uint32_t seed, float size, float scale);

    [[nodiscard]] static status load_assets(scene* scene);

    [[nodiscard]] static float get_height(const std::vector<float>& heights, uint32_t size, float x, float z);
    [[nodiscard]] static glm::vec3 get_normal(const std::vector<float>& heights, float size, float x, float z, float scale);
    [[nodiscard]] static glm::mat3 get_normal_mat3(const std::vector<float>& heights, float size, float x, float z, float scale, float factor);
  };
}