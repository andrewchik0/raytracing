#include "final_render.h"

#include "rt.h"

namespace raytracing
{
  final_render::final_render(rt* rt)
    : mRt(rt)
  {
  }

  void final_render::render_to_image()
  {
    rt_assert(mRt != nullptr, "Failed to render image");

    render_texture rt(mRenderOptions.width, mRenderOptions.height);

    // Store data
    const uint32_t bounces = mRt->mRender.mBouncesCount;
    const uint32_t samples = mRt->mRender.mSamplesCount;
    const size_t maxAccumulation = mRt->mRender.mMaxAccumulation;
    const int renderMode = mRt->mRender.mRenderMode;

    mRt->mRender.mRenderMode = true;
    mRt->mRender.mBouncesCount = mRenderOptions.bounces;
    mRt->mRender.reset_accumulation();
    mRt->mRender.mMaxAccumulation = mRenderOptions.samples;
    mRt->mRender.mSamplesCount = 1;
    mRt->set_viewport(mRenderOptions.width, mRenderOptions.height);

    size_t sampleCounter = 0;

    while (sampleCounter++ < mRenderOptions.samples)
    {
      mRt->mRender.clear();
      mRt->mRender.draw(&rt);
      mRt->mTimeHandler.tick();
    }

    rt.write_to_file(mRenderOptions.filename);

    // Restore data
    mRt->mRender.mBouncesCount = bounces;
    mRt->mRender.mRenderMode = renderMode;
    mRt->mRender.mMaxAccumulation = maxAccumulation;
    mRt->mRender.reset_accumulation();
    mRt->mRender.mSamplesCount = samples;
    mRt->set_viewport();
  }

  void final_render::render_to_video()
  {
    rt_assert(mRt != nullptr, "Failed to render video");

    std::filesystem::path directory = mRenderOptions.video_filename_base + ".png";
    std::filesystem::create_directory(directory.parent_path());

    // Store data
    const uint32_t bounces = mRt->mRender.mBouncesCount;
    const size_t maxAccumulation = mRt->mRender.mMaxAccumulation;
    const int renderMode = mRt->mRender.mRenderMode;
    mRt->mRender.reset_accumulation();

    mRt->mRender.mRenderMode = true;
    mRt->mRender.mBouncesCount = mRenderOptions.bounces;
    mRt->mRender.reset_accumulation();
    mRt->mRender.mMaxAccumulation = mRenderOptions.samples;

    mRt->set_viewport(mRenderOptions.width, mRenderOptions.height);

    for (size_t i = 0; i < mRenderOptions.duration * mRenderOptions.framerate; i++)
    {
      render_texture rt(mRenderOptions.width, mRenderOptions.height);
      size_t sampleCounter = 0;
      mRt->mScene.mWater.animationTime += mRt->mScene.mWater.speed * 1.0 / mRenderOptions.framerate;

      while (sampleCounter++ < mRenderOptions.samples)
      {
        mRt->mRender.clear();
        mRt->mRender.draw(&rt);
        mRt->mTimeHandler.tick();
      }

      mRt->mRender.reset_accumulation();

      rt.write_to_file(mRenderOptions.video_filename_base + std::to_string(i) + ".png");
    }

    // Restore data
    mRt->mRender.mBouncesCount = bounces;
    mRt->mRender.mRenderMode = renderMode;
    mRt->mRender.mMaxAccumulation = maxAccumulation;
    mRt->mRender.reset_accumulation();
    mRt->set_viewport();
  }
}