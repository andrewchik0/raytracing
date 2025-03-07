#pragma once

#include "camera.h"
#include "gui.h"
#include "input.h"
#include "render/render.h"

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
    camera mCamera;
    render mRender;
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

    void add_model(const std::string& filename, const glm::mat4& model = glm::mat4(1.0f));

    void add_sphere(const std::string& name, const SphereObject& object);
    void add_plane(const std::string& name, const PlaneObject& object);
    void add_material(const std::string& name, const Material& material);
    void delete_sphere(size_t index);
    void delete_plane(size_t index);
    void delete_material(size_t index);

    void render_to_image();
    void render_to_video();

    static rt* get() { return sInstance; }

  private:

    bool mTexturesLoading = false, mModelsLoading = false, mBVHLoading = false;
    bool mLoaded = false;

    utils::thread_pool mThreadPool;

    std::string mSceneFilename = "";
    struct model_data
    {
      std::string name;
      glm::mat4 model;
    };
    std::vector<model_data> mModelData;

    std::string mSkyFilename;

    void set_viewport();
    void set_viewport(uint32_t width, uint32_t height);

    void load_async();

    static rt* sInstance;

    friend class camera;
    friend class input;
    friend class render;
    friend class gui;
    friend class scene_serializer;
    friend class skybox;
    friend class textures;
    friend class bounding_volume_builder;
  };
} // namespace raytracing
