#include "input.h"

namespace raytracing
{
  void input::handle(const std::optional<sf::Event>& event)
  {
    if (event->is<sf::Event::KeyPressed>())
    {
      mKeyPressed[static_cast<int>(event->getIf<sf::Event::KeyPressed>()->code)] = true;
    }
    if (event->is<sf::Event::KeyReleased>())
    {
      mKeyPressed[static_cast<int>(event->getIf<sf::Event::KeyReleased>()->code)] = false;
    }
    if (event->is<sf::Event::MouseButtonPressed>())
    {
      mMousePressed[static_cast<int>(event->getIf<sf::Event::MouseButtonPressed>()->button)] = true;
    }
    if (event->is<sf::Event::MouseButtonReleased>())
    {
      mMousePressed[static_cast<int>(event->getIf<sf::Event::MouseButtonReleased>()->button)] = false;
    }
    if (event->is<sf::Event::MouseMoved>())
    {
      mMouseX = event->getIf<sf::Event::MouseMoved>()->position.x;
      mMouseY = event->getIf<sf::Event::MouseMoved>()->position.y;
      mMouseDeltaX = mMouseX - mMouseXOld;
      mMouseDeltaY = mMouseY - mMouseYOld;
      mMouseXOld = mMouseX;
      mMouseYOld = mMouseY;
    }
  }

}