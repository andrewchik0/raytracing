#include "textures.h"

#include <GL/glew.h>
#include <nfd.h>

#include "rt.h"

namespace raytracing
{

  textures::~textures()
  {
    unload();
  }

  void textures::load()
  {
    glGenTextures(1, &mTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureArray);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, mTextureWidth, mTextureWidth, mTexturesCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    for (size_t i = 0; i < mTexturesCount && i < mTextureFilenames.size(); ++i)
    {
      sf::Texture texture;
      texture.loadFromFile(mTextureFilenames[i]);
      texture.setSmooth(true);
      texture.setRepeated(true);
      texture.resize({mTextureWidth, mTextureHeight});
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, mTextureWidth, mTextureHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                      texture.copyToImage().getPixelsPtr());
    }
  }
  void textures::unload()
  {
    if (glIsTexture(mTextureArray))
    {
      glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
      glDeleteTextures(1, &mTextureArray);
      mTextureArray = 0;
    }
  }

  void textures::push()
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureArray);
    rt::get()->mRender.mShader.setUniform("texArray", 0);
  }

  void textures::reload()
  {
    unload();
    load();
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
