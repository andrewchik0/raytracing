#pragma once

#include "pch.h"

namespace raytracing
{

  class input
  {
  public:
    double
      mMouseX = 0, mMouseY = 0,
      mMouseXOld = 0, mMouseYOld = 0,
      mMouseDeltaX = 0, mMouseDeltaY = 0;

    bool mKeyPressed[GLFW_KEY_LAST] { false };

    input() = default;
    input(const input&) = delete;
    ~input() = default;

    void clear();

    static bool key(int32_t key);

  private:

    void handle_mouse_click(int32_t button, int32_t action);
    void handle_key_click(int32_t key, int32_t action);
    void handle_mouse_move(double xpos, double ypos);

    friend class window;
  };
}
