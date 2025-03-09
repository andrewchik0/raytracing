#include "rt.h"

#include <filesystem>
#include <iostream>

#include <nfd.h>

#include "scene/serializer.h"

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

    serializer::load(options.scene_filename);
  }

  bool rt::is_loading() const
  {
    return mTexturesLoading || mModelsLoading || mBVHLoading;
  }

  void rt::run()
  {
    while (mWindow.is_open())
    {
      if (!is_loading())
      {
        if (!mLoaded)
        {
          mRender.post_init();
          mWindow.vsync(mVSyncEnabled);
          mLoaded = true;
        }

        mTimeHandler.tick();
        mWindow.clear();
        mark_zone("Poll events");
        mRender.clear();
        mark_zone("Render clear");

        if (input::key(GLFW_KEY_R))
          mRender.load_shaders();

        mScene.update(mTimeHandler.mDeltaTime);

        set_viewport();

        mark_zone("Update");

        mRender.draw();

        mGui.update();
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
      mScene.mCamera.move_right(0.1);

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
    mScene.mCamera.resize(width, height);
  }

  void rt::load_async()
  {
    mWindow.vsync(true);
    mModelsLoading = true;
    mThreadPool.send_task([&]
    {
      mScene.load_models();
      mTexturesLoading = true;
      mModelsLoading = false;

      mRender.mTextures.reload();

      mBVHLoading = true;
      mTexturesLoading = false;

      mScene.mBoundingVolumeBuilder.build();

      mBVHLoading = false;
    });
  }
} // namespace raytracing
