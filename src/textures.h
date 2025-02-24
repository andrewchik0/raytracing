#pragma once

#include "pch.h"

#include <SFML/Graphics/Texture.hpp>

namespace raytracing
{
  class textures
  {
  public:
    ~textures();

    void load_from_filesystem();
    void load_to_memory();
    void unload();
    void push();

    void reload();

    void load_to_gpu();
    void add_texture(const std::string& name);

  private:
    uint32_t
      mTextureWidth = 2048, mTextureHeight = 2048,
      mTexturesCount = 32,
      mSkyWidth = 2048, mSkyHeight = 1024;

    uint32_t mTextureArray = -1;
    uint32_t mSky = 0;
    std::vector<std::string> mTextureFilenames;
    std::vector<uint8_t *> mTexturesData;
    float* mSkyTextureData;

    friend class scene_serializer;
    friend class imgui;
  };
}
