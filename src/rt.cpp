#include "rt.h"

#include <filesystem>
#include <iostream>
#include <thread>

#include <nfd.h>

#include "scene_serializer.h"

namespace raytracing
{
  rt* rt::sInstance = nullptr;

  rt::~rt()
  {
    NFD_Quit();
  }

  void rt::init(const init_options& options)
  {
    sInstance = this;

    rt_assert(std::filesystem::exists("shaders/"), "Failed to load shaders, folder does not exist!")

    NFD_Init();

    mWindow.init(options);
    mRender.init();
    mGui.init();

    scene_serializer::load(options.scene_filename);
  }

  void rt::run()
  {
    while (mWindow.is_open())
    {
      if (!mTexturesLoading && !mModelsLoading && !mBVHLoading)
      {
        if (!mLoaded)
        {
          mRender.post_init();
          mWindow.vsync(mVSyncEnabled);
          mLoaded = true;
        }

        mWindow.clear();
        mRender.clear();

        mGui.update();
        mTimeHandler.tick();
        mCamera.update(mTimeHandler.mDeltaTime);

        set_viewport();

        mRender.draw(nullptr);
        mGui.draw();
        mWindow.draw();
        mInput.clear();
      }
      else
      {
        mWindow.clear();
        mTimeHandler.tick();
        mGui.update();
        mGui.draw();
        mWindow.draw();
        mInput.clear();
      }
    }
  }

  void rt::add_model(const std::string& filename)
  {
    mModelNames.push_back(filename);
  }

  void rt::render_to_image()
  {
    render_texture rt(mRenderOptions.width, mRenderOptions.height);

    // Store data
    uint32_t bounces = mRender.mBouncesCount;
    size_t accumulatingFrameIndex = mRender.mAccumulatingFrameIndex;
    size_t maxAccumulation = mRender.mMaxAccumulation;
    int renderMode = mRender.mRenderMode;

    mRender.mRenderMode = true;
    mRender.mBouncesCount = mRenderOptions.bounces;
    mRender.mAccumulatingFrameIndex = 0;
    mRender.mMaxAccumulation = mRenderOptions.samples;
    set_viewport(mRenderOptions.width, mRenderOptions.height);

    size_t sampleCounter = 0;

    while (sampleCounter++ < mRenderOptions.samples)
    {
      mRender.clear();
      mRender.draw(&rt);
      mTimeHandler.tick();
    }

    rt.write_to_file(mRenderOptions.filename);

    // Restore data
    mRender.mBouncesCount = bounces;
    mRender.mRenderMode = renderMode;
    mRender.mMaxAccumulation = maxAccumulation;
    mRender.mAccumulatingFrameIndex = accumulatingFrameIndex;
    set_viewport();
  }

  void rt::render_to_video()
  {
    // Store data
    uint32_t bounces = mRender.mBouncesCount;
    size_t accumulatingFrameIndex = mRender.mAccumulatingFrameIndex;
    size_t maxAccumulation = mRender.mMaxAccumulation;
    int renderMode = mRender.mRenderMode;

    mRender.mRenderMode = true;
    mRender.mBouncesCount = mRenderOptions.bounces;
    mRender.reset_accumulation();
    mRender.mMaxAccumulation = mRenderOptions.samples;

    set_viewport(mRenderOptions.width, mRenderOptions.height);

    for (size_t i = 0; i < mRenderOptions.duration * mRenderOptions.framerate; i++)
    {
      render_texture rt(mRenderOptions.width, mRenderOptions.height);
      size_t sampleCounter = 0;
      mCamera.move_right(0.1);

      while (sampleCounter++ < mRenderOptions.samples)
      {
        mRender.clear();
        mRender.draw(&rt);
        mTimeHandler.tick();
      }

      mRender.reset_accumulation();

      rt.write_to_file(mRenderOptions.video_filename_base + std::to_string(i) + ".png");
    }

    // Restore data
    mRender.mBouncesCount = bounces;
    mRender.mRenderMode = renderMode;
    mRender.mMaxAccumulation = maxAccumulation;
    mRender.mAccumulatingFrameIndex = accumulatingFrameIndex;
    set_viewport();
  }

  void rt::set_viewport()
  {
    set_viewport(mGui.mViewportSize.x, mGui.mViewportSize.y);
  }
  void rt::set_viewport(const uint32_t width, const uint32_t height)
  {
    mRender.resize(width, height);
    mCamera.resize(width, height);
  }

  void rt::load_async()
  {
    mWindow.vsync(true);
    mModelsLoading = true;
    std::thread([&]
    {
      std::vector<std::thread> modelThreads;

      for (auto& modelName : mModelNames)
      {
        modelThreads.emplace_back([&]
        {
          model m;
          m.load_from_file(modelName);

          mRender.mTriangles.insert(mRender.mTriangles.end(), m.mTriangles.begin(), m.mTriangles.end());
          mRender.mVertices.insert(mRender.mVertices.end(), m.mVertices.begin(), m.mVertices.end());
        });
      }

      for (auto& thread : modelThreads)
        thread.join();

      mTexturesLoading = true;
      mModelsLoading = false;

      mRender.mTextures.reload();

      mBVHLoading = true;
      mTexturesLoading = false;

      mRender.mBoundingVolumeBuilder.build();

      mBVHLoading = false;
    }).detach();
  }

  void rt::add_sphere(const std::string& name, const SphereObject& object)
  {
    if (mRender.mSpheresCount >= MAX_SPHERES)
      return;
    mRender.mSpheresAdditional[mRender.mSpheresCount].name = name;
    mRender.mSpheres[mRender.mSpheresCount++] = object;
  }
  void rt::add_plane(const std::string& name, const PlaneObject& object)
  {
    if (mRender.mPlanesCount >= MAX_PLANES)
      return;
    mRender.mPlanesAdditional[mRender.mPlanesCount].name = name;
    mRender.mPlanes[mRender.mPlanesCount++] = object;
  }
  void rt::add_material(const std::string& name, const Material& material)
  {
    if (mRender.mMaterialsCount >= MAX_MATERIALS)
      return;
    mRender.mMaterialsAdditional[mRender.mMaterialsCount].name = name;
    mRender.mMaterials[mRender.mMaterialsCount++] = material;
  }
  void rt::delete_sphere(size_t index)
  {
    for (size_t i = index; i < mRender.mSpheresCount; ++i)
      mRender.mSpheres[i] = mRender.mSpheres[i + 1];
    mRender.mSpheresCount--;
  }
  void rt::delete_plane(size_t index)
  {
    for (size_t i = index; i < mRender.mPlanesCount; ++i)
      mRender.mPlanes[i] = mRender.mPlanes[i + 1];
    mRender.mPlanesCount--;
  }
  void rt::delete_material(size_t index)
  {
    for (size_t i = index; i < mRender.mMaterialsCount; ++i)
      mRender.mMaterials[i] = mRender.mMaterials[i + 1];
    mRender.mMaterialsCount--;
  }
} // namespace raytracing
