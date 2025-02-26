#include "camera.h"

#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "rt.h"

namespace raytracing
{

  glm::vec3 getNormalizedProjection(glm::vec3 v)
  {
    return glm::normalize(glm::vec3(v.x, 0.0, v.z));
  }

  void camera::resize(const uint32_t width, const uint32_t height)
  {
    mAspect = static_cast<float>(width) / static_cast<float>(height);

    mHalfHeight = tan(mFovY / 2.0f);
    mHalfWidth = mAspect * mHalfHeight;
  }

  void camera::move_back(float deltaSpeed) { move_forward(-deltaSpeed); }
  void camera::move_forward(float deltaSpeed)
  {
    mPosition += glm::normalize(getNormalizedProjection(mDirection)) * deltaSpeed;
  }

  void camera::move_right(float deltaSpeed) { move_left(-deltaSpeed); }
  void camera::move_left(float deltaSpeed)
  {
    mPosition += glm::normalize(glm::cross(glm::vec3(0, 1, 0), mDirection)) * deltaSpeed;
  }

  void camera::move_down(float deltaSpeed) { move_up(-deltaSpeed); }
  void camera::move_up(float deltaSpeed)
  {
    mPosition.y += deltaSpeed;
  }

  void camera::update(float deltaTime)
  {
    float speed = 2.0f * deltaTime * mSpeed;

    if (
      input::key(sf::Keyboard::Key::Space) |
      input::key(sf::Keyboard::Key::LShift) |
      input::key(sf::Keyboard::Key::W) |
      input::key(sf::Keyboard::Key::A) |
      input::key(sf::Keyboard::Key::S) |
      input::key(sf::Keyboard::Key::D) |
      input::key(sf::Keyboard::Key::R))
    {
      rt::get()->mRender.reset_accumulation();
    }

    move_up(input::key(sf::Keyboard::Key::Space) * speed);
    move_down(input::key(sf::Keyboard::Key::LShift) * speed);
    move_forward(input::key(sf::Keyboard::Key::W) * speed);
    move_left(input::key(sf::Keyboard::Key::A) * speed);
    move_back(input::key(sf::Keyboard::Key::S) * speed);
    move_right(input::key(sf::Keyboard::Key::D) * speed);

    if (rt::get()->mInput.mMousePressed[static_cast<int>(sf::Mouse::Button::Left)])
    {
      const auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::SizeAll).value();
      rt::get()->mWindow.setMouseCursor(cursor);
      float yaw = rt::get()->mInput.mMouseDeltaX / 200.0 * mMouseSensitivity;
      float pitch = rt::get()->mInput.mMouseDeltaY / 200.0 * mMouseSensitivity;

      if (rt::get()->mInput.mMouseDeltaX || rt::get()->mInput.mMouseDeltaY)
        rt::get()->mRender.reset_accumulation();

      glm::mat4 rotateX = glm::rotate(glm::mat4(1.0), -yaw, glm::vec3(0, 1, 0));
      glm::mat4 rotateY = glm::rotate(glm::mat4(1.0), pitch, glm::cross(glm::vec3(0, 1, 0), mDirection));

      glm::vec3 checkLookAt = glm::normalize(rotateY * glm::vec4(mDirection, 1.0));

      if (float angleCos = (glm::dot(checkLookAt, getNormalizedProjection(mDirection))); angleCos <= 0)
      {
        float threshold = 1e-3f;
        int8_t sign = mDirection.y > 0 ? 1 : -1;

        mDirection = getNormalizedProjection(mDirection);
        mDirection = rotateX * glm::vec4(mDirection.x * threshold, sign * (1 - threshold), mDirection.z * threshold, 1);
      }
      else
      {
        mDirection = rotateY * rotateX * glm::vec4(mDirection, 1.0);
      }
    }
    else
    {
      const auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow).value();
      rt::get()->mWindow.setMouseCursor(cursor);
    }


    mRight = glm::normalize(glm::cross(mDirection, glm::vec3(0, 1, 0)));
    mUp = glm::cross(mRight, mDirection);

    mViewMatrix = glm::lookAt(mPosition, mPosition + mDirection, mUp);
    mProjectionMatrix = glm::perspective(glm::radians(mFovY), mAspect, 0.001f, 10000.0f);
  }
}
