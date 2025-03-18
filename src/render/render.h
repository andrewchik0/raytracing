#pragma once

#include "shader.h"
#include "textures.h"
#include "uniform_buffer.h"

#include "render_texture.h"
#include "storage_buffer.h"

namespace raytracing
{
  class render
  {
  public:

    render() = default;
    render(const render&) = delete;

    uint32_t mViewportWidth, mViewportHeight;
    textures mTextures;

    bool mUseFXAA = false;
    bool mUseSSAA = true;
    int32_t mSSAAGridSize = 4;
    bool mRenderMode = false;
    bool mInterpolateNormals = true;
    bool mShowTextures = false;
    bool mPostProcessing = true;
    bool mDenoise = false;
    bool mGenerateNoise = true;
    uint32_t mSamplesCount = 1;
    uint32_t mBouncesCount = 16;
    uint32_t mMaxAccumulation = 32;

    float mGamma = 1.0;
    float mExposure = 2.5;
    float mBlurSize = 5.0;

    int32_t mDebugTextureLayer = 0;
    std::string mShaderErrors;

    void init();
    void post_init();
    void clear();
    void draw(render_texture* target = nullptr);
    void resize(uint32_t width, uint32_t height);

    void reset_accumulation();
    status load_shaders();

    shader& get_main_shader()
      { return mShader; }
    render_texture& get_final_texture()
      { return mDenoise ? mDenoisedTexture : mFinalTexture; }

  private:
    shader
      mShader,
      mPostShader,
      mBloomShader,
      mAccumulationShader,
      mDenoiseShader,
      mNoiseGeneratorShader,
      mDummyShader;
    render_texture
      mLastFrameTexture,
      mBloomTexture,
      mPostProcessedTexture,
      mAccumulatedTexture,
      mDenoisedTexture,
      mNoiseTextureBuffer,
      mFinalTexture;

    int32_t mAccumulatingFrameIndex = 0;

    uniform_buffer mSceneBuffer, mGlobalDataBuffer;
    storage_buffer mBVHEntriesBuffer, mBVHBuffer, mVerticesBuffer;

    void push_scene();
    void push_geometry();
    void set_uniforms();

    [[nodiscard]] bool should_accumulate();
  };
}
