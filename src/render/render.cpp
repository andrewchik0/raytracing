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
    if (rt::get()->mScene.mMaterialsCount == 0)
    {
      Material mat;
      mat.albedo = glm::vec3(0.5, 0.5, 0.5);
      mat.roughness = 0.8;
      rt::get()->mScene.add_material("Default material", mat);
    }
    mTextures.load_to_gpu();
    push_geometry();
    reset_accumulation();
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
        mark_zone("Noise generation");
      }

      // Main pass
      set_uniforms();
      push_scene();
      mShader.set_uniform("u_noiseTexture", mNoiseTextureBuffer);
      mTextures.bind();
      mShader.dispatch_compute(mLastFrameTexture);
      mark_zone("Main pass");

      // Bloom pass
      if (mBlurSize != 0)
      {
        mBloomShader.set_uniform("u_renderedTexture", mLastFrameTexture);
        mBloomShader.dispatch_compute(mBloomTexture);
        mark_zone("Bloom pass");
      }

      // Post-processing pass
      mPostShader.set_uniform("u_renderedTexture", mLastFrameTexture);
      mPostShader.set_uniform("u_bloomTexture", mBloomTexture);
      mPostShader.dispatch_compute(mPostProcessedTexture);

      // Accumulation pass
      mAccumulationShader.set_uniform("u_lastFrameTexture", mPostProcessedTexture);
      mAccumulationShader.set_uniform("u_accumulatedTexture", mAccumulatedTexture);
      mAccumulationShader.set_uniform("u_frameIndex", mAccumulatingFrameIndex);
      mAccumulationShader.dispatch_compute(mFinalTexture);

      // Store final buffer in accumulation buffer
      mAccumulatedTexture.copy_from(mFinalTexture);
    }

    // Denoise final image
    if (mDenoise)
    {
      mDenoiseShader.set_uniform("u_renderedTexture", mFinalTexture);
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
    memcpy(buffer.u_planes, rt::get()->mScene.mPlanes.data(), sizeof(PlaneObject) * MAX_PLANES);
    memcpy(buffer.u_spheres, rt::get()->mScene.mSpheres.data(), sizeof(SphereObject) * MAX_SPHERES);
    memcpy(buffer.u_materials, rt::get()->mScene.mMaterials.data(), sizeof(Material) * MAX_MATERIALS);
    memcpy(&buffer.u_water, &rt::get()->mScene.mWater, sizeof(Water));
    memcpy(&buffer.u_mandelbulb, &rt::get()->mScene.mMandelbulb, sizeof(Mandelbulb));
    memcpy(&buffer.u_terrain, &rt::get()->mScene.mTerrain, sizeof(Terrain));
    buffer.u_planesCount = rt::get()->mScene.mPlanesCount;
    buffer.u_spheresCount = rt::get()->mScene.mSpheresCount;
    buffer.u_bvhEntriesCount = rt::get()->mScene.mBVHEntriesCount;
    mSceneBuffer.set(&buffer);
    mBVHEntriesBuffer.set(rt::get()->mScene.mBVHEntries.data(), sizeof(BoundingVolumeEntry) * (rt::get()->mScene.mBVHEntries.size()));
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
    data.u_cameraDirection = rt::get()->mScene.mCamera.mDirection;
    data.u_cameraPosition = rt::get()->mScene.mCamera.mPosition;
    data.u_cameraUp = rt::get()->mScene.mCamera.mUp;
    data.u_cameraRight = rt::get()->mScene.mCamera.mRight;
    data.u_time = rt::get()->mTimeHandler.mTimeSinceStart;
    data.u_samples = mSamplesCount;
    data.u_bounces = mBouncesCount;
    data.u_halfHeight = rt::get()->mScene.mCamera.mHalfHeight;
    data.u_halfWidth = rt::get()->mScene.mCamera.mHalfWidth;
    data.u_useFXAA = mUseFXAA;
    data.u_gamma = mGamma;
    data.u_exposure = mExposure;
    data.u_blurSize = mBlurSize;
    data.u_windowSize = {mViewportWidth, mViewportHeight, 0, 0};
    data.u_renderMode = mRenderMode;
    data.u_interpolateNormals = mInterpolateNormals;
    data.u_showTextures = mShowTextures;
    data.u_postProcessing = mPostProcessing;
    data.u_debugTextureLayer = mDebugTextureLayer;
    data.u_useSSAA = mUseSSAA;
    data.u_SSAAGridSize = mSSAAGridSize;
    data.u_accumulationIndex = mAccumulatingFrameIndex;
    mGlobalDataBuffer.set(&data);
  }
} // namespace raytracing
