#include "render.h"

#include "rt.h"

namespace raytracing
{
  void render::init()
  {
    glewInit();

    rt_assert(load_shaders() == status::success, "Failed to load shaders");

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

  void render::clear()
  {
    if (mAccumulatingFrameIndex > mMaxAccumulation || (!mRenderMode && mAccumulatingFrameIndex)) return;
    mLastFrameTexture.clear();
    mFinalTexture.clear();
    mBloomTexture.clear();
    mPostProcessedTexture.clear();
  }

  void render::draw(render_texture* target)
  {
    if (mAccumulatingFrameIndex > mMaxAccumulation || (!mRenderMode && mAccumulatingFrameIndex)) return;
    mAccumulatingFrameIndex++;

    // Main pass
    set_uniforms();
    push_scene();
    mTextures.bind();
    mLastFrameTexture.draw(mShader);

    // Bloom pass
    mBloomShader.set_uniform("renderedTexture", mLastFrameTexture);
    mBloomTexture.draw(mBloomShader);

    // Post-processing pass
    mPostShader.set_uniform("renderedTexture", mLastFrameTexture);
    mPostShader.set_uniform("bloomTexture", mBloomTexture);
    mPostProcessedTexture.draw(mPostShader);

    glBindTexture(GL_TEXTURE_2D, mPostProcessedTexture.get_handle());
    std::vector<unsigned char> pixels(mViewportWidth * mViewportHeight * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Accumulation pass
    mAccumulationShader.set_uniform("lastFrameTexture", mPostProcessedTexture);
    mAccumulationShader.set_uniform("accumulatedTexture", mAccumulatedTexture);
    mAccumulationShader.set_uniform("frameIndex", mAccumulatingFrameIndex);
    mFinalTexture.draw(mAccumulationShader);

    // Store final buffer in accumulation buffer
    mDummyShader.set_uniform("frameTexture", mFinalTexture);
    mAccumulatedTexture.draw(mDummyShader);

    if (target)
    {
      target->draw(mDummyShader);
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
      mShader.load("./shaders/quad.vert", "./shaders/main.frag"),
      mPostShader.load("./shaders/quad.vert", "./shaders/post.frag"),
      mBloomShader.load("./shaders/quad.vert", "./shaders/bloom.frag"),
      mAccumulationShader.load("./shaders/quad.vert", "./shaders/accumulation.frag"),
      mDummyShader.load("./shaders/quad.vert", "./shaders/empty.frag"),
    };

    for (size_t i = 0; i < sizeof(result) / sizeof(status); i++)
      if (result[i] != status::success)
        return result[i];

    return status::success;
  }

  void render::set_uniforms()
  {
    GlobalData data;
    data.cameraDirection = glm::vec4(rt::get()->mCamera.mDirection, 1.0f);
    data.cameraPosition = glm::vec4(rt::get()->mCamera.mPosition, 1.0f);
    data.cameraUp = glm::vec4(rt::get()->mCamera.mUp, 1.0f);
    data.cameraRight = glm::vec4(rt::get()->mCamera.mRight, 1.0f);
    data.time = rt::get()->mTime;
    data.samples = mSamplesCount;
    data.bounces = mBouncesCount;
    data.halfHeight = rt::get()->mCamera.mHalfHeight;
    data.halfWidth = rt::get()->mCamera.mHalfWidth;
    data.useFXAA = mUseFXAA;
    data.gamma = mGamma;
    data.exposure = mExposure;
    data.blurSize = mBlurSize;
    data.windowSize = { mViewportWidth, mViewportHeight, 0, 0 };
    data.maxTextureSize = mTextures.sMaxTextureDataSize;
    data.renderMode = mRenderMode;
    data.interpolateNormals = mInterpolateNormals;
    mGlobalDataBuffer.set(&data);
  }
}
