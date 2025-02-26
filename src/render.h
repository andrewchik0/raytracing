#pragma once

#include "bounding_volume_builder.h"
#include "model.h"
#include "textures.h"
#include "uniform_buffer.h"
#include "shader.h"

#include <SFML/Graphics.hpp>

namespace raytracing
{
  struct object_additional
  {
    std::string name;
  };

  class render
  {
  public:
    textures mTextures;

    bool mUseFXAA = true;
    uint32_t mSamplesCount = 1;
    uint32_t mBouncesCount = 2;

    float mGamma = 1.0;
    float mExposure = 2.5;
    float mBlurSize = 5.0;

    void init();
    void post_init();
    void clear();
    void draw(sf::RenderTarget* window);
    void resize(uint32_t width, uint32_t height);

    void push_scene();

    void reset_accumulation();

  private:
    sf::RectangleShape mRenderQuad;
    shader
      mShader,
      mPostShader,
      mBloomShader,
      mAccumulationShader,
      mDummyShader;
    sf::RenderTexture
      mLastFrameTexture,
      mBloomTexture,
      mPostProcessedTexture,
      mAccumulatedTexture,
      mFinalTexture;

    int32_t mAccumulatingFrameIndex = 0;

    std::array<SphereObject, MAX_SPHERES> mSpheres;
    std::array<PlaneObject, MAX_PLANES> mPlanes;
    std::array<Vertex, MAX_VERTICES> mVertices;
    std::array<Material, MAX_MATERIALS> mMaterials;
    std::vector<TriangleObject> mTriangles;
    std::array<BoundingVolume, MAX_BOUNDING_VOLUMES> mBoundingVolumes;
    size_t
      mSpheresCount = 0,
      mPlanesCount = 0,
      mBoundingVolumesCount = 0,
      mVertexCount = 0,
      mMaterialsCount = 0;

    // Additional data stored separately in order to easily pass object data to the shader
    std::array<object_additional, MAX_SPHERES> mSpheresAdditional;
    std::array<object_additional, MAX_PLANES> mPlanesAdditional;
    std::array<object_additional, MAX_MATERIALS> mMaterialsAdditional;

    uniform_buffer mSceneBuffer, mGlobalDataBuffer;
    bounding_volume_builder mBoundingVolumeBuilder;

    void set_uniforms();

    status load_shaders();

    friend class rt;
    friend class gui;
    friend class scene_serializer;
    friend class textures;
    friend class bounding_volume_builder;
  };
}
