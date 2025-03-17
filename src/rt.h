#pragma once

#include "final_render.h"
#include "gui.h"
#include "input.h"
#include "render/render.h"

#include "scene/scene.h"
#include "window.h"

namespace raytracing
{

#define mark_zone(x) \
  rt::get()->mTimeHandler.mark_zone((x))

  class rt
  {
  public:

    input mInput;
    render mRender;
    scene mScene;
    gui mGui;
    window mWindow;
    final_render mFinalRender;
    utils::time_handler mTimeHandler;

    bool mVSyncEnabled = false;

    rt();
    rt(const rt&) = delete;

    ~rt();

    void init(const init_options& options = init_options());
    void run();

    void set_load_callback(const std::function<void(rt* app)>& loadCallBack)
      { mLoadCallBack = loadCallBack; }

    bool is_loading() const;

    static rt* get()
    {
      rt_assert(sInstance != nullptr, "Application instance is NULL!");
      return sInstance;
    }

    utils::thread_pool& thread_pool() { return mThreadPool; }

  private:

    bool mTexturesLoading = false, mModelsLoading = false, mBVHLoading = false;
    bool mLoaded = false;

    std::function<void(rt* app)> mLoadCallBack;

    utils::thread_pool mThreadPool;

    void set_viewport();
    void set_viewport(uint32_t width, uint32_t height);

    void load_async();

    static rt* sInstance;

    friend class gui;
    friend class serializer;
    friend class final_render;
  };
} // namespace raytracing
