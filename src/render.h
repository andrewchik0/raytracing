#pragma once

#include "bounding_volume_builder.h"
#include "model.h"
#include "textures.h"
#include "uniform_buffer.h"

#include <set>

#include <SFML/Graphics.hpp>


namespace raytracing
{
  enum class status
  {
    success = 0,
    error = 1,
    file_not_found = 2,
  };

  struct object_additional
  {
    std::string name;
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
      mShader, mPostShader, mBloomShader;
    sf::RectangleShape mRenderQuad;
    sf::RenderTexture mBloomTexture;
    sf::RenderTexture mTexture;

    bool mUseFXAA = true;
    uint32_t mSamplesCount = 1;
    uint32_t mBouncesCount = 2;
    glm::vec3 mLightDirection = glm::vec3(0.8, 1.0, 1.0);

    float mGamma = 1.0;
    float mExposure = 2.5;
    float mBlurSize = 5.0;

    std::array<SphereObject, MAX_SPHERES> mSpheres;
    std::array<PlaneObject, MAX_PLANES> mPlanes;
    std::array<TriangleObject, MAX_TRIANGLES> mTriangles;
    std::array<Vertex, MAX_VERTICES> mVertices;
    std::array<Material, MAX_MATERIALS> mMaterials;
    std::array<BoundingVolume, MAX_BOUNDING_VOLUMES> mBoundingVolumes;
    size_t
      mSpheresCount = 0,
      mPlanesCount = 0,
      mTrianglesCount = 0,
      mBoundingVolumesCount = 0,
      mVertexCount = 0,
      mMaterialsCount = 0;

    // Additional data stored separately in order to easily pass object data to the shader
    std::array<object_additional, MAX_SPHERES> mSpheresAdditional;
    std::array<object_additional, MAX_PLANES> mPlanesAdditional;
    std::array<object_additional, MAX_TRIANGLES> mTrianglesAdditional;
    std::array<object_additional, MAX_MATERIALS> mMaterialsAdditional;

    uniform_buffer mSceneBuffer;
    textures mTextures;
    bounding_volume_builder mBoundingVolumeBuilder;

    void set_uniforms();

    status load_shaders();

    static status load_shader(sf::Shader* shader, const std::string& vertexPath, const std::string& fragmentPath);

    static std::string read_shader_file(const std::string& path);
    static std::string parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles);

    friend class rt;
    friend class imgui;
    friend class scene_serializer;
    friend class textures;
    friend class bounding_volume_builder;
  };
}
