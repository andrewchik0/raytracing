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
    void push();

    void reload()
    {
      this->~textures();
      load();
    }

    void add_texture(const std::string& name);

  private:
    uint32_t
      mTextureWidth = 2048, mTextureHeight = 2048,
      mTexturesCount = 16;

    uint32_t mTextureArray = -1;
    std::vector<std::string> mTextureFilenames;
  };
}
