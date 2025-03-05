#pragma once
#include "pch.h"

namespace raytracing
{
  class window
  {
  public:

    window() = default;
    window(const window&) = delete;

    ~window();

    void init(const init_options& options);

    void clear();
    void draw();

    void vsync(bool value);

    [[nodiscard]] uint32_t width() const;
    [[nodiscard]] uint32_t height() const;

    bool is_open() const;

    [[nodiscard]] GLFWwindow* get() const;

    void set_grabbing(bool value);

  private:

    static void key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);
    static void framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height);

    GLFWwindow* mWindowHandle = nullptr;
    GLFWcursor* mGrabbingCursor, *mDefaultCursor;
    uint32_t mWindowWidth = 0, mWindowHeight = 0;

    bool mIsGrabbing = false;
  };
}