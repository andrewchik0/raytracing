#include "window.h"

#include "rt.h"

namespace raytracing
{
  void window::key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
  {
    rt::get()->mInput.handle_key_click(key, action);
  }

  void window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
  {
    rt::get()->mInput.handle_mouse_move(xpos, ypos);
  }

  void window::mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
  {
    rt::get()->mInput.handle_mouse_click(button, action);
  }

  void window::framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height)
  {
    rt::get()->mWindow.mWindowWidth = width;
    rt::get()->mWindow.mWindowHeight = height;
  }

  window::~window()
  {
    glfwTerminate();
  }

  void window::init(const init_options& options)
  {
    mWindowWidth = options.width;
    mWindowHeight = options.height;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_MAXIMIZED, options.maximized);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    mWindowHandle = glfwCreateWindow(mWindowWidth, mWindowHeight, options.title.c_str(), nullptr, nullptr);
    if (!mWindowHandle)
    {
      glfwTerminate();
    }
    glfwMakeContextCurrent(mWindowHandle);
    glfwSetKeyCallback(mWindowHandle, key_callback);
    glfwSetCursorPosCallback(mWindowHandle, cursor_position_callback);
    glfwSetMouseButtonCallback(mWindowHandle, mouse_button_callback);
    glfwSetFramebufferSizeCallback(mWindowHandle, framebuffer_size_callback);

    mGrabbingCursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    mDefaultCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
  }

  void window::clear()
  {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void window::draw() { glfwSwapBuffers(mWindowHandle); }
  void window::vsync(const bool value) { glfwSwapInterval(value); }

  uint32_t window::width() const { return mWindowWidth; }
  uint32_t window::height() const { return mWindowHeight; }

  bool window::is_open() const { return !glfwWindowShouldClose(mWindowHandle); }

  GLFWwindow* window::get() const { return mWindowHandle; }

  void window::set_grabbing(const bool value)
  {
    if (value != mIsGrabbing)
    {
      glfwSetCursor(mWindowHandle, value ? mGrabbingCursor : mDefaultCursor);
      mIsGrabbing = value;
    }
  }
} // namespace raytracing
