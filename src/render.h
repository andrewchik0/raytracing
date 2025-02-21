#pragma once

#include "textures.h"


#include <set>

#include <SFML/Graphics.hpp>

#include "uniform_buffer.h"
#include "../shaders/uniforms.h"

namespace raytracing
{
  enum class status
  {
    success = 0,
    error = 1,
    file_not_found = 2,
  };

  class render
  {
  public:
    void init();
    void clear();
    void draw(sf::RenderTarget* window);
    void resize(uint32_t width, uint32_t height);

    void push_scene();

  private:
    sf::Shader
      mShader, mPostShader;
    sf::RectangleShape mRenderQuad;
    sf::RenderTexture mTexture;
    sf::Texture mSkyTexture;

    bool mUseFXAA = true;
    uint32_t mSamplesCount = 12;
    uint32_t mBouncesCount = 6;
    glm::vec3 mLightDirection = glm::vec3(0.8, 1.0, 1.0);

    std::array<SphereObject, MAX_SPHERES> mSpheres;
    std::array<PlaneObject, MAX_PLANES> mPlanes;
    std::array<Material, MAX_MATERIALS> mMaterials;
    size_t
      mSpheresCount = 0,
      mPlanesCount = 0,
      mMaterialsCount = 1;

    uniform_buffer mSceneBuffer;
    textures mTextures;

    void set_uniforms();

    status load_shaders();

    static status load_shader(sf::Shader* shader, const std::string& vertexPath, const std::string& fragmentPath);

    static std::string read_shader_file(const std::string& path);
    static std::string parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles);

    friend class rt;
    friend class imgui;
    friend class scene_serializer;
    friend class textures;
  };
}
