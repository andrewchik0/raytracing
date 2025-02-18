#pragma once

#include "input.h"
#include "camera.h"
#include "render.h"

namespace raytracing
{

  struct init_options
  {
    std::string title = "Ray Tracing";
    uint32_t width = 800, height = 600;
  };

  class rt
  {
  public:
    void init(const init_options& options);
    void run();

    void add_sphere(const SphereObject& object);
    void add_plane(const PlaneObject& object);

    static rt* get()
    {
      return sInstance;
    }

  private:
    sf::RenderWindow mWindow;
    sf::Clock mClock;
    sf::Time mElapsedTime;

    float mTime = 0.0f;

    uint32_t mWindowWidth = 0, mWindowHeight = 0;

    input mInput;
    camera mCamera;
    render mRender;

    void handle_messages();
    void resize();
    void imgui_update();

    static rt* sInstance;

    friend class camera;
    friend class input;
    friend class render;
  };
}
