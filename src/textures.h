#pragma once

#include "pch.h"

#include <SFML/Graphics/Texture.hpp>

#include "../shaders/uniforms.h"

namespace raytracing
{
  class textures
  {
  public:
    static constexpr size_t
      sMaxTextureDataSize = 4096,
      sMaxTriangles = sMaxTextureDataSize * sMaxTextureDataSize,
      sMaxBoundingVolumes = sMaxTextureDataSize * sMaxTextureDataSize,
      sMaxVertices = sMaxTextureDataSize * sMaxTextureDataSize;

    ~textures();

    void bind();

    void reload();
    void unload();

    void load_triangles_to_gpu(std::vector<TriangleObject>& triangles, std::vector<BoundingVolume>& bounds, std::vector<Vertex>& vertices);
    void load_to_gpu();

    size_t add_texture(const std::string& name);
    void load_from_filesystem();

    void allocate_triangles_buffer();

  private:

    uint32_t
      mTextureWidth = 2048, mTextureHeight = 2048,
      mTexturesCountMax = 256,
      mSkyWidth = 2048, mSkyHeight = 1024;

    // Textures are stored in arrays because of conflicts with SFML
    uint32_t mTextureArray = 0;              // Usual material textures
    uint32_t mSky = 0;                       // Sky texture
    uint32_t mTrianglesDataTexture = 0;      // Triangles data encoded into a texture
    uint32_t mVerticesDataTexture = 0;       // Vertices data encoded into a texture
    uint32_t mBoundingVolumesTexture = 0;    // Bounding volumes data encoded into a texture

    std::vector<std::string> mTextureFilenames;
    std::vector<uint8_t *> mTexturesData;
    float* mSkyTextureData;

    void load_to_memory();

    friend class scene_serializer;
    friend class gui;
  };
}
