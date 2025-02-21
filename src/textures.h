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

    void add_texture(const std::string& name);

  private:
    uint32_t
      mTextureWidth = 2048, mTextureHeight = 2048,
      mTexturesCount = 16;

    uint32_t mTextureArray = -1;
    std::vector<std::string> mTextureFilenames;

    friend class scene_serializer;
  };
}
