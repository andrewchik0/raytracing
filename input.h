#pragma once

#include <cstdint>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Window.hpp>

namespace raytracing
{
  class input
  {
  public:
    int32_t
      mMouseX = 0, mMouseY = 0,
      mMouseXOld = 0, mMouseYOld = 0,
      mMouseDeltaX = 0, mMouseDeltaY = 0;
    bool mMousePressed[sf::Mouse::ButtonCount] { false };

    bool mKeyPressed[sf::Keyboard::KeyCount] { false };

    void handle(const std::optional<sf::Event>& event);
  };
}
