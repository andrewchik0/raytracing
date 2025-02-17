#pragma once

#include <glm\glm.hpp>

namespace raytracing
{
  class camera
  {
  public:
    float mFovY = glm::radians(90.0f);

    glm::vec3 mPosition { 0, 0, 2 };
    glm::vec3 mDirection { 0, 0, -1 };

    void resize(const uint32_t width, const uint32_t height);
    void update(float deltaTime);

    void move_back(float deltaSpeed);
    void move_forward(float deltaSpeed);

    void move_left(float deltaSpeed);
    void move_right(float deltaSpeed);

    void move_down(float deltaSpeed);
    void move_up(float deltaSpeed);

  private:
    float mHalfHeight = 0, mHalfWidth = 0;

    glm::vec3 mUp = { 0, 1, 0 };
    glm::vec3 mRight = { 1, 0, 0 };

    friend class rt;
  };
}