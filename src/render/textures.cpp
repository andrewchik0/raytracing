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
    mTexturesData.resize(glm::min(mTexturesCountMax, (uint32_t)mTextureFilenames.size()));
    std::vector<std::future<void>> textureLoadingJobs;

    for (size_t i = 0; i < mTexturesData.size(); ++i)
    {
      size_t index = i;
      textureLoadingJobs.emplace_back(rt::get()->thread_pool().enqueue([index]
        {
          if (rt::get()->mRender.mTextures.mTextureFilenames[index].ends_with("__loaded"))
            return;
          int w, h, channels;
          if (uchar* data = stbi_load(rt::get()->mRender.mTextures.mTextureFilenames[index].c_str(), &w, &h, &channels, 4))
          {
            auto ptr = stbir_resize_uint8_linear(
              data, w, h, 0, nullptr,
              rt::get()->mRender.mTextures.mTextureWidth, rt::get()->mRender.mTextures.mTextureHeight, 0, STBIR_RGBA);
            rt_assert(ptr, "Failed to resize a texture");
            rt::get()->mRender.mTextures.mTexturesData[index] = ptr;
            stbi_image_free(data);
          }
        }
      ));
    }

    textureLoadingJobs.emplace_back(rt::get()->thread_pool().enqueue([&]
    {
      int channels;
      mSkyTextureData = stbi_loadf(rt::get()->mScene.mSkyFilename.c_str(), &mSkyWidth, &mSkyHeight, &channels, 3);
    }));

    for (auto& thread : textureLoadingJobs)
      thread.wait();
  }

  void textures::load_to_gpu()
  {
    const int mipLevels = static_cast<int>(std::log2(std::max(mTextureWidth, mTextureHeight))) + 1;

    glGenTextures(1, &mTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureArray);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA8, mTextureWidth, mTextureHeight, mTexturesCountMax);

    for (size_t i = 0; i < mTexturesCountMax && i < mTextureFilenames.size(); ++i)
    {
      if (mTexturesData[i] == nullptr) continue;
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGB16F, mSkyWidth, mSkyHeight);

    glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0, mSkyWidth, mSkyHeight,
        GL_RGB, GL_FLOAT, mSkyTextureData);

    stbi_image_free(mSkyTextureData);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    mTexturesData.clear();
    mSkyTextureData = nullptr;
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
    rt::get()->mRender.get_main_shader().use();
    size_t index = rt::get()->mRender.get_main_shader().get_free_texture_index();
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureArray);
    rt::get()->mRender.get_main_shader().set_uniform("u_texArray", index);
    glActiveTexture(GL_TEXTURE0 + index + 1);
    glBindTexture(GL_TEXTURE_2D, mSky);
    rt::get()->mRender.get_main_shader().set_uniform("u_sky", index + 1);
  }

  void textures::reload()
  {
    unload();
    load_to_memory();
  }

  void textures::load_from_filesystem()
  {
    constexpr nfdu8filteritem_t filters[1] = {{"Image files", "png,jpg,jpeg,hdr,psd,bmp,tga,pic"}};
    const auto defaultPath = (std::filesystem::current_path() / "assets").string();
    nfdopendialogu8args_t args = {nullptr};

    args.filterList = filters;
    args.filterCount = 1;
    args.defaultPath = defaultPath.c_str();

    nfdu8char_t* outPath;
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result != NFD_OKAY)
      return;

    add_texture(outPath);
    NFD_FreePathU8(outPath);
  }

  size_t textures::add_texture(const std::string& name)
  {
    for (size_t i = 0; i < mTextureFilenames.size(); i++)
      if (mTextureFilenames[i] == name)
        return i;

    if (mTextureFilenames.size() < mTexturesCountMax)
      mTextureFilenames.push_back(name);
    return mTextureFilenames.size() - 1;
  }

  size_t textures::add_texture(const std::string& name, ubyte* data)
  {
    for (size_t i = 0; i < mTextureFilenames.size(); i++)
      if (mTextureFilenames[i] == name + "__loaded")
        return i;

    if (mTextureFilenames.size() < mTexturesCountMax)
    {
      mTextureFilenames.push_back(name + "__loaded");
      mTexturesData.resize(mTextureFilenames.size());
      mTexturesData[mTextureFilenames.size() - 1] = data;
    }
    return mTextureFilenames.size() - 1;
  }

} // namespace raytracing
