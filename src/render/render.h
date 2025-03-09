#pragma once

#include "../scene/bounding_volume_builder.h"
#include "model.h"
#include "shader.h"
#include "textures.h"
#include "uniform_buffer.h"

#include "render_texture.h"

namespace raytracing
{
  class render
  {
  public:

    render() = default;
    render(const render&) = delete;

    uint32_t mViewportWidth, mViewportHeight;
    textures mTextures;

    bool mUseFXAA = true;
    bool mUseSSAA = true;
    int32_t mSSAAGridSize = 4;
    bool mRenderMode = false;
    bool mInterpolateNormals = true;
    bool mShowTextures = false;
    bool mPostProcessing = true;
    uint32_t mSamplesCount = 1;
    uint32_t mBouncesCount = 16;
    uint32_t mMaxAccumulation = 32;

    float mGamma = 1.0;
    float mExposure = 2.5;
    float mBlurSize = 5.0;

    std::string mShaderErrors;

    void init();
    void post_init();
    void clear();
    void draw(const render_texture* target = nullptr);
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

    int32_t mDebugTextureLayer = 0;

    int32_t mAccumulatingFrameIndex = 0;

    uniform_buffer mSceneBuffer, mGlobalDataBuffer;

    void set_uniforms();

    bool should_accumulate();

    status load_shaders();

    friend class rt;
    friend class gui;
    friend class serializer;
    friend class textures;
    friend class bounding_volume_builder;
  };
}
