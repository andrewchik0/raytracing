#include "rt.h"

#include <filesystem>
#include <iostream>

#include <nfd.h>

#include "scene/serializer.h"
#include "scene/terrain_generator.h"

namespace raytracing
{
  rt* rt::sInstance = nullptr;

  rt::rt() : mRender(), mScene(), mGui(), mWindow(), mFinalRender(this) {}

  rt::~rt()
  {
    NFD_Quit();
  }

  void rt::init(const init_options& options /* = init_options() */)
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
      if (mLoadCallBack)
        mLoadCallBack(this);
      if (mScene.mTerrainOptions.exists)
        terrain_generator::init(&mScene);
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
