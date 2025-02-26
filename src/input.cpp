#include "input.h"

#include <imgui.h>

#include "rt.h"

namespace raytracing
{
  bool input::key(sf::Keyboard::Key key)
  {
    return rt::get()->mInput.mKeyPressed[static_cast<int>(key)];
  }

  void input::handle(const std::optional<sf::Event>& event)
  {
    if (event->is<sf::Event::MouseButtonReleased>())
    {
      mMousePressed[static_cast<int>(event->getIf<sf::Event::MouseButtonReleased>()->button)] = false;
    }
    if (
      event->is<sf::Event::MouseButtonPressed>() &&
      event->getIf<sf::Event::MouseButtonPressed>()->position.x > rt::get()->mGui.mViewport->Pos.x &&
      event->getIf<sf::Event::MouseButtonPressed>()->position.y > rt::get()->mGui.mViewport->Pos.y &&
      event->getIf<sf::Event::MouseButtonPressed>()->position.x < rt::get()->mGui.mViewport->Pos.x + rt::get()->mGui.mViewport->Size.x &&
      event->getIf<sf::Event::MouseButtonPressed>()->position.y < rt::get()->mGui.mViewport->Pos.y + rt::get()->mGui.mViewport->Size.y
      )
    {
      if (!rt::get()->mGui.mIsViewPortInFocus)
      {
        mMouseXOld = event->getIf<sf::Event::MouseButtonPressed>()->position.x;
        mMouseYOld = event->getIf<sf::Event::MouseButtonPressed>()->position.y;
      }
      mMousePressed[static_cast<int>(event->getIf<sf::Event::MouseButtonPressed>()->button)] = true;
    }
    if (!rt::get()->mGui.mIsViewPortInFocus)
      return;

    if (event->is<sf::Event::KeyPressed>())
    {
      mKeyPressed[static_cast<int>(event->getIf<sf::Event::KeyPressed>()->code)] = true;
    }
    if (event->is<sf::Event::KeyReleased>())
    {
      mKeyPressed[static_cast<int>(event->getIf<sf::Event::KeyReleased>()->code)] = false;
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

  void input::clear()
  {
    mMouseDeltaX = 0;
    mMouseDeltaY = 0;
  }
}
