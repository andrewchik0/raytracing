#include "camera.h"

#include <cstdint>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../rt.h"

namespace raytracing
{

  void camera::resize(const uint32_t width, const uint32_t height)
  {
    mAspect = static_cast<float>(width) / static_cast<float>(height);

    mHalfHeight = tan(glm::radians(mFovY) / 2.0f);
    mHalfWidth = mAspect * mHalfHeight;
  }

  void camera::move_back(const float deltaSpeed) { move_forward(-deltaSpeed); }
  void camera::move_forward(const float deltaSpeed)
  {
    mPosition += glm::normalize(math::getNormalizedProjection(mDirection)) * deltaSpeed;
  }

  void camera::move_right(const float deltaSpeed) { move_left(-deltaSpeed); }
  void camera::move_left(const float deltaSpeed)
  {
    mPosition += glm::normalize(glm::cross(glm::vec3(0, 1, 0), mDirection)) * deltaSpeed;
  }

  void camera::move_down(const float deltaSpeed) { move_up(-deltaSpeed); }
  void camera::move_up(const float deltaSpeed)
  {
    mPosition.y += deltaSpeed;
  }

  void camera::update(const float deltaTime)
  {
    const float speed = 2.0f * deltaTime * mSpeed;

    if (
      input::key(GLFW_KEY_SPACE) |
      input::key(GLFW_KEY_LEFT_SHIFT) |
      input::key(GLFW_KEY_W) |
      input::key(GLFW_KEY_A) |
      input::key(GLFW_KEY_S) |
      input::key(GLFW_KEY_D) |
      input::key(GLFW_KEY_R))
    {
      rt::get()->mRender.reset_accumulation();
    }

    move_up(input::key(GLFW_KEY_SPACE) * speed);
    move_down(input::key(GLFW_KEY_LEFT_SHIFT) * speed);
    move_forward(input::key(GLFW_KEY_W) * speed);
    move_left(input::key(GLFW_KEY_A) * speed);
    move_back(input::key(GLFW_KEY_S) * speed);
    move_right(input::key(GLFW_KEY_D) * speed);

    if (input::key(GLFW_MOUSE_BUTTON_LEFT))
    {
      rt::get()->mWindow.set_grabbing(true);
      const float yaw = rt::get()->mInput.mMouseDeltaX / 200.0 * mMouseSensitivity;
      const float pitch = rt::get()->mInput.mMouseDeltaY / 200.0 * mMouseSensitivity;

      if (rt::get()->mInput.mMouseDeltaX || rt::get()->mInput.mMouseDeltaY)
        rt::get()->mRender.reset_accumulation();

      glm::mat4 rotateX = glm::rotate(glm::mat4(1.0), -yaw, glm::vec3(0, 1, 0));
      glm::mat4 rotateY = glm::rotate(glm::mat4(1.0), pitch, glm::cross(glm::vec3(0, 1, 0), mDirection));

      glm::vec3 checkLookAt = glm::normalize(rotateY * glm::vec4(mDirection, 1.0));

      if (float angleCos = (glm::dot(checkLookAt, math::getNormalizedProjection(mDirection))); angleCos <= 0)
      {
        constexpr float threshold = 1e-3f;
        const char sign = mDirection.y > 0 ? 1 : -1;

        mDirection = math::getNormalizedProjection(mDirection);
        mDirection = rotateX * glm::vec4(mDirection.x * threshold, sign * (1 - threshold), mDirection.z * threshold, 1);
      }
      else
      {
        mDirection = rotateY * rotateX * glm::vec4(mDirection, 1.0);
      }
    }
    else
    {
      rt::get()->mWindow.set_grabbing(false);
    }

    mHalfHeight = tan(glm::radians(mFovY) / 2.0f);
    mHalfWidth = mAspect * mHalfHeight;

    mRight = glm::normalize(glm::cross(mDirection, glm::vec3(0, 1, 0)));
    mUp = glm::cross(mRight, mDirection);

    mViewMatrix = glm::lookAt(mPosition, mPosition + mDirection, mUp);
    mProjectionMatrix = glm::perspective(glm::radians(mFovY), mAspect, 0.001f, 10000.0f);
  }
}
