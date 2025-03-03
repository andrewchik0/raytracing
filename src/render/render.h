#pragma once

#include "uniform_buffer.h"
#include "bounding_volume_builder.h"
#include "model.h"
#include "shader.h"
#include "textures.h"

#include "render_texture.h"

namespace raytracing
{
  struct object_additional
  {
    std::string name;
  };

  class render
  {
  public:

    render() = default;
    render(const render&) = delete;

    uint32_t mViewportWidth, mViewportHeight;
    textures mTextures;

    bool mUseFXAA = true;
    bool mRenderMode = false;
    bool mInterpolateNormals = true;
    uint32_t mSamplesCount = 1;
    uint32_t mBouncesCount = 16;
    uint32_t mMaxAccumulation = 32;

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
    void draw(render_texture* target);
    void resize(uint32_t width, uint32_t height);

    void push_scene();

    void reset_accumulation();

  private:
    shader
      mShader,
      mPostShader,
      mBloomShader,
      mAccumulationShader,
      mDummyShader;
    render_texture
      mLastFrameTexture,
      mBloomTexture,
      mPostProcessedTexture,
      mAccumulatedTexture,
      mFinalTexture;

    int32_t mAccumulatingFrameIndex = 0;

    std::vector<Vertex> mVertices;
    std::vector<ivec4> mTriangles;
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
