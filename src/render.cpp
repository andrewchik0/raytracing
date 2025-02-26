#include "render.h"

#include <GL/glew.h>

#include "rt.h"

namespace raytracing
{
  sf::Glsl::Vec3 glm_to_sfml(glm::vec3 v)
  {
    return { v.x, v.y, v.z };
  }

  sf::Glsl::Vec2 glm_to_sfml(glm::vec2 v)
  {
    return { v.x, v.y };
  }

  void render::init()
  {
    if (load_shaders() != status::success)
      return;

    glewInit();

    mRenderQuad = sf::RectangleShape({static_cast<float>(rt::get()->mWindowWidth), static_cast<float>(rt::get()->mWindowHeight)});
    mRenderQuad.setFillColor(sf::Color::Red);
    mSceneBuffer.create(SCENE_BINDING, sizeof(SceneBuffer), "SceneBuffer", mShader.getNativeHandle());
    mGlobalDataBuffer.create(GLOBAL_DATA_BINDING, sizeof(GlobalData), "GlobalData", mShader.getNativeHandle());
    mGlobalDataBuffer.bind_to_shader("GlobalData", mPostShader.getNativeHandle());

    mTextures.allocate_triangles_buffer();
  }

  void render::post_init()
  {
    mBoundingVolumeBuilder.build();
    mTextures.load_triangles_to_gpu(mTriangles);
    mTextures.load_to_gpu();
  }

  void render::clear()
  {
    mLastFrameTexture.clear();
    mFinalTexture.clear();
    mBloomTexture.clear();
    mPostProcessedTexture.clear();
  }

  void render::draw(sf::RenderTarget* window)
  {
    mAccumulatingFrameIndex++;
    set_uniforms();
    push_scene();
    mTextures.bind();

    // Main pass
    mLastFrameTexture.draw(mRenderQuad, &mShader);
    mLastFrameTexture.display();

    // Gaussian blur pass
    mBloomShader.setUniform("renderedTexture", mLastFrameTexture.getTexture());
    mBloomTexture.draw(mRenderQuad, &mBloomShader);
    mBloomTexture.display();

    // Post-processing pass
    mPostShader.setUniform("renderedTexture", mLastFrameTexture.getTexture());
    mPostShader.setUniform("bloomTexture", mBloomTexture.getTexture());
    mPostProcessedTexture.draw(mRenderQuad, &mPostShader);
    mPostProcessedTexture.display();

    // Accumulation pass
    mAccumulationShader.setUniform("lastFrameTexture", mPostProcessedTexture.getTexture());
    mAccumulationShader.setUniform("accumulatedTexture", mAccumulatedTexture.getTexture());
    mAccumulationShader.setUniform("frameIndex", mAccumulatingFrameIndex);
    mFinalTexture.draw(mRenderQuad, &mAccumulationShader);
    mFinalTexture.display();

    // Store final buffer in accumulation buffer
    mDummyShader.setUniform("frameTexture", mFinalTexture.getTexture());
    mAccumulatedTexture.draw(mRenderQuad, &mDummyShader);
    mAccumulatedTexture.display();

    // Draw final result on window
    window->draw(mRenderQuad, &mDummyShader);
  }


  void render::resize(uint32_t width, uint32_t height)
  {
    auto result =
      mLastFrameTexture.resize({ width, height}) &&
      mBloomTexture.resize({ width, height}) &&
      mPostProcessedTexture.resize({ width, height}) &&
      mAccumulatedTexture.resize({ width, height}) &&
      mFinalTexture.resize({ width, height});
    if (!result)
      return;
    reset_accumulation();
  }

  void render::push_scene()
  {
    SceneBuffer buffer = {};
    for (size_t i = 0; i < mPlanesCount; i++)
    {
      mPlanes[i].normal = glm::normalize(mPlanes[i].normal);
    }
    memcpy(buffer.planes, mPlanes.data(), sizeof(PlaneObject) * MAX_PLANES);
    memcpy(buffer.spheres, mSpheres.data(), sizeof(SphereObject) * MAX_SPHERES);
    memcpy(buffer.vertices, mVertices.data(), sizeof(Vertex) * MAX_VERTICES);
    memcpy(buffer.materials, mMaterials.data(), sizeof(Material) * MAX_MATERIALS);
    memcpy(buffer.volumes, mBoundingVolumes.data(), sizeof(BoundingVolume) * MAX_BOUNDING_VOLUMES);
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
    if (!sf::Shader::isAvailable())
    {
      return status::error;
    }

    mShader.load("./shaders/quad.vert", "./shaders/main.frag");
    mPostShader.load("./shaders/quad.vert", "./shaders/post.frag");
    mBloomShader.load("./shaders/quad.vert", "./shaders/bloom.frag");
    mAccumulationShader.load("./shaders/quad.vert", "./shaders/accumulation.frag");
    mDummyShader.load("./shaders/quad.vert", "./shaders/empty.frag");

    return status::success;
  }

  void render::set_uniforms()
  {
    GlobalData data;
    data.cameraDirection = rt::get()->mCamera.mDirection;
    data.cameraPosition = rt::get()->mCamera.mPosition;
    data.cameraUp = rt::get()->mCamera.mUp;
    data.cameraRight = rt::get()->mCamera.mRight;
    data.time = rt::get()->mTime;
    data.samples = mSamplesCount;
    data.bounces = mBouncesCount;
    data.halfHeight = rt::get()->mCamera.mHalfHeight;
    data.halfWidth = rt::get()->mCamera.mHalfWidth;
    data.useFXAA = mUseFXAA;
    data.gamma = mGamma;
    data.exposure = mExposure;
    data.blurSize = mBlurSize;
    data.windowSize = { rt::get()->mWindowWidth, rt::get()->mWindowHeight };
    data.maxTextureSize = mTextures.sMaxTexture;
    mGlobalDataBuffer.set(&data);
  }
}
