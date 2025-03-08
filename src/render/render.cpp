#include "render.h"

#include "rt.h"

namespace raytracing
{
  void render::init()
  {
    glewInit();

    if (load_shaders() == status::file_not_found)
      rt_assert(false, "Failed to load shaders, files dont exist!");

    mSceneBuffer.create(SCENE_BINDING, sizeof(SceneBuffer), "SceneBuffer", mShader.get_handle());
    mGlobalDataBuffer.create(GLOBAL_DATA_BINDING, sizeof(GlobalData), "GlobalData", mShader.get_handle());
    mGlobalDataBuffer.bind_to_shader("GlobalData", mPostShader.get_handle());

    mTextures.allocate_triangles_buffer();
  }

  void render::post_init()
  {
    mTextures.load_triangles_to_gpu(mBoundingVolumes, mVertices);
    mTextures.load_to_gpu();
    mAccumulatingFrameIndex = 0;
    clear();
  }

  bool render::should_accumulate()
  {
    return mAccumulatingFrameIndex < mMaxAccumulation && (mUseSSAA || mRenderMode || !mAccumulatingFrameIndex);
  }

  void render::clear()
  {
    if (!should_accumulate())
      return;
    mLastFrameTexture.clear();
    mFinalTexture.clear();
    mBloomTexture.clear();
    mPostProcessedTexture.clear();
  }

  void render::draw(const render_texture* target /* = nullptr */)
  {
    if (!should_accumulate())
      return;
    mAccumulatingFrameIndex++;

    // Main pass
    set_uniforms();
    push_scene();
    mTextures.bind();
    mShader.dispatch_compute(mLastFrameTexture);

    mark_zone("Main pass");

    // Bloom pass
    mBloomShader.set_uniform("renderedTexture", mLastFrameTexture);
    mBloomShader.dispatch_compute(mBloomTexture);

    mark_zone("Bloom pass");

    // Post-processing pass
    mPostShader.set_uniform("renderedTexture", mLastFrameTexture);
    mPostShader.set_uniform("bloomTexture", mBloomTexture);
    mPostShader.dispatch_compute(mPostProcessedTexture);

    // Accumulation pass
    mAccumulationShader.set_uniform("lastFrameTexture", mPostProcessedTexture);
    mAccumulationShader.set_uniform("accumulatedTexture", mAccumulatedTexture);
    mAccumulationShader.set_uniform("frameIndex", mAccumulatingFrameIndex);
    mAccumulationShader.dispatch_compute(mFinalTexture);

    // Store final buffer in accumulation buffer
    mDummyShader.set_uniform("frameTexture", mFinalTexture);
    mDummyShader.dispatch_compute(mAccumulatedTexture);

    mark_zone("Post processing pass");

    if (target)
    {
      mDummyShader.dispatch_compute(*target);
    }
  }


  void render::resize(const uint32_t width, const uint32_t height)
  {
    if (width == mViewportWidth && height == mViewportHeight) return;

    mViewportWidth = width;
    mViewportHeight = height;
    auto result =
      mLastFrameTexture.resize(width, height) &&
      mBloomTexture.resize(width, height) &&
      mPostProcessedTexture.resize(width, height) &&
      mAccumulatedTexture.resize(width, height) &&
      mFinalTexture.resize(width, height);

    rt_assert(result, "Failed to resize framebuffers");

    reset_accumulation();
  }

  void render::push_scene()
  {
    SceneBuffer buffer = {};
    memcpy(buffer.planes, mPlanes.data(), sizeof(PlaneObject) * MAX_PLANES);
    memcpy(buffer.spheres, mSpheres.data(), sizeof(SphereObject) * MAX_SPHERES);
    memcpy(buffer.materials, mMaterials.data(), sizeof(Material) * MAX_MATERIALS);
    buffer.planesCount = mPlanesCount;
    buffer.spheresCount = mSpheresCount;
    mSceneBuffer.set(&buffer);
  }

  void render::reset_accumulation()
  {
    mAccumulatingFrameIndex = 0;
  }

  status render::load_shaders()
  {
    mShaderErrors.clear();

    status result[] = {
      mShader.load("./shaders/main.comp"),
      mPostShader.load("./shaders/post.comp"),
      mBloomShader.load("./shaders/bloom.comp"),
      mAccumulationShader.load("./shaders/accumulation.comp"),
      mDummyShader.load("./shaders/empty.comp"),
    };

    for (size_t i = 0; i < sizeof(result) / sizeof(status); i++)
      if (result[i] != status::success)
        return result[i];

    return status::success;
  }

  void render::set_uniforms()
  {
    GlobalData data;
    data.cameraDirection = rt::get()->mCamera.mDirection;
    data.cameraPosition = rt::get()->mCamera.mPosition;
    data.cameraUp = rt::get()->mCamera.mUp;
    data.cameraRight = rt::get()->mCamera.mRight;
    data.time = rt::get()->mTimeHandler.mTimeSinceStart;
    data.samples = mSamplesCount;
    data.bounces = mBouncesCount;
    data.halfHeight = rt::get()->mCamera.mHalfHeight;
    data.halfWidth = rt::get()->mCamera.mHalfWidth;
    data.useFXAA = mUseFXAA;
    data.gamma = mGamma;
    data.exposure = mExposure;
    data.blurSize = mBlurSize;
    data.windowSize = {mViewportWidth, mViewportHeight, 0, 0};
    data.maxTextureSize = mTextures.sMaxTextureDataSize;
    data.renderMode = mRenderMode;
    data.interpolateNormals = mInterpolateNormals;
    data.showTextures = mShowTextures;
    data.postProcessing = mPostProcessing;
    data.debugTextureLayer = mDebugTextureLayer;
    data.useSSAA = mUseSSAA;
    data.SSAAGridSize = mSSAAGridSize;
    data.accumulationIndex = mAccumulatingFrameIndex;
    mGlobalDataBuffer.set(&data);
  }
} // namespace raytracing
