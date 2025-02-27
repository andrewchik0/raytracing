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
    uint32_t mViewportWidth, mViewportHeight;
    textures mTextures;

    bool mUseFXAA = true;
    bool mRenderMode = false;
    bool mInterpolateNormals = true;
    uint32_t mSamplesCount = 1;
    uint32_t mBouncesCount = 16;

    float mGamma = 1.0;
    float mExposure = 2.5;
    float mBlurSize = 5.0;

    std::array<SphereObject, MAX_SPHERES> mSpheres;
    std::array<PlaneObject, MAX_PLANES> mPlanes;
    std::array<Material, MAX_MATERIALS> mMaterials;
    size_t
      mSpheresCount = 0,
      mPlanesCount = 0,
      mMaterialsCount = 0;

    std::string mShaderErrors;

    void init();
    void post_init();
    void clear();
    void draw(sf::RenderTarget* target);
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


    std::vector<Vertex> mVertices;
    std::vector<TriangleObject> mTriangles;
    std::vector<BoundingVolume> mBoundingVolumes;

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
