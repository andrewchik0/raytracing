#pragma once

#include "camera.h"
#include "imgui.h"
#include "input.h"
#include "render.h"
#include "scene_serializer.h"

namespace raytracing
{

  struct init_options
  {
    std::string title = "Ray Tracing";
    std::filesystem::path scene_filename;
    uint32_t width = 800, height = 600;
  };

  class rt
  {
  public:
    ~rt();

    void init(const init_options& options);
    void run();

    void add_sphere(const SphereObject& object);
    void add_plane(const PlaneObject& object);
    void add_material(const Material& material);
    void delete_sphere(size_t index);
    void delete_plane(size_t index);
    void delete_material(size_t index);

    void render_to_image(const std::string& image_path);

    static rt* get() { return sInstance; }

  private:
    sf::RenderWindow mWindow;
    sf::Clock mClock;
    sf::Time mElapsedTime;

    float mTime = 0.0f;

    uint32_t mWindowWidth = 0, mWindowHeight = 0;

    input mInput;
    camera mCamera;
    render mRender;
    imgui mGui;
    scene_serializer mSceneSerializer;

    void handle_messages();
    void resize();

    static rt* sInstance;

    friend class camera;
    friend class input;
    friend class render;
    friend class imgui;
    friend class scene_serializer;
    friend class skybox;
    friend class textures;
  };
} // namespace raytracing
