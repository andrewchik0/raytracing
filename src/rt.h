#pragma once

#include "camera.h"
#include "gui.h"
#include "input.h"
#include "render.h"
#include "scene_serializer.h"

namespace raytracing
{

  struct init_options
  {
    std::string title = "Ray Tracing";
    std::filesystem::path scene_filename;
    uint32_t width = 0, height = 0;
  };

  struct render_options
  {
    uint32_t width = 1920, height = 1080;
    uint32_t samples = 32;
    uint32_t bounces = 12;
    std::string filename = "screenshot.png";
  };

  class rt
  {
  public:

    input mInput;
    camera mCamera;
    render mRender {};
    gui mGui;
    render_options mRenderOptions;

    ~rt();

    void init(const init_options& options);
    void run();

    void add_model(const std::string& filename);

    void add_sphere(const std::string& name, const SphereObject& object);
    void add_plane(const std::string& name, const PlaneObject& object);
    void add_material(const std::string& name, const Material& material);
    void delete_sphere(size_t index);
    void delete_plane(size_t index);
    void delete_material(size_t index);

    void render_to_image();

    static rt* get() { return sInstance; }

  private:
    sf::RenderWindow mWindow;
    sf::Clock mClock;
    sf::Time mElapsedTime;

    float mTime = 0.0f;

    bool mTexturesLoading = false, mModelsLoading = false, mBVHLoading = false;
    bool mLoaded = false;

    uint32_t mWindowWidth = 0, mWindowHeight = 0;

    scene_serializer mSceneSerializer;

    std::string mSceneFilename = "";
    std::vector<std::string> mModelNames;

    std::string mSkyFilename;

    bool handle_messages();

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
