#include "input.h"

#include <iostream>

#include "rt.h"

namespace raytracing
{
  bool input::key(const int32_t key)
  {
    return rt::get()->mInput.mKeyPressed[key];
  }

  void input::handle_key_click(const int32_t key, const int32_t action)
  {
    if (action == GLFW_PRESS)
    {
      mKeyPressed[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
      mKeyPressed[key] = false;
    }
  }

  void input::handle_mouse_click(const int32_t button, const int32_t action)
  {
    if (action == GLFW_RELEASE)
    {
      mKeyPressed[button] = false;
    }
    glfwGetCursorPos(rt::get()->mWindow.get(), &mMouseX, &mMouseY);
    if (
      action == GLFW_PRESS &&
      mMouseX > rt::get()->mGui.mViewportPosition.x &&
      mMouseY > rt::get()->mGui.mViewportPosition.y &&
      mMouseX < rt::get()->mGui.mViewportPosition.x + rt::get()->mGui.mViewportSize.x &&
      mMouseY < rt::get()->mGui.mViewportPosition.y + rt::get()->mGui.mViewportSize.y)
    {
      if (!rt::get()->mGui.mIsViewPortInFocus)
      {
        glfwGetCursorPos(rt::get()->mWindow.get(), &mMouseXOld, &mMouseYOld);
      }
      mKeyPressed[button] = true;
    }
  }


  void input::handle_mouse_move(const double xpos, const double ypos)
  {
    if (!rt::get()->mGui.mIsViewPortInFocus)
      return;

    mMouseX = xpos;
    mMouseY = ypos;
    mMouseDeltaX = mMouseX - mMouseXOld;
    mMouseDeltaY = mMouseY - mMouseYOld;
    mMouseXOld = mMouseX;
    mMouseYOld = mMouseY;
  }

  void input::clear()
  {
    mMouseDeltaX = 0;
    mMouseDeltaY = 0;
  }
}
