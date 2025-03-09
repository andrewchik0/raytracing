#pragma once

#include "gui.h"
#include "input.h"
#include "render/render.h"
#include "scene/camera.h"

#include "scene/scene.h"
#include "window.h"

namespace raytracing
{

#define mark_zone(x) \
  rt::get()->mTimeHandler.mark_zone((x))

  struct render_options
  {
    uint32_t width = 1920, height = 1080;
    uint32_t samples = 128;
    uint32_t bounces = 16;

    std::string filename = "screenshot.png";

    bool sequence = true;

    std::string video_filename_base = "tmp/video";
    uint32_t framerate = 30;
    float duration = 10;
  };

  class rt
  {
  public:

    input mInput;
    render mRender;
    scene mScene;
    gui mGui;
    window mWindow;
    render_options mRenderOptions;
    utils::time_handler mTimeHandler;

    bool mVSyncEnabled = false;

    rt() = default;
    rt(const rt&) = delete;

    ~rt();

    void init(const init_options& options);
    void run();

    void render_to_image();
    void render_to_video();

    bool is_loading() const;

    static rt* get() { return sInstance; }

    utils::thread_pool& thread_pool() { return mThreadPool; }

  private:

    bool mTexturesLoading = false, mModelsLoading = false, mBVHLoading = false;
    bool mLoaded = false;

    utils::thread_pool mThreadPool;

    void set_viewport();
    void set_viewport(uint32_t width, uint32_t height);

    void load_async();

    static rt* sInstance;

    friend class camera;
    friend class input;
    friend class render;
    friend class gui;
    friend class serializer;
    friend class skybox;
    friend class textures;
    friend class bounding_volume_builder;
  };
} // namespace raytracing
