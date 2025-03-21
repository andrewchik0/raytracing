#pragma once

#include "pch.h"

#include "shaders/uniforms.h"

namespace raytracing
{
  class textures
  {
  public:

    textures() = default;
    textures(const textures&) = delete;

    ~textures();

    void bind();

    void reload();
    void unload();

    void load_to_gpu();

    size_t add_texture(const std::string& name);
    size_t add_texture(const std::string& name, ubyte* data);
    void load_from_filesystem();

  private:

    uint32_t
      mTextureWidth = 2048, mTextureHeight = 2048,
      mTexturesCountMax = 64;

    int32_t mSkyWidth, mSkyHeight;

    uint32_t mTextureArray = 0;              // Usual material textures
    uint32_t mSky = 0;                       // Sky texture

    std::vector<std::string> mTextureFilenames;
    std::vector<uchar *> mTexturesData;
    float* mSkyTextureData;

    void load_to_memory();

    friend class serializer;
    friend class gui;
  };
}
