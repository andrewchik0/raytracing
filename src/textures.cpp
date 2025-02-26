#include "textures.h"

#include <future>

#include <GL/glew.h>

#include <nfd.h>

#include <stb_image.h>
#include <stb_image_resize2.h>

#include "rt.h"

namespace raytracing
{

  textures::~textures()
  {
    unload();
  }

  void textures::load_to_memory()
  {
    for (size_t i = 0; i < mTexturesCount && i < mTextureFilenames.size(); ++i)
    {
      int w, h, channels;
      mTexturesData.push_back(stbi_load(mTextureFilenames[i].c_str(), &w, &h, &channels, 4));
    }

    int w, h, channels;
    if (float* data = stbi_loadf(rt::get()->mSkyFilename.c_str(), &w, &h, &channels, 4))
    {
      for (size_t i = 0; i < w * h * channels; ++i)
        if (data[i] > 1000.0)
          data[i] = 1000.0;
      mSkyTextureData = stbir_resize_float_linear(data, w, h, 0, nullptr, mSkyWidth, mSkyHeight, 0, STBIR_RGBA);
      stbi_image_free(data);
    }
  }

  void textures::allocate_triangles_buffer()
  {
    glGenTextures(1, &mTrianglesDataTexture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTrianglesDataTexture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32I, sMaxTexture, sMaxTexture, 1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
  }

  void textures::load_to_gpu()
  {
    int mipLevels = static_cast<int>(std::log2(std::max(mTextureWidth, mTextureHeight))) + 1;

    glGenTextures(1, &mTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureArray);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA8, mTextureWidth, mTextureHeight, mTexturesCount);

    for (size_t i = 0; i < mTexturesCount && i < mTextureFilenames.size(); ++i)
    {
      glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, mTextureWidth, mTextureHeight,
        1, GL_RGBA, GL_UNSIGNED_BYTE, mTexturesData[i]);
      stbi_image_free(mTexturesData[i]);
    }
    mTexturesData.clear();

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    glGenTextures(1, &mSky);
    glBindTexture(GL_TEXTURE_2D, mSky);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA16F, mSkyWidth, mSkyHeight);

    glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0, mSkyWidth, mSkyHeight,
        GL_RGBA, GL_FLOAT, mSkyTextureData);

    stbi_image_free(mSkyTextureData);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

  }


  void textures::unload()
  {
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    if (glIsTexture(mTextureArray))
    {
      glDeleteTextures(1, &mTextureArray);
      mTextureArray = 0;
    }
    if (glIsTexture(mSky))
    {
      glDeleteTextures(1, &mSky);
      mSky = 0;
    }
  }

  void textures::bind()
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureArray);
    rt::get()->mRender.mShader.setUniform("texArray", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mSky);
    rt::get()->mRender.mShader.setUniform("sky", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTrianglesDataTexture);
    rt::get()->mRender.mShader.setUniform("trianglesTexture", 2);
  }

  void textures::reload()
  {
    unload();
    std::thread([&]
    {
      load_to_memory();
      rt::get()->mLoading = false;
    }).detach();
  }

  void textures::load_triangles_to_gpu(std::vector<TriangleObject>& triangles)
  {
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTrianglesDataTexture);
    size_t trianglesSize = triangles.size();
    trianglesSize = (trianglesSize / sMaxTexture + 1) * sMaxTexture;
    triangles.resize(trianglesSize);
    glTexSubImage3D(
      GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
      sMaxTexture, trianglesSize / sMaxTexture, 1, GL_RGBA_INTEGER, GL_INT,
      triangles.data()
    );
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
  }

  void textures::load_from_filesystem()
  {
    nfdu8char_t* outPath;
    const nfdu8filteritem_t filters[1] = {{"Image files", "png,jpg,jpeg,hdr,psd,bmp,tga,pic"}};
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 1;
    auto defaultPath = (std::filesystem::current_path() / "assets").string();
    args.defaultPath = defaultPath.c_str();
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result != NFD_OKAY)
      return;

    add_texture(outPath);
    NFD_FreePathU8(outPath);
  }

  void textures::add_texture(const std::string& name)
  {
    if (mTextureFilenames.size() < mTexturesCount)
      mTextureFilenames.push_back(name);
  }

} // namespace raytracing
