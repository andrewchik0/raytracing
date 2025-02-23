#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <cstdint>

namespace raytracing
{
  class textures
  {
  public:
    ~textures();

    void load();
    void unload();
    void push();

    void reload();

    void load_from_filesystem();

    void add_texture(const std::string& name);

  private:
    uint32_t
      mTextureWidth = 2048, mTextureHeight = 2048,
      mTexturesCount = 32;

    uint32_t mTextureArray = -1;
    uint32_t mSky = 0;
    std::vector<std::string> mTextureFilenames;

    friend class scene_serializer;
    friend class imgui;
  };
}
