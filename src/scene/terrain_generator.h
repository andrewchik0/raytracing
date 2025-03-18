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
    [[nodiscard]] static status load_assets(scene* scene);
  };
}