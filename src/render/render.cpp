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
    mSceneBuffer.bind_to_shader("SceneBuffer", mNoiseGeneratorShader.get_handle());
    mBVHEntriesBuffer.create(BVH_ENTRIES_BINDING);
    mBVHBuffer.create(BVH_BINDING);
    mVerticesBuffer.create(VERTICES_BINDING);

    mNoiseTextureBuffer.resize(NOISE_WIDTH, NOISE_HEIGHT);
    mNoiseTextureBuffer.set_repeated(true);
  }

  void render::post_init()
  {
    mTextures.load_to_gpu();
    push_geometry();
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
    if (mBlurSize != 0)
    {
      mBloomTexture.clear();
    }
  }

  void render::draw(render_texture* target /* = nullptr */)
  {
    if (should_accumulate())
    {
      mAccumulatingFrameIndex++;

      // Noise generator
      if (mGenerateNoise)
      {
        mNoiseGeneratorShader.dispatch_compute(mNoiseTextureBuffer);
      }

      // Main pass
      set_uniforms();
      push_scene();
      mShader.set_uniform("noiseTexture", mNoiseTextureBuffer);
      mTextures.bind();
      mShader.dispatch_compute(mLastFrameTexture);
      mark_zone("Main pass");

      // Bloom pass
      if (mBlurSize != 0)
      {
        mBloomShader.set_uniform("renderedTexture", mLastFrameTexture);
        mBloomShader.dispatch_compute(mBloomTexture);
        mark_zone("Bloom pass");
      }


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
      mAccumulatedTexture.copy_from(mFinalTexture);
    }

    // Denoise final image
    if (mDenoise)
    {
      mDenoiseShader.set_uniform("renderedTexture", mFinalTexture);
      mDenoiseShader.dispatch_compute(mDenoisedTexture);
    }

    // Store to external buffer if needed
    if (target)
    {
      target->copy_from(mDenoise ? mDenoisedTexture : mFinalTexture);
    }

    mark_zone("Post processing pass");
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
      mDenoisedTexture.resize(width, height) &&
      mFinalTexture.resize(width, height);

    rt_assert(result, "Failed to resize framebuffers");

    reset_accumulation();
  }

  void render::push_scene()
  {
    SceneBuffer buffer = {};
    memcpy(buffer.planes, rt::get()->mScene.mPlanes.data(), sizeof(PlaneObject) * MAX_PLANES);
    memcpy(buffer.spheres, rt::get()->mScene.mSpheres.data(), sizeof(SphereObject) * MAX_SPHERES);
    memcpy(buffer.materials, rt::get()->mScene.mMaterials.data(), sizeof(Material) * MAX_MATERIALS);
    memcpy(&buffer.water, &rt::get()->mScene.mWater, sizeof(Water));
    buffer.planesCount = rt::get()->mScene.mPlanesCount;
    buffer.spheresCount = rt::get()->mScene.mSpheresCount;
    mSceneBuffer.set(&buffer);
    rt::get()->mScene.mBVHEntries.indices.insert(rt::get()->mScene.mBVHEntries.indices.begin(), rt::get()->mScene.mBVHEntries.count);
    mBVHEntriesBuffer.set(rt::get()->mScene.mBVHEntries.indices.data(), sizeof(int) * (rt::get()->mScene.mBVHEntries.indices.size()));
    rt::get()->mScene.mBVHEntries.indices.erase(rt::get()->mScene.mBVHEntries.indices.begin());
  }

  void render::push_geometry()
  {
    std::vector<Vertex> vertices;

    for (auto& modelVertices: rt::get()->mScene.mVertices)
      vertices.insert(vertices.end(), modelVertices.begin(), modelVertices.end());
    mVerticesBuffer.set(vertices.data(), vertices.size() * sizeof(Vertex));
    mBVHBuffer.set(rt::get()->mScene.mBoundingVolumes.data(), rt::get()->mScene.mBoundingVolumes.size() * sizeof(BoundingVolume));
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
      mDenoiseShader.load("./shaders/denoise.comp"),
      mNoiseGeneratorShader.load("./shaders/noise_generator.comp"),
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
    data.cameraDirection = rt::get()->mScene.mCamera.mDirection;
    data.cameraPosition = rt::get()->mScene.mCamera.mPosition;
    data.cameraUp = rt::get()->mScene.mCamera.mUp;
    data.cameraRight = rt::get()->mScene.mCamera.mRight;
    data.time = rt::get()->mTimeHandler.mTimeSinceStart;
    data.samples = mSamplesCount;
    data.bounces = mBouncesCount;
    data.halfHeight = rt::get()->mScene.mCamera.mHalfHeight;
    data.halfWidth = rt::get()->mScene.mCamera.mHalfWidth;
    data.useFXAA = mUseFXAA;
    data.gamma = mGamma;
    data.exposure = mExposure;
    data.blurSize = mBlurSize;
    data.windowSize = {mViewportWidth, mViewportHeight, 0, 0};
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
